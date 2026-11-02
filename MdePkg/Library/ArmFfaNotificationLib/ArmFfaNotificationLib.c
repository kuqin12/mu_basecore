// SPDX-License-Identifier: BSD-3-Clause

/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 * Copyright (c), Microsoft Corporation.
 */

#include <Uefi.h>
#include <IndustryStandard/ArmFfaSvc.h>
#include <Library/DebugLib.h>
#include <Library/ArmFfaLib.h>
#include <Library/ArmFfaNotificationLib.h>

#define INVALID_SOURCE_ID  0xFFFF

STATIC UINT16  mPartitionId = INVALID_SOURCE_ID;

/**
  Retrieves the partition ID for the current partition.

  @retval EFI_SUCCESS           The operation was successful.
  @retval EFI_DEVICE_ERROR      Failed to get the partition ID.
**/
STATIC
EFI_STATUS
FfaGetPartitionId (
  VOID
  )
{
  EFI_STATUS  Status;
  UINT16      PartitionId;

  if (mPartitionId != INVALID_SOURCE_ID) {
    return EFI_SUCCESS;
  }

  Status = ArmFfaLibPartitionIdGet (&PartitionId);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to get partition ID: %r\n", __func__, Status));
    return Status;
  }

  mPartitionId = PartitionId;
  return EFI_SUCCESS;
}

// Match ArmFfaLib's internal FfaArgsToEfiStatus helper.
STATIC
EFI_STATUS
FfaGetResponseStatus (
  IN CONST ARM_FFA_ARGS  *FfaArgs
  )
{
  UINT32  FfaStatus;

  if (FfaArgs == NULL) {
    FfaStatus = ARM_FFA_RET_INVALID_PARAMETERS;
  } else if (IS_FID_FFA_ERROR (FfaArgs->Arg0)) {
    FfaStatus = FfaArgs->Arg2;
  } else if (FfaArgs->Arg0 == ARM_FFA_RET_NOT_SUPPORTED) {
    FfaStatus = ARM_FFA_RET_NOT_SUPPORTED;
  } else if ((FfaArgs->Arg0 == ARM_FID_FFA_INTERRUPT) || (FfaArgs->Arg0 == ARM_FID_FFA_YIELD)) {
    FfaStatus = ARM_FFA_RET_INTERRUPTED;
  } else {
    FfaStatus = ARM_FFA_RET_SUCCESS;
  }

  return FfaStatusToEfiStatus (FfaStatus);
}

