/** @file
  Arm Ffa library code for Dxe Driver

  Copyright (c) 2024, Arm Limited. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

   @par Glossary:
     - FF-A - Firmware Framework for Arm A-profile

   @par Reference(s):
     - Arm Firmware Framework for Arm A-Profile [https://developer.arm.com/documentation/den0077/latest]

**/

#include <Uefi.h>
#include <Pi/PiMultiPhase.h>
#include <Pi/PiDxeCis.h>
#include <Guid/FfaPerf.h>

#include <Library/ArmLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/ArmFfaLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <IndustryStandard/ArmFfaSvc.h>

#include <Guid/ArmFfaRxTxBufferInfo.h>

#include "ArmFfaCommon.h"
#include "ArmFfaRxTxMap.h"

STATIC EFI_EVENT  mFfaExitBootServiceEvent;
STATIC UINT16     mPartId;
STATIC BOOLEAN    mIsFfaSupported;
extern UINT64     mFfaPerfBuffer;

// Notification event when virtual address map is set.
STATIC EFI_EVENT  mSetVirtualAddressMapEvent;

/**
  Unmap RX/TX buffer on Exit Boot Service.

  @param [in]   Event      Registered exit boot service event.
  @param [in]   Context    Additional data.

**/
STATIC
VOID
EFIAPI
ArmFfaLibExitBootServiceEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  ArmFfaLibRxTxUnmap ();
}

/**
  Notification callback on SetVirtualAddressMap event.

  This function notifies the MM communication protocol interface on
  SetVirtualAddressMap event and converts pointers used in this driver
  from physical to virtual address.

  @param  Event          SetVirtualAddressMap event.
  @param  Context        A context when the SetVirtualAddressMap triggered.

  @retval EFI_SUCCESS    The function executed successfully.
  @retval Other          Some error occurred when executing this function.

**/
STATIC
VOID
EFIAPI
NotifySetVirtualAddressMap (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS  Status;
  DIRECT_MSG_ARGS  DirectMsgArgs;

  Status = gRT->ConvertPointer (
                  EFI_OPTIONAL_PTR,
                  (VOID **)&mFfaPerfBuffer
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "NotifySetVirtualAddressMap():"
      " Unable to convert MM runtime pointer. Status:0x%r\n",
      Status
      ));
  }

  DirectMsgArgs.Arg0 = (UINTN)(UINT64)mFfaPerfBuffer;

  // After conversion, send a Ffa direct req 2 to stmm to notify the change of virtual address map.
  Status = ArmFfaLibMsgSendDirectReq2 (
    0x8004, // SPMC Part ID
    &gArmFfaPerfDataBufferGuid,
    &DirectMsgArgs
    );
  if (EFI_ERROR (Status)) {
    CpuDeadLoop ();
  }
}

