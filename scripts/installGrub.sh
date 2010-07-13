#!/bin/sh

basename=$1
raw="${basename}.raw"
filename="${basename}.vmdk"

./scripts/create-disk.sh 100 $raw

loopDevice=$(losetup -f)
echo $loopDevice
losetup -o 0 $loopDevice $raw

offset=$(parted $loopDevice unit b print | tail -2 | head -1 | cut -f 1 --delimit="B" | cut -c 9-) 

loopDevice2=$(losetup -f)
losetup -o $offset $loopDevice2 $loopDevice

mount -t vfat $loopDevice2 mount/

mkdir mount/boot
mkdir mount/boot/grub
cp -r boot/grub/stage1 boot/grub/stage2 boot/grub/menu.lst mount/boot/grub

grub --device-map=/dev/null
#look for intructions on http://www.omninerd.com/articles/Installing_GRUB_on_a_Hard_Disk_Image_File
#device (hd0) $1
#root (hd0,0)
#setup (hd0)
umount mount/

losetup -d $loopDevice2
losetup -d $loopDevice

./scripts/raw-to-vmdk.sh $raw $filename
mv $filename vmdk/squeakNOS.vmdk
rm $raw
./scripts/image2vmdk.sh
./scripts/kernel2vmdk.sh
