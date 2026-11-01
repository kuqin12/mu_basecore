/** @file
  Provides specification defined structures to describe FF-A non-secure resource information.

  Specification v1.3 BETA0: https://support.arm.com/documentation/den0077

  Copyright (c), Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#pragma once

#pragma pack(1)

/**
 * Resource Information Descriptor Header
 * Section 13.13: FFA_NS_RES_INFO_GET - Table 13.63
 */
typedef struct {
  UINT32    AmdSize;
  UINT32    AmdCount;
  UINT32    AmdOffset;
  UINT32    Reserved;
} ARM_FFA_RESOURCE_INFO_DESC_HEADER;

/**
 * Address Map Descriptor
 * Section 13.13: FFA_NS_RES_INFO_GET - Table 13.64
 */
typedef struct {
  UINT64    BaseAddress;
  UINT32    PageCount;
  UINT8     Permissions;
  UINT16    EndpointId;
  UINT8     Flags;
} ARM_FFA_ADDRESS_MAP_DESC;
#pragma pack()
