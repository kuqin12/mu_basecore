/** @file
  Standalone MM Unblock Page Protocol.

  This protocol provides a means of requesting certain page access to be unblocked by
  MM memory manager.

  Copyright (c), Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _STANDALONE_MM_UNBLOCK_MEMORY_H_
#define _STANDALONE_MM_UNBLOCK_MEMORY_H_

#define STANDALONE_MM_UNBLOCK_MEMORY_PROTOCOL_GUID \
  { \
    0x10b5eea9, 0xbe0d, 0x4f11, { 0x86, 0x36, 0x1c, 0xb7, 0xa, 0xa3, 0xba, 0x6d } \
  }

#define STANDALONE_MM_UNBLOCK_REQUEST_PROTOCOL_VERSION   1

typedef struct _STANDALONE_MM_UNBLOCK_MEMORY_PROTOCOL       STANDALONE_MM_UNBLOCK_MEMORY_PROTOCOL;

extern EFI_GUID gStandaloneMmUnblockMemoryProtocolGuid;

/**
  This API provides a way to unblock certain data pages to be accessible inside MM environment.

  The requested buffer needs to be page size aligned and mapped as data pages. The unblocked
  buffer will labeled as CPL3 data pages accessible by user mode drivers. MM memory manager will
  reject the unblock request after Ready-To-Lock event is signaled or whenever memory manager
  deems a necessity to lock down memory management.

  @param  UnblockAddress          The address of buffer caller requests to unblock, the address
                                  has to be page aligned.
  @param  NumberOfPages           The number of pages requested to be unblocked from MM
                                  environment.
  @param  IdentifierGuid          The unique caller ID from requester.

  @return EFI_SUCCESS             The request goes through successfully.
  @return EFI_SECURITY_VIOLATION  The requested address failed to pass security check for
                                  unblocking.
  @return EFI_INVALID_PARAMETER   Input address or caller ID is either NULL pointer or not aligned.
  @return EFI_ACCESS_DENIED       The request is rejected by standalone MM due to post ready-to-lock.

**/
typedef
EFI_STATUS
(EFIAPI *REQUEST_UNBLOCK_PAGE) (
  IN EFI_PHYSICAL_ADDRESS   UnblockAddress,
  IN UINT64                 NumberOfPages,
  IN CONST EFI_GUID         *IdentifierGuid
  );

#pragma pack (1)

struct _STANDALONE_MM_UNBLOCK_MEMORY_PROTOCOL {
  UINTN                     Version;
  REQUEST_UNBLOCK_PAGE      RequestUnblockPages;
};

#pragma pack ()

#endif // _STANDALONE_MM_UNBLOCK_MEMORY_H_

