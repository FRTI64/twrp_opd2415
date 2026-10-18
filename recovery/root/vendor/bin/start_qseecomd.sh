#!/sbin/sh
# Start qseecomd only after the block device nodes exist and the TEE app
# firmware (/vendor/firmware_mnt, bind-mounted from TWRP's /firmware) is
# visible: the TEE keymaster/gatekeeper initialize on first connection and
# need their firmware images at that point. Starting too early makes the
# gatekeeper verify fail with TEE status -10008.
i=0
while [ $i -lt 120 ]; do
    if [ -e /dev/block/by-name/ssd ] || [ -e /dev/block/bootdevice/by-name/ssd ]; then
        break
    fi
    sleep 1
    i=$((i+1))
done

j=0
while [ $j -lt 120 ]; do
    if [ -d /firmware/image ]; then
        mount --bind /firmware /vendor/firmware_mnt 2>/dev/null
        if [ -d /vendor/firmware_mnt/image ]; then
            break
        fi
    fi
    sleep 1
    j=$((j+1))
done

sleep 2
setprop ctl.start vendor.qseecomd
