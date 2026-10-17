#!/sbin/sh
# Wait for TWRP to mount the modem partition at /firmware (mounttodecrypt),
# then bind-mount it at /vendor/firmware_mnt where the secure processor
# daemon expects the TEE app images.
i=0
while [ $i -lt 180 ]; do
    if [ -d /firmware/image ]; then
        mount --bind /firmware /vendor/firmware_mnt 2>/dev/null
        if [ -d /vendor/firmware_mnt/image ]; then
            break
        fi
    fi
    sleep 1
    i=$((i+1))
done
