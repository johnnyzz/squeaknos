#!/bin/bash -x


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

loopDeviceFS=$(losetup -f)
losetup -o $offset $loopDeviceFS $imageFile

loopDeviceRaw=$(losetup -f)
mountDir=./mount/hdd

mkdir ${mountDir}
mount -t vfat $loopDeviceFS ${mountDir}

mkdir ${mountDir}/boot
mkdir ${mountDir}/boot/grub

# copy handmade config file.
cp -r boot/grub/grub.cfg ${mountDir}/boot/grub

echo "(hd0) ${loopDeviceRaw}" >${mountDir}/boot/grub/device.map
echo "(hd0,1) ${loopDeviceFS}" >>${mountDir}/boot/grub/device.map

# now we have to install grub, so it copies it's own files
losetup $loopDeviceRaw $imageFile

MODULES="normal ls cat help ext2 iso9660 reiserfs xfs fat part_msdos part_gpt ata serial memdisk multiboot linux minicmd configfile search tar at_keyboard"

grub-install --modules="${MODULES}" --root-directory=${mountDir} "(hd0)" --recheck

rm ${mountDir}/SqueakNOS.*
cp release/iso/SqueakNOS.* ${mountDir}

umount ${mountDir}

losetup -d $loopDeviceRaw

losetup -d $loopDeviceFS


#./scripts/raw-to-vmdk.sh disk.img release/bootvacio.vmdk
#./scripts/copy.sh SqueakNOS release/bootvacio.vmdk











##### old, USED to generate ISO.

# mkdir /tmp/cdroot
# mkdir /tmp/cdroot/boot
# mkdir /tmp/cdroot/boot/grub
# cp /boot/grub/grub.cfg ./temp/boot/grub
# 
# grub-mkrescue --modules="linux ext2 fshelp ls boot pc" --overlay=cdroot --image-type=cdrom SqueakNOS.iso


