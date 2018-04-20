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

  make mrproper

  cp -f arch/arm/configs/$(echo $DEVICE)_defconfig .config
  make oldconfig
  mv -f .config arch/arm/configs/$(echo $DEVICE)_defconfig


