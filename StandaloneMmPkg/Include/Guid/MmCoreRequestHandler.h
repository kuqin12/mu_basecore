/** @file
  Data structure used to request information/functionality between DXE and
  Standalone MM core.

Copyright (c), Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MM_CORE_REQUEST_HANDLER_H_
#define _MM_CORE_REQUEST_HANDLER_H_

#define MM_CORE_REQUEST_HANDLER_GUID \
  { 0x8c633b23, 0x1260, 0x4ea6, { 0x83, 0xf, 0x7d, 0xdc, 0x97, 0x38, 0x21, 0x11 } }


extern EFI_GUID gStandaloneMmCoreRequestHandlerGuid;

#define   MM_CORE_REQUEST_SIG         SIGNATURE_32('M', 'C', 'R', 'H')
#define   MMC_ORE_REQUEST_REVISION    1

#pragma pack(push, 1)

typedef struct _MM_CORE_REQUEST_HEADER {
  UINT32      Signature;
  UINT32      Revision;
  UINT32      Request;
  EFI_STATUS  Result;
} MM_CORE_REQUEST_HEADER;

/**
  This structure is used to communicate requested unblock memory information
  from DXE to MM environment. This data will be checked by MM core to ensure
  the requested region does not cover critical pages.

**/
typedef struct _UNBLOCK_MEMORY_DATA_BUFFER {
  EFI_MEMORY_DESCRIPTOR   MemoryDescriptor;
  EFI_GUID                IdentifierGuid;
} MM_CORE_UNBLOCK_MEMORY_PARAMS;

#pragma pack(pop)

#define   MM_CORE_REQUEST_UNBLOCK_MEM     0x0001

#endif // _MM_CORE_REQUEST_HANDLER_H_
