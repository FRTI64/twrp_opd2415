# TWRP Device Tree for OnePlus Pad 3 (OPD2415) / OnePlus Pad 2 Pro (OPD2413)

TWRP device tree for the OnePlus Pad 3 (international, `OPD2415`, OP6190L1) and
the OnePlus Pad 2 Pro (China, `OPD2413`, OP615EL1). Both are the same hardware,
codenamed **erhai** (Qualcomm SM8750 "sun" platform, Oryon cores).

## Status

- [x] Device tree based on the proven [kmiit sm87xx unified tree](https://github.com/kmiit/twrp_device_oplus_sm87xx)
- [x] fstab/partition data cross-checked against stock OxygenOS 16 recovery and the LineageOS erhai tree
- [x] Tablet landscape rotation via libinit (`persist.twrp.rotation=270`)
- [x] On-device testing (touch, display orientation, MTP, fastbootd)
- [x] Metadata decryption (wrappedkey_v0) - working
- [x] keystore2 / keymint / QTI secureprocessor stack brought up in recovery
- [ ] CE decryption with lock screen PIN - **known limitation**

Works:

- ADB
- Display (landscape)
- Metadata FBE decryption (wrappedkey_v0 + inlinecrypt)
- Fastbootd
- Flashing (all partitions incl. oplus `my_*`)
- MTP
- Sideload
- Touch
- USB OTG
- Vibrator

Known limitation:

- **CE decryption with a lock screen PIN fails.** The password check itself
  succeeds (TEE gatekeeper verifies the handle), but the QTI TEE keymaster
  rejects the gatekeeper's auth-token request with an internal error (-10020),
  so the token comes back empty and the CE key cannot be unlocked. This is
  inside QTI's closed TEE stack; everything in the userspace recovery
  environment (keystore2, keymint, secureprocessor daemon, TEE app mounts,
  service ordering) has been brought up and verified.
  **Workaround:** remove the lock screen PIN in OxygenOS before using TWRP;
  with no credential the CE key degrades to the default password and /data
  mounts fine (standard practice for modern TWRP devices).

## How the recovery boots on this device

These OPLUS SM87xx devices have a dedicated `recovery_a/_b` partition, but the
image contains **only a ramdisk** - the bootloader combines the kernel from the
`boot` partition with the ramdisk from the `recovery` partition. The kernel and
its modules therefore always match the stock firmware. This is why the build
sets `BOARD_EXCLUDE_KERNEL_FROM_RECOVERY_IMAGE := true`.

## Build

```bash
mkdir twrp && cd twrp
repo init --depth=1 -u https://github.com/TWRP-Test/platform_manifest_twrp_aosp.git -b twrp-16.0
repo sync
git clone https://github.com/frti64/twrp_opd2415 device/oneplus/opd2415

source build/envsetup.sh
lunch twrp_opd2415
make recoveryimage
```

The image will be at `out/target/product/opd2415/recovery.img`.

## Flash (bootloader must be unlocked)

```bash
adb reboot bootloader
fastboot flash recovery_a recovery.img
fastboot flash recovery_b recovery.img
fastboot reboot recovery
```

To restore the stock recovery, flash the `recovery.img` from your full OTA
package (e.g. via Oxygen Updater / `payload-dumper`).

## Credits

- [TeamWin](https://github.com/TeamWin) for TWRP
- [kmiit](https://github.com/kmiit) for the OPLUS sm87xx device tree this work is based on
- [LineageOS](https://github.com/LineageOS) for the erhai device tree / sm8750-common data
- OnePlus for the device

## Disclaimer

```
Your warranty is now void. Flashing a custom recovery can brick your device.
You are doing this at your own risk.
```
