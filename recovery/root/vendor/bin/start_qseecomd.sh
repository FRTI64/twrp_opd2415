#!/sbin/sh
# Start qseecomd only after the block device nodes exist. The QSEECom SSD
# listener (secure storage for the TEE apps) needs /dev/block/by-name/ssd;
# if qseecomd starts before ueventd created the node, ssd_init_service fails
# and the TEE gatekeeper/keymaster initialize without their storage, breaking
# gatekeeper password verification (-10008/-10020).
i=0
while [ $i -lt 120 ]; do
    if [ -e /dev/block/by-name/ssd ] || [ -e /dev/block/bootdevice/by-name/ssd ]; then
        break
    fi
    sleep 1
    i=$((i+1))
done
sleep 2
setprop ctl.start vendor.qseecomd
