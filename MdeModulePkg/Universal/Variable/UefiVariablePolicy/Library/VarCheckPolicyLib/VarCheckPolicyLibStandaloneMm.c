/** @file -- VarCheckPolicyLibTraditional.c
This is an instance of a VarCheck lib constructor for Standalone MM.

Copyright (c) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "VarCheckPolicyLib.h"

/**
  Standalone MM constructor function of VarCheckPolicyLib to invoke common
  constructor routine.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The constructor executed correctly.

**/
EFI_STATUS
EFIAPI
VarCheckPolicyLibStandaloneConstructor (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_MM_SYSTEM_TABLE    *SystemTable
  )
{
  return VarCheckPolicyLibCommonConstructor ();
}
