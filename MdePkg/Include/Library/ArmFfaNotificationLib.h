/** @file
  Provides function interfaces to perform notification related functionality through FF-A.

  Copyright (c) 2020 - 2022, Arm Ltd. All rights reserved.<BR>
  Copyright (c), Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

/**
  Sets the notification bitmap for a given destination.
  @param DestinationId       The ID of the destination.
  @param Flags               Flags for the operation.
  @param NotificationBitmap  The notification bitmap to set.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationSet (
  IN UINT16  DestinationId,
  IN UINT64  Flags,
  IN UINT64  NotificationBitmap
  );

/**
  Gets the notification bitmap for a given virtual CPU.
  @param VCpuId             The ID of the virtual CPU.
  @param Flags              Flags for the operation.
  @param NotificationBitmap  Pointer to store the retrieved notification bitmap.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationGet (
  IN UINT16   VCpuId,
  IN UINT64   Flags,
  OUT UINT64  *NotificationBitmap
  );

/**
  Creates a notification bitmap for the specified number of virtual CPUs.
  @param VCpuCount  The number of virtual CPUs.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationBitmapCreate (
  IN UINT16  VCpuCount
  );

/**
  Destroys the previously created notification bitmap.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationBitmapDestroy (
  VOID
  );

/**
  Binds a notification bitmap to a given destination.
  @param DestinationId       The ID of the destination.
  @param Flags               Flags for the operation.
  @param NotificationBitmap  The notification bitmap to bind.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationBind (
  IN UINT16  DestinationId,
  IN UINT32  Flags,
  IN UINT64  NotificationBitmap
  );

/**
  Unbinds a notification bitmap from a given destination.
  @param DestinationId       The ID of the destination.
  @param NotificationBitmap  The notification bitmap to unbind.

  @retval EFI_SUCCESS     The operation was successful.
  @retval Error           An error occurred.
**/
EFI_STATUS
EFIAPI
ArmFfaNotificationUnbind (
  IN UINT16  DestinationId,
  IN UINT64  NotificationBitmap
  );
