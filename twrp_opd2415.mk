#
# Copyright (C) 2026 The Android Open Source Project
#
# SPDX-License-Identifier: Apache-2.0
#

DEVICE_PATH := device/oneplus/opd2415

# Inherit from device.mk configuration
$(call inherit-product, $(DEVICE_PATH)/device.mk)

## Device identifier
PRODUCT_DEVICE  := opd2415
PRODUCT_NAME    := twrp_opd2415
PRODUCT_BRAND   := OnePlus
PRODUCT_MODEL   := OnePlus Pad 3
PRODUCT_MANUFACTURER := OnePlus

# Status icons centered (tablet has no punch-hole camera)
TW_STATUS_ICONS_ALIGN   := center
