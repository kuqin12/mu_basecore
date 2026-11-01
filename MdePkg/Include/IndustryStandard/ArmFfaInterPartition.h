/** @file
  Provides function interfaces to communicate with Notification service through FF-A.

  Copyright (c), Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#pragma once

#define ARM_FFA_INTER_PARTITION_GUID \
  { 0xe474d87e, 0x5731, 0x4044, { 0xa7, 0x27, 0xcb, 0x3e, 0x8c, 0xf3, 0xc8, 0xdf } }

#define ARM_FFA_INTER_PARTITION_STATUS_SUCCESS            (0)
#define ARM_FFA_INTER_PARTITION_STATUS_NOT_SUPPORTED      (-1)
#define ARM_FFA_INTER_PARTITION_STATUS_INVALID_PARAMETER  (-2)
#define ARM_FFA_INTER_PARTITION_STATUS_NO_MEM             (-3)

#define ARM_FFA_INTER_PARTITION_OPCODE_BASE          (0)
#define ARM_FFA_INTER_PARTITION_OPCODE_ADD           (ARM_FFA_INTER_PARTITION_OPCODE_BASE + 0)
#define ARM_FFA_INTER_PARTITION_OPCODE_REMOVE        (ARM_FFA_INTER_PARTITION_OPCODE_BASE + 1)
#define ARM_FFA_INTER_PARTITION_OPCODE_REGISTER      (ARM_FFA_INTER_PARTITION_OPCODE_BASE + 2)
#define ARM_FFA_INTER_PARTITION_OPCODE_UNREGISTER    (ARM_FFA_INTER_PARTITION_OPCODE_BASE + 3)
#define ARM_FFA_INTER_PARTITION_OPCODE_MEM_ASSIGN    (ARM_FFA_INTER_PARTITION_OPCODE_BASE + 4)
#define ARM_FFA_INTER_PARTITION_OPCODE_MEM_UNASSIGN  (ARM_FFA_INTER_PARTITION_OPCODE_BASE + 5)

#pragma pack (1)
typedef union {
  struct {
    UINT64    PerVcpu  : 1;
    UINT64    Reserved : 22;
    UINT64    Id       : 9;
    UINT64    Cookie   : 32;
  } Bits;
  UINT64    Uint64;
} ARM_FFA_INTER_PARTITION_NOTIFICATION_REG;

typedef union {
  struct {
    UINT64    Id       : 9;
    UINT64    Reserved : 23;
    UINT64    Cookie   : 32;
  } Bits;
  UINT64    Uint64;
} ARM_FFA_INTER_PARTITION_NOTIFICATION_UNREG;
#pragma pack ()

extern EFI_GUID  gArmFfaInterPartitionGuid;
