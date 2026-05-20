/** @file

  Copyright (c) 2011-2017, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PeilessSec.h"

#include <AArch64/AArch64.h>

/**
  Architecture specific initialization routine.
**/
VOID
ArchInitialize (
  VOID
  )
{
  if (ArmReadCurrentEL () == AARCH64_EL2) {
    // Trap General Exceptions. All exceptions that would be routed to EL1 are routed to EL2.
    // MU_CHANGE: OR in TGE rather than overwriting HCR_EL2; otherwise we would clear E2H/RW
    // that the early MMU bring-up relies on, which silently changes the layout of TCR_EL2/
    // SCTLR_EL2/TTBR0_EL2 out from under the running CPU and faults on the next fetch.
    ArmWriteHcr (ArmReadHcr () | ARM_HCR_TGE);
  }
}
