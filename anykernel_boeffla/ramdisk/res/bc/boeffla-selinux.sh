#!/system/bin/sh

# this script corrects selinux context of Boeffla-specific files

# root fs
busybox mount -o remount,rw /

chcon u:object_r:system_file:s0 /boeffla-anykernel
chcon u:object_r:system_file:s0 /sbin/busybox
chcon -R u:object_r:system_file:s0 /res/bc

busybox mount -o remount,ro /

# data fs
chcon -R u:object_r:system_file:s0 /data/.boeffla
