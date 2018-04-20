#!/bin/sh
export ARCH=arm

case "$1" in
	i9300)
    DEVICE="i9300"
    ;;
    
    n7100)
    DEVICE="n7100"
    ;;

    n7105)
    DEVICE="n7105"
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

mv ../$DEVICE/$DEVICE/boeffla-init-bc.sh ../$DEVICE/ramdisk/res/bc

mv ../$DEVICE/$DEVICE/anykernel.sh ../$DEVICE/

find -name '*.ko' -exec cp -av {} ../$DEVICE/modules/ \;
${CROSS_COMPILE}strip --strip-unneeded ../$DEVICE/modules/*
cp arch/arm/boot/zImage ../$DEVICE/
rm ../$DEVICE/modules/placeholder
