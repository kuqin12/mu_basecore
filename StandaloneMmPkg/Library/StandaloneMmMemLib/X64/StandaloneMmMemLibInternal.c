/** @file
  Internal ARCH Specific file of MM memory check library.

  MM memory check library implementation. This library consumes MM_ACCESS_PROTOCOL
  to get MMRAM information. In order to use this library instance, the platform should produce
  all MMRAM range via MM_ACCESS_PROTOCOL, including the range for firmware (like MM Core
  and MM driver) and/or specific dedicated hardware.

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2016 - 2018, ARM Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>

#include <Guid/MmramMemoryReserve.h>

//
// Maximum support address used to check input buffer
//
extern EFI_PHYSICAL_ADDRESS  mMmMemLibInternalMaximumSupportAddress;
extern EFI_MMRAM_DESCRIPTOR *mMmMemLibInternalMmramRanges;
extern UINTN                 mMmMemLibInternalMmramCount;

/**
  Calculate and save the maximum support address.

**/
VOID
MmMemLibInternalCalculateMaximumSupportAddress (
  VOID
  )
{
  VOID         *Hob;
  UINT32       RegEax;
  UINT8        PhysicalAddressBits;

  //
  // Get physical address bits supported.
  //
  Hob = GetFirstHob (EFI_HOB_TYPE_CPU);
  if (Hob != NULL) {
    PhysicalAddressBits = ((EFI_HOB_CPU *) Hob)->SizeOfMemorySpace;
  } else {
    AsmCpuid (0x80000000, &RegEax, NULL, NULL, NULL);
    if (RegEax >= 0x80000008) {
      AsmCpuid (0x80000008, &RegEax, NULL, NULL, NULL);
      PhysicalAddressBits = (UINT8) RegEax;
    } else {
      PhysicalAddressBits = 36;
    }
  }
  //
  // IA-32e paging translates 48-bit linear addresses to 52-bit physical addresses.
  //
  ASSERT (PhysicalAddressBits <= 52);
  if (PhysicalAddressBits > 48) {
    PhysicalAddressBits = 48;
  }

  //
  // Save the maximum support address in one global variable
  //
  mMmMemLibInternalMaximumSupportAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)(LShiftU64 (1, PhysicalAddressBits) - 1);
  DEBUG ((EFI_D_INFO, "mMmMemLibInternalMaximumSupportAddress = 0x%lx\n", mMmMemLibInternalMaximumSupportAddress));
}

/**
  Initialize cached Mmram Ranges from HOB.

**/
VOID
MmMemLibInternalPopulateMmramRanges (
  VOID
  )
{
  EFI_HOB_GUID_TYPE               *MmramRangesHob;
  EFI_MMRAM_HOB_DESCRIPTOR_BLOCK  *MmramRangesHobData;

  MmramRangesHob = GetFirstGuidHob (&gEfiMmPeiMmramMemoryReserveGuid);
  if (MmramRangesHob == NULL) {
    return;
  }

  MmramRangesHobData = GET_GUID_HOB_DATA (MmramRangesHob);
  if (MmramRangesHobData == NULL) {
    return;
  }

  if (MmramRangesHobData->Descriptor == NULL) {
    return;
  }

  mMmMemLibInternalMmramRanges = AllocatePool (MmramRangesHobData->NumberOfMmReservedRegions * sizeof (EFI_MMRAM_DESCRIPTOR));
  if (mMmMemLibInternalMmramRanges) {
    CopyMem (mMmMemLibInternalMmramRanges,
             MmramRangesHobData->Descriptor,
             MmramRangesHobData->NumberOfMmReservedRegions * sizeof (EFI_MMRAM_DESCRIPTOR));
    mMmMemLibInternalMmramCount = MmramRangesHobData->NumberOfMmReservedRegions;
  }
}

/**
  Deinitialize cached Mmram Ranges.

**/
VOID
MmMemLibInternalFreeMmramRanges (
  VOID
  )
{
  if (mMmMemLibInternalMmramRanges != NULL) {
    FreePool (mMmMemLibInternalMmramRanges);
  }
}

