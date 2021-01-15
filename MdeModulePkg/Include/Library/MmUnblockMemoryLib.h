/** @file
  MM Unblock Memory Library Interface.

  This library provides an abstraction layer of requesting certain page access to be unblocked
  by MM environment if applicable.

  Copyright (c), Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MM_UNBLOCK_MEMORY_LIB_H_
#define _MM_UNBLOCK_MEMORY_LIB_H_

/**
  This API provides a way to unblock certain data pages to be accessible inside MM environment.

  @param  UnblockAddress          The address of buffer caller requests to unblock, the address
                                  has to be page aligned.
  @param  NumberOfPages           The number of pages requested to be unblocked from MM
                                  environment.

  @return EFI_SUCCESS             The request goes through successfully.
  @return EFI_NOT_AVAILABLE_YET   The requested functionality is not produced yet.
  @return EFI_UNSUPPORTED         The requested functionality is not supported on current platform.
  @return Error                   See defintion of REQUEST_UNBLOCK_PAGE.

**/
EFI_STATUS
EFIAPI
MmUnblockMemoryRequest (
  IN EFI_PHYSICAL_ADDRESS   UnblockAddress,
  IN UINT64                 NumberOfPages
);

#endif // _MM_UNBLOCK_MEMORY_LIB_H_
