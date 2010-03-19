#!/bin/sh

# this script installs grub in the image disk passed as parameter

imageFile=$1

# get offset, sometimes 512, 16384 or 35226 (512 bytes per unit by 63 cylinders)
offset=$(parted $imageFile unit b print | tail -2 | head -1 | cut -f 1 --delimit="B" | cut -c 9-) 

echo $offset
losetup -o $offset /dev/loop5 $imageFile

mkdir /tmp/hdd
mount /dev/loop5 /tmp/hdd/

mkdir /tmp/hdd/boot
cp -r boot/iso.template/boot/grub /tmp/hdd/boot

umount /tmp/hdd



#rm -rf /tmp/hdd
#losetup -d /dev/loop5