/**
  Sets the notification bitmap for a given destination.
  @param DestinationId       The ID of the destination.
  @param Flags               Flags for the operation.
  @param NotificationBitmap  The notification bitmap to set.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationSet (
  IN UINT16  DestinationId,
  IN UINT64  Flags,
  IN UINT64  NotificationBitmap
  )
{
  EFI_STATUS    Status;
  ARM_FFA_ARGS  FfaArgs = { 0 };

  Status = FfaGetPartitionId ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FfaArgs.Arg0 = ARM_FID_FFA_NOTIFICATION_SET;
  FfaArgs.Arg1 = PACK_PARTITION_ID_INFO ((UINT32)mPartitionId, DestinationId);
  FfaArgs.Arg2 = Flags;
  FfaArgs.Arg3 = (UINT32)NotificationBitmap;
  FfaArgs.Arg4 = (UINT32)(NotificationBitmap >> 32);

  ArmCallFfa (&FfaArgs);

  return FfaGetResponseStatus (&FfaArgs);
}

/**
  Gets the notification bitmap for a given virtual CPU.
  @param VCpuId             The ID of the virtual CPU.
  @param Flags              Flags for the operation.
  @param NotificationBitmap  Pointer to store the retrieved notification bitmap.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationGet (
  IN UINT16   VCpuId,
  IN UINT64   Flags,
  OUT UINT64  *NotificationBitmap
  )
{
  EFI_STATUS    Status;
  ARM_FFA_ARGS  FfaArgs = { 0 };

  if (NotificationBitmap == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: NotificationBitmap is NULL\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  Status = FfaGetPartitionId ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FfaArgs.Arg0 = ARM_FID_FFA_NOTIFICATION_GET;
  FfaArgs.Arg1 = PACK_PARTITION_ID_INFO ((UINT32)VCpuId, mPartitionId);
  FfaArgs.Arg2 = Flags;

  ArmCallFfa (&FfaArgs);

  Status = FfaGetResponseStatus (&FfaArgs);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  switch (Flags) {
    case ARM_FFA_NOTIFICATION_FLAG_BITMAP_SP:
      *NotificationBitmap = ((UINT64)(UINT32)FfaArgs.Arg3 << 32) | (UINT32)FfaArgs.Arg2;
      break;
    case ARM_FFA_NOTIFICATION_FLAG_BITMAP_VM:
      *NotificationBitmap = ((UINT64)(UINT32)FfaArgs.Arg5 << 32) | (UINT32)FfaArgs.Arg4;
      break;
    case ARM_FFA_NOTIFICATION_FLAG_BITMAP_HYP:
      *NotificationBitmap = ((UINT64)(UINT32)FfaArgs.Arg7 << 32) | (UINT32)FfaArgs.Arg6;
      break;
    default:
      return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Creates a notification bitmap for the specified number of virtual CPUs.
  @param VCpuCount  The number of virtual CPUs.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationBitmapCreate (
  IN UINT16  VCpuCount
  )
{
  EFI_STATUS    Status;
  ARM_FFA_ARGS  FfaArgs = { 0 };

  Status = FfaGetPartitionId ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FfaArgs.Arg0 = ARM_FID_FFA_NOTIFICATION_BITMAP_CREATE;
  FfaArgs.Arg1 = mPartitionId;
  FfaArgs.Arg2 = VCpuCount;

  ArmCallFfa (&FfaArgs);
  return FfaGetResponseStatus (&FfaArgs);
}

/**
  Destroys the previously created notification bitmap.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationBitmapDestroy (
  VOID
  )
{
  EFI_STATUS    Status;
  ARM_FFA_ARGS  FfaArgs = { 0 };

  Status = FfaGetPartitionId ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FfaArgs.Arg0 = ARM_FID_FFA_NOTIFICATION_BITMAP_DESTROY;
  FfaArgs.Arg1 = mPartitionId;

  ArmCallFfa (&FfaArgs);
  return FfaGetResponseStatus (&FfaArgs);
}

/**
  Binds a notification bitmap to a given destination.
  @param DestinationId       The ID of the destination.
  @param Flags               Flags for the operation.
  @param NotificationBitmap  The notification bitmap to bind.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationBind (
  IN UINT16  DestinationId,
  IN UINT32  Flags,
  IN UINT64  NotificationBitmap
  )
{
  EFI_STATUS    Status;
  ARM_FFA_ARGS  FfaArgs = { 0 };

  Status = FfaGetPartitionId ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FfaArgs.Arg0 = ARM_FID_FFA_NOTIFICATION_BIND;
  FfaArgs.Arg1 = PACK_PARTITION_ID_INFO ((UINT32)DestinationId, mPartitionId);
  FfaArgs.Arg2 = Flags;
  FfaArgs.Arg3 = (UINT32)NotificationBitmap;
  FfaArgs.Arg4 = (UINT32)(NotificationBitmap >> 32);

  ArmCallFfa (&FfaArgs);
  return FfaGetResponseStatus (&FfaArgs);
}

/**
  Unbinds a notification bitmap from a given destination.
  @param DestinationId       The ID of the destination.
  @param NotificationBitmap  The notification bitmap to unbind.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationUnbind (
  IN UINT16  DestinationId,
  IN UINT64  NotificationBitmap
  )
{
  EFI_STATUS    Status;
  ARM_FFA_ARGS  FfaArgs = { 0 };

  Status = FfaGetPartitionId ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FfaArgs.Arg0 = ARM_FID_FFA_NOTIFICATION_UNBIND;
  FfaArgs.Arg1 = PACK_PARTITION_ID_INFO ((UINT32)DestinationId, mPartitionId);
  FfaArgs.Arg3 = (UINT32)NotificationBitmap;
  FfaArgs.Arg4 = (UINT32)(NotificationBitmap >> 32);

  ArmCallFfa (&FfaArgs);
  return FfaGetResponseStatus (&FfaArgs);
}
