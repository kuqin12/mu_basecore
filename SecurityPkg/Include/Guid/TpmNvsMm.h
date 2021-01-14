/** @file
  MM Core data.

Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2018, ARM Limited. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MM_TPM_NVS_HOB_H__
#define __MM_TPM_NVS_HOB_H__

#define MM_TPM_NVS_HOB_GUID \
  { 0xc96c76eb, 0xbc78, 0x429c, { 0x9f, 0x4b, 0xda, 0x51, 0x78, 0xc2, 0x84, 0x57 }}

extern EFI_GUID gTpmNvsMmGuid;

#pragma pack(1)
typedef struct {
  UINT8                  SoftwareSmi;
  UINT32                 Parameter;
  UINT32                 Response;
  UINT32                 Request;
  UINT32                 RequestParameter;
  UINT32                 LastRequest;
  UINT32                 ReturnCode;
} PHYSICAL_PRESENCE_NVS;

typedef struct {
  UINT8                  SoftwareSmi;
  UINT32                 Parameter;
  UINT32                 Request;
  UINT32                 ReturnCode;
} MEMORY_CLEAR_NVS;

typedef struct {
  PHYSICAL_PRESENCE_NVS  PhysicalPresence;
  MEMORY_CLEAR_NVS       MemoryClear;
  UINT32                 PPRequestUserConfirm;
  UINT32                 TpmIrqNum;
  BOOLEAN                IsShortFormPkgLength;
} TCG_NVS;

typedef struct {
  UINT8                  OpRegionOp;
  UINT32                 NameString;
  UINT8                  RegionSpace;
  UINT8                  DWordPrefix;
  UINT32                 RegionOffset;
  UINT8                  BytePrefix;
  UINT8                  RegionLen;
} AML_OP_REGION_32_8;

typedef struct {
  UINT16                Function;
  UINT16                Reserved;
  EFI_STATUS            ReturnStatus;
  EFI_PHYSICAL_ADDRESS  TargetAddress;
  UINT64                TargetValue;
} TPM_NVS_MM_COMM_BUFFER;
#pragma pack()

typedef enum {
  TPM_NVS_MM_EXCHANGE_INFO,
} TPM_NVS_MM_OP;

#endif
