#!/bin/sh
export ARCH=arm

case "$1" in
	i9300)
    CODENAME="m0"
    DEVICE="i9300"
    SYSTEM_DEVICE="/dev/block/mmcblk0p9"
    CACHE_DEVICE="/dev/block/mmcblk0p8"
    DATA_DEVICE="/dev/block/mmcblk0p12"
    BOOT_DEVICE="/dev/block/mmcblk0p5"
    ;;
    
    n7100)
    CODENAME="t03g"
    DEVICE="n7100"
    SYSTEM_DEVICE="/dev/block/mmcblk0p13"
    CACHE_DEVICE="/dev/block/mmcblk0p12"
    DATA_DEVICE="/dev/block/mmcblk0p16"
    BOOT_DEVICE="/dev/block/mmcblk0p8"
    ;;

    n7105)
    CODENAME="t0lte"
    DEVICE="n7105"
    SYSTEM_DEVICE="/dev/block/mmcblk0p13"
    CACHE_DEVICE="/dev/block/mmcblk0p12"
    DATA_DEVICE="/dev/block/mmcblk0p16"
    BOOT_DEVICE="/dev/block/mmcblk0p8"
    ;;
esac

  cp -f arch/arm/configs/$(echo $DEVICE)_defconfig .config
  make oldconfig

  TIMESTAMP=$(date +%s)

  make -j$(grep -c ^processor /proc/cpuinfo)

  TIMESTAMP2=$(date +%s)

  echo "compile time:" $(($TIMESTAMP2 - $TIMESTAMP)) "seconds"

if [ -e ../$DEVICE ]; then
	rm -rf ../$DEVICE
fi

mkdir ../$DEVICE

cp -rf template/* ../$DEVICE

mv ../$DEVICE/$DEVICE/bccontroller.sh ../$DEVICE/ramdisk/res/bc

sed -i "s;###CODENAME###;${CODENAME};" ../$DEVICE/anykernel.sh;
sed -i "s;###DEVICE###;${DEVICE};" ../$DEVICE/anykernel.sh;
sed -i "s;###BOOT###;${BOOT_DEVICE};" ../$DEVICE/anykernel.sh;

sed -i "s;###SYSTEM###;${SYSTEM_DEVICE};" ../$DEVICE/ramdisk/res/bc/boeffla-init-bc.sh;
sed -i "s;###CACHE###;${CACHE_DEVICE};" ../$DEVICE/ramdisk/res/bc/boeffla-init-bc.sh;
sed -i "s;###DATA###;${DATA_DEVICE};" ../$DEVICE/ramdisk/res/bc/boeffla-init-bc.sh;

find -name '*.ko' -exec cp -av {} ../$DEVICE/modules/ \;
${CROSS_COMPILE}strip --strip-unneeded ../$DEVICE/modules/*
cp arch/arm/boot/zImage ../$DEVICE/
rm ../$DEVICE/modules/placeholder
