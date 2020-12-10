/** @file
  Report Status Code Router Driver which produces SMM Report Stataus Code Handler Protocol
  and SMM Status Code Protocol.

  Copyright (c) 2009 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "ReportStatusCodeRouterCommon.h"

/**
  Entry point of Generic Status Code Driver.

  This function is the entry point of SMM Status Code Router .
  It produces SMM Report Stataus Code Handler and Status Code protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/
EFI_STATUS
EFIAPI
GenericStatusCodeStandaloneMmEntry (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_MM_SYSTEM_TABLE    *SystemTable
  )
{
  return GenericStatusCodeCommonEntry ();
}
