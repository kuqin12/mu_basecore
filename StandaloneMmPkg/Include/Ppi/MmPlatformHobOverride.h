/** @file
  MM Platform HOB Override PPI.

  When a platform produces this PPI, the Standalone MM IPL delegates the entire
  MM HOB list construction to the platform instead of performing its default
  platform + foundation HOB assembly. This provides a non-breaking extension
  point for platforms that need to overhaul the MM HOB creation flow without
  changing the MmPlatformHobProducerLib library class interface.

  Copyright (c) 2025, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MM_PLATFORM_HOB_OVERRIDE_PPI_H_
#define MM_PLATFORM_HOB_OVERRIDE_PPI_H_

#define MM_PLATFORM_HOB_OVERRIDE_PPI_GUID \
  { 0x9d1f6a2c, 0x4b8e, 0x4c7a, { 0x8f, 0x3d, 0x21, 0x6b, 0x5e, 0x9a, 0x44, 0xd1 } }

typedef struct _MM_PLATFORM_HOB_OVERRIDE_PPI MM_PLATFORM_HOB_OVERRIDE_PPI;

/**
  Produce the complete HOB list for the Standalone MM environment.

  This uses the same two-call buffer protocol as CreateMmPlatformHob(): call once
  with HobBuffer = NULL to retrieve the required size (the function returns
  RETURN_BUFFER_TOO_SMALL and updates HobBufferSize), then call again with a
  buffer of at least that size.

  When this PPI is present, the IPL treats the returned HOB list as the complete
  MM HOB list and does NOT append its own platform or foundation HOBs. The
  producer is therefore responsible for every HOB the MM Core requires, including
  the MM Core module allocation HOB.

  @param[in]      This            Pointer to this PPI.
  @param[in]      HobBuffer       Buffer to receive the HOB list, or NULL for sizing.
  @param[in, out] HobBufferSize   On input, the size of HobBuffer. On output, the
                                  required/used size of the HOB list.

  @retval RETURN_SUCCESS            The HOB list was produced successfully.
  @retval RETURN_BUFFER_TOO_SMALL   HobBuffer is too small; HobBufferSize is updated.
  @retval RETURN_INVALID_PARAMETER  HobBufferSize is NULL, or HobBuffer is NULL with a non-zero size.

**/
typedef
EFI_STATUS
(EFIAPI *MM_PLATFORM_HOB_OVERRIDE_BUILD_HOB_LIST)(
  IN     CONST MM_PLATFORM_HOB_OVERRIDE_PPI  *This,
  IN OUT VOID                                *HobBuffer,
  IN OUT UINTN                               *HobBufferSize
  );

///
/// This PPI provides a platform-supplied override of the MM HOB list construction.
///
struct _MM_PLATFORM_HOB_OVERRIDE_PPI {
  MM_PLATFORM_HOB_OVERRIDE_BUILD_HOB_LIST    BuildHobList;
};

extern EFI_GUID  gMmPlatformHobOverridePpiGuid;

#endif
