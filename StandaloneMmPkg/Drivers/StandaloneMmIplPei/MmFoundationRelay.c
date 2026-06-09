/** @file

  Copyright (c) 2024, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Guid/MmStandaloneSecondaryIpl.h>
#include <Ppi/LoadFile.h>

#include "StandaloneMmIplPei.h"

/**
  Locates the relay image entry point.

  @param RelayImageEntryPoint      Pointer to relay image entry point for output.

  @retval EFI_SUCCESS     Relay image successfully located.
  @retval Others          Failed to locate the relay image.

**/
EFI_STATUS
FindMmIplRelayImage (
  OUT EFI_PHYSICAL_ADDRESS  *RelayImageEntryPoint
  )
{
  EFI_STATUS             Status;
  UINTN                  Instance;
  EFI_PEI_LOAD_FILE_PPI  *LoadFile;
  EFI_PEI_FV_HANDLE      VolumeHandle;
  EFI_PEI_FILE_HANDLE    FileHandle;
  EFI_PHYSICAL_ADDRESS   RelayImageAddress;
  UINT64                 RelayImageSize;
  UINT32                 AuthenticationState;

  Instance = 0;

  while (TRUE) {
    Status = PeiServicesFfsFindNextVolume (Instance++, &VolumeHandle);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = PeiServicesFfsFindFileByName (&gMmStandaloneSecondaryIplFileGuid, VolumeHandle, &FileHandle);
    if (!EFI_ERROR (Status)) {
      Status = PeiServicesLocatePpi (&gEfiPeiLoadFilePpiGuid, 0, NULL, (VOID **)&LoadFile);
      ASSERT_EFI_ERROR (Status);

      Status = LoadFile->LoadFile (
                           LoadFile,
                           FileHandle,
                           &RelayImageAddress,
                           &RelayImageSize,
                           RelayImageEntryPoint,
                           &AuthenticationState
                           );
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Unable to find PE32 section in MmIplRelay image ffs %r!\n", Status));
        return Status;
      }

      break;
    } else {
      continue;
    }
  }

  return Status;
}
