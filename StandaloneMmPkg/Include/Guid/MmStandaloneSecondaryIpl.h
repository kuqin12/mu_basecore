/** @file
  GUID of the Standalone MM Secondary IPL firmware file.

  The Standalone MM IPL searches the firmware volumes for a file with this name.
  When present, the IPL loads that image OUTSIDE MMRAM and hands control to it
  (rather than jumping directly to the MM Core in MMRAM). The secondary IPL
  performs one-time preparation and then transfers control to the MM Core, whose
  entry point it recovers from the MM Core module allocation HOB
  (gEfiHobMemoryAllocModuleGuid) in the MM HOB list.

  Absence of this file selects the default behavior: the IPL jumps directly to
  the MM Core in MMRAM. The feature is therefore purely data driven - including
  the file in the platform FDF turns it on, omitting it turns it off.

  Copyright (c) 2025, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MM_STANDALONE_SECONDARY_IPL_H_
#define MM_STANDALONE_SECONDARY_IPL_H_

#define MM_STANDALONE_SECONDARY_IPL_FILE_GUID \
  { 0x7c2b4e90, 0x6f15, 0x4ad3, { 0xb9, 0x1c, 0x0a, 0x77, 0x3e, 0x82, 0x5d, 0x46 } }

extern EFI_GUID  gMmStandaloneSecondaryIplFileGuid;

#endif
