# TWRP Device Tree for OnePlus Pad 3 (OPD2415) / OnePlus Pad 2 Pro (OPD2413)

TWRP device tree for the OnePlus Pad 3 (international, `OPD2415`, OP6190L1) and
the OnePlus Pad 2 Pro (China, `OPD2413`, OP615EL1). Both are the same hardware,
codenamed **erhai** (Qualcomm SM8750 "sun" platform, Oryon cores).

## Status

- [x] Device tree based on the proven [kmiit sm87xx unified tree](https://github.com/kmiit/twrp_device_oplus_sm87xx)
- [x] fstab/partition data cross-checked against stock OxygenOS 16 recovery and the LineageOS erhai tree
- [x] Tablet landscape rotation via libinit (`persist.twrp.rotation=270`)
- [x] On-device testing (touch, display orientation, MTP, fastbootd, flashing)
- [x] Metadata decryption (wrappedkey_v0) - working
- [x] Full TEE stack brought up in recovery: keystore2, keymint, QTI secureprocessor,
      qseecomd (SSD storage listener), HIDL gatekeeper@1.0, hwservicemanager
      (recovery VINTF manifests fixed for AOSP 16 libvintf, SELinux userspace allows)
- [ ] CE decryption with lock screen PIN - **known limitation** (see below)

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

- **CE decryption with a lock screen PIN fails.** Password verification reaches
  the TEE gatekeeper with the correct handle and password token (verified
  byte-for-byte against an independent scrypt/SHA512 reference), and the
  HIDL gatekeeper@1.0 service is registered and running, but the QTI TEE
  answers the gatekeeper verify with an internal status (-10008) and returns
  no auth token. Without the token, the old keystore2 build rejects the
  synthetic-password blob unwrap with KEY_USER_NOT_AUTHENTICATED (-26).
  This is inside QTI's closed TEE stack; every userspace component around it
  (keystore2/keymint/qseecomd/secureprocessor/gatekeeper/manifests/sepolicy)
  has been brought up and verified working.
  **Workaround:** remove the lock screen PIN in OxygenOS before using TWRP;
  with no credential the CE key degrades to the default password and /data
  mounts fine (standard practice for modern TWRP devices).

## Debug history

This port required bringing up the whole OPLUS/QTI security stack in recovery:

1. `keystore2.rc`: the module rc runs keystore2 with `/data/misc/keystore`,
   which cannot be created before /data is decrypted. Use
   `/tmp/misc/keystore` as root (TWRP's own init.rc pre-creates it).
2. `recovery.fstab`: mount the modem partition at `/vendor/firmware_mnt`
   (TEE app images). TWRP additionally mounts it at `/firmware` via
   twrp.flags (`mounttodecrypt`); the secureprocessor setup script
   bind-mounts it for the TEE tools.
3. Recovery VINTF: servicemanager/hwservicemanager in recovery only read
   `/system/etc/vintf/manifest.xml` + `manifest/`. AOSP 16 libvintf is
   strict: no `<interface>` + `<fqname>` duplicates, no duplicate majors,
   single framework/device type per directory, and v8.0 fragments.
   Without `android.hidl.manager@1.2` declared, hwservicemanager disables
   itself (`hwservicemanager.disabled=true`).
4. SELinux: hwservicemanager checks `service_manager add/find` in userspace
   even in permissive mode; recovery needs explicit allows
   (`sepolicy/recovery_crypto.te`, `SELINUX_IGNORE_NEVERALLOWS`).
5. TWRP vold: the twrp-16.0-new-data-decryption branch (stretched LSKF,
   protector ID from locksettings.db, keystore DB sync) is applied as
   `patches/0002-new-data-decryption.patch`; libhidl is built without
   `ENFORCE_VINTF_MANIFEST` so TWRP's HIDL passthrough works
   (`patches/0003-libhidl-disable-vintf-enforcement.patch`).
6. QTI security stack binaries/libs copied from the working color597 erhai
   ramdisk (qseecomd, keymint, keystore2 V1 + private libcrypto, HIDL
   gatekeeper@1.0 impl) and the secureprocessor daemon + TEE firmware mount.


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
