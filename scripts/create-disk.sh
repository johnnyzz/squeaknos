#!/bin/sh

# this script is used to create a disk image. I'd like it to be able to generate raw, vmdk and vdi images.

if [ $# -eq 0 ]
then
	echo Usage: `basename $0` size-in-mb disk-image-file
	echo Example: `basename $0` 33 disk.img
	exit
fi

if [ $1 -le 32 ]
then
	echo "Minium fat32 size is 65527 clusters, which gives around 32MB."
	echo "So, you have to use disk sizes of over 32MB."
	exit
fi

sizeMB=$1
imageFile=$2

size=$(echo $(($sizeMB*1024*1024/512)))               # set size of disk
dd if=/dev/zero of=$imageFile bs=512 count=$size    # equivalent to: qemu-img create -f raw harddisk.img 100M
parted $imageFile mktable msdos                     # create partition table
parted $imageFile "mkpart primary fat32 2 -0"             # make primary partition, type fat32 from 1 to end
parted $imageFile mkfs y 1 fat32                    # make fat32 filesystem on partition 1, without confirmation
parted $imageFile toggle 1 boot                     # make partition 1 bootable
parted $imageFile unit b print