/**
  ArmFfaLib Constructor.

  @param [in]   ImageHandle      Image Handle
  @param [in]   SystemTable      System Table

  @retval EFI_SUCCESS            Success
  @retval EFI_INVALID_PARAMETER  Invalid alignment of Rx/Tx buffer
  @retval EFI_OUT_OF_RESOURCES   Out of memory
  @retval Others                 Error

**/
EFI_STATUS
EFIAPI
ArmFfaDxeLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                 Status;
  EFI_HOB_GUID_TYPE          *RxTxBufferHob;
  ARM_FFA_RX_TX_BUFFER_INFO  *BufferInfo;
  UINTN                      Property1;
  UINTN                      Property2;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR Descriptor;

  Status = gDS->AddMemorySpace (
                  EfiGcdMemoryTypeReserved,
                  FFA_PERF_DATA_BUFFER_BASE,
                  FFA_PERF_DATA_BUFFER_SIZE,
                  EFI_MEMORY_WB |
                  EFI_MEMORY_XP |
                  EFI_MEMORY_RUNTIME
                  );
  DEBUG ((
    DEBUG_ERROR,
    "%a: "
    "Failed to add MM-NS Buffer Memory Space - %r\n",
    __func__,
    Status
    ));
  if (EFI_ERROR (Status) && (Status != EFI_ACCESS_DENIED)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: "
      "Failed to add MM-NS Buffer Memory Space - %r\n",
      __func__,
      Status
      ));
  } else if (Status == EFI_ACCESS_DENIED) {
    // We are already good with this process.
    DEBUG ((
      DEBUG_INFO,
      "%a: MM-NS Buffer Memory Space is already added by other driver.\n",
      __func__
      ));

    gDS->GetMemorySpaceDescriptor (
      FFA_PERF_DATA_BUFFER_BASE,
      &Descriptor
      );

    if ((Descriptor.BaseAddress != FFA_PERF_DATA_BUFFER_BASE) ||
        (Descriptor.Length != FFA_PERF_DATA_BUFFER_SIZE) ||
        ((Descriptor.Attributes & (EFI_MEMORY_WB | EFI_MEMORY_XP | EFI_MEMORY_RUNTIME)) !=
         (EFI_MEMORY_WB | EFI_MEMORY_XP | EFI_MEMORY_RUNTIME))) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: MM-NS Buffer Memory Space attributes are different from expected (BaseAddress: 0x%lx, Length: 0x%lx, Attributes: 0x%lx).\n",
        __func__,
        Descriptor.BaseAddress,
        Descriptor.Length,
        Descriptor.Attributes
        ));

      Status = gDS->RemoveMemorySpace (
                      FFA_PERF_DATA_BUFFER_BASE,
                      FFA_PERF_DATA_BUFFER_SIZE
                      );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "%a: "
          "Failed to remove existing MM-NS Buffer Memory Space - %r\n",
          __func__,
          Status
          ));
      }

      // Try to add the memory space again
      Status = gDS->AddMemorySpace (
                      EfiGcdMemoryTypeReserved,
                      FFA_PERF_DATA_BUFFER_BASE,
                      FFA_PERF_DATA_BUFFER_SIZE,
                      EFI_MEMORY_WB |
                      EFI_MEMORY_XP |
                      EFI_MEMORY_RUNTIME
                      );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "%a: "
          "Failed to add MM-NS Buffer Memory Space (second attempt) - %r\n",
          __func__,
          Status
          ));
      }

      Status = gDS->SetMemorySpaceAttributes (
                    FFA_PERF_DATA_BUFFER_BASE,
                    FFA_PERF_DATA_BUFFER_SIZE,
                    EFI_MEMORY_WB | EFI_MEMORY_XP | EFI_MEMORY_RUNTIME
                    );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "%a: "
          "Failed to set MM-NS Buffer Memory attributes\n",
          __func__
          ));
      }
    }
  } else {
    Status = gDS->SetMemorySpaceAttributes (
                    FFA_PERF_DATA_BUFFER_BASE,
                    FFA_PERF_DATA_BUFFER_SIZE,
                    EFI_MEMORY_WB | EFI_MEMORY_XP | EFI_MEMORY_RUNTIME
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: "
        "Failed to set MM-NS Buffer Memory attributes\n",
        __func__
        ));
    }
  }

  // Register notification callback when virtual address is associated
  // with the physical address.
  // Create a Set Virtual Address Map event.
  Status = gBS->CreateEvent (
                  EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                  TPL_NOTIFY,
                  NotifySetVirtualAddressMap,
                  NULL,
                  &mSetVirtualAddressMapEvent
                  );
  ASSERT_EFI_ERROR (Status);

  Status = ArmFfaLibCommonInit (&mPartId, &mIsFfaSupported);
  if (EFI_ERROR (Status)) {
    if (!mIsFfaSupported) {
      /*
       * FF-A being unsupported doesn't mean a failure of loading the driver/library
       * instance (i.e) ArmPkg's MmCommunication Dxe/PEI Driver uses as well as SpmMm.
       * So If FF-A is not supported the the MmCommunication Dxe/PEI falls back to SpmMm.
       * For this case, return EFI_SUCCESS.
       */
      return EFI_SUCCESS;
    }

    return Status;
  }

  if (PcdGetBool (PcdFfaExitBootEventRegistered)) {
    return EFI_SUCCESS;
  }

  RxTxBufferHob = GetFirstGuidHob (&gArmFfaRxTxBufferInfoGuid);
  if (RxTxBufferHob != NULL) {
    BufferInfo = GET_GUID_HOB_DATA (RxTxBufferHob);
    if (!BufferInfo->RemapRequired) {
      /*
       * ArmFfaPeiLib handles the Rx/Tx buffer Remap and update the
       * BufferInfo with permanant memory. So use it as it is.
       */
      PcdSet64S (PcdFfaTxBuffer, (UINTN)BufferInfo->TxBufferAddr);
      PcdSet64S (PcdFfaRxBuffer, (UINTN)BufferInfo->RxBufferAddr);
    } else {
      /*
       * SEC maps Rx/Tx buffer, But no PEIM module doesn't use
       * ArmFfaPeiLib. In this case, the BufferInfo includes
       * temporary Rx/Tx buffer address.
       *
       * Therefore, remap Rx/Tx buffer with migrated address again.
       */
      Status = RemapFfaRxTxBuffer (BufferInfo);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a: Failed to remap Rx/Tx buffer... Status: %r\n", __func__, Status));
        return Status;
      }

      BufferInfo->RemapRequired = FALSE;
    }
  } else {
    Status = ArmFfaLibRxTxMap ();
    if (Status == EFI_UNSUPPORTED) {
      /*
       * When ARM_FID_FFA_PARTITION_INFO_GET_REGS is supported,
       * Rx/Tx buffer might not be required to request service to
       * secure partition.
       * So, consider EFI_UNSUPPORTED for Rx/Tx buffer as SUCCESS.
       */
      Status = ArmFfaLibGetFeatures (
                 ARM_FID_FFA_PARTITION_INFO_GET_REGS,
                 0x00,
                 &Property1,
                 &Property2
                 );
      if (!EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, "%a Rx/Tx buffer doesn't support.\n", __func__));
      }
    }

    /*
     * When first Dxe instance (library or driver) which uses ArmFfaLib loaded,
     * It already maps Rx/Tx buffer.
     * From Next Dxe instance which uses ArmFfaLib it doesn't need to map Rx/Tx
     * buffer again but it uses the mapped one.
     * ArmFfaLibRxTxMap() returns EFI_ALREADY_STARTED when the Rx/Tx buffers
     * already maps.
     */
    if ((Status != EFI_SUCCESS) && (Status != EFI_ALREADY_STARTED)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Failed to Map Rx/Tx buffer. Status: %r\n",
        __func__,
        Status
        ));
      return Status;
    }
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ArmFfaLibExitBootServiceEvent,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &mFfaExitBootServiceEvent
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Failed to register ExitBootService event. Status: %r\n",
      __func__,
      Status
      ));
    goto ErrorHandler;
  }

  PcdSetBoolS (PcdFfaExitBootEventRegistered, TRUE);

  return EFI_SUCCESS;

ErrorHandler:
  if (RxTxBufferHob != NULL) {
    ArmFfaLibRxTxUnmap ();
  }

  return Status;
}

/**
  Return partition or VM ID

  @param[out] PartId  The partition or VM ID

  @retval EFI_SUCCESS  Partition ID or VM ID returned
  @retval Others       Errors

**/
EFI_STATUS
EFIAPI
ArmFfaLibGetPartId (
  OUT UINT16  *PartId
  )
{
  if (PartId != NULL) {
    *PartId = mPartId;
  }

  return EFI_SUCCESS;
}

/**
  Check FF-A support or not.

  @retval TRUE                   Supported
  @retval FALSE                  Not supported

**/
BOOLEAN
EFIAPI
IsFfaSupported (
  IN VOID
  )
{
  return mIsFfaSupported;
}

/**
  Callback for when Unmap is called to handle any post unmap
  functionality.

**/
VOID
EFIAPI
UnmapCallback (
  IN VOID
  )
{
  // Do nothing
}
