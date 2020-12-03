/** @file -- VarCheckPolicyLib.h
This internal header file defines the common interface of constructor for
VarCheckPolicyLib.

Copyright (c) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _VAR_CHECK_POLICY_LIB_H_
#define _VAR_CHECK_POLICY_LIB_H_

/**
  Common constructor function of VarCheckPolicyLib to register VarCheck handler
  and SW MMI handlers.

  @retval EFI_SUCCESS       The constructor executed correctly.

**/
EFI_STATUS
EFIAPI
VarCheckPolicyLibCommonConstructor (
  VOID
  );

#endif // _VAR_CHECK_POLICY_LIB_H_
