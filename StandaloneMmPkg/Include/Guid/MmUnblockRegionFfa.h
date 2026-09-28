/** @file
  Defines the GUID and structure used in FF-A based unblock memory secure service.

  Copyright (c), Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#pragma once

///
/// The GUID of the FF-A based unblock memory secure service.
///
#define MM_UNBLOCK_FFA_GUID \
  { \
    0x1abea672, 0x414d, 0x4e3d, { 0xb5, 0x50, 0xa0, 0x4c, 0x49, 0x72, 0xb, 0x7e } \
  }

typedef enum {
  MM_UNBLOCK_FFA_OPCODE_RETRIEVE,
  MM_UNBLOCK_FFA_OPCODE_RELINQUISH,
  MM_UNBLOCK_FFA_OPCODE_MAX,
} MM_UNBLOCK_FFA_OPCODE;

#pragma pack (1)

///
/// The structure defines the data layout of the FF-A based unblock memory secure service.
///
typedef struct {
  ///
  /// OpCode of the unblock request.
  ///
  UINT64        OpCode;

  ///
  /// Status of the operation.
  ///
  EFI_STATUS    Status;

  union {
    struct {
      ///
      /// Handle of the unblock request returned by the FF-A memory management protocol.
      ///
      UINT64    Handle;

      ///
      /// Base address of the memory region to be unblocked.
      ///
      UINT64    BaseAddress;

      ///
      /// Number of pages of the memory region to be unblocked.
      ///
      UINT64    NumOfPages;
    } Retrieve;
  };
} MM_UNBLOCK_FFA;

#pragma pack ()

extern EFI_GUID  gMmUnblockRegionFfaGuid;
