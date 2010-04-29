#!/bin/sh -x

# this script installs grub in the image disk passed as parameter

imageFile=$1

if [ $# -eq 0 ]
then
	echo Usage: `basename $0` disk-image-file
	echo Example: `basename $0` disk.img
	exit
fi

# first, we copy grub files inside the partition

# get offset, sometimes 512, 16384 or 35226 (512 bytes per unit by 63 cylinders)
offset=$(parted $imageFile unit b print | tail -2 | head -1 | cut -f 1 --delimit="B" | cut -c 9-) 

echo "Partition offset: ${offset}"

loopDevice=$(losetup -f)
losetup -o $offset $loopDevice $imageFile

mkdir ./mount/hdd
mount -t vfat $loopDevice ./mount/hdd/

mkdir ./mount/hdd/boot
cp -r boot/iso.template/boot/grub ./mount/hdd/boot

umount ./mount/hdd
losetup -d $loopDevice

# done copying files, now we have to install grub

losetup $loopDevice $imageFile

echo "(hd0) ${loopDevice}" >/tmp/device_map
grub-setup --device-map=/tmp/device_map -r "(hd0,1)" "(hd0)"

rm -rf ./mount/hdd
losetup -d $loopDevice


./scripts/raw-to-vmdk.sh disk.img release/bootvacio.vmdk
./scripts/copy.sh SqueakNOS release/bootvacio.vmdk



