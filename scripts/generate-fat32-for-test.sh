#!/bin/bash

# this script generates a raw FAT32 file that is to be used in FAT32 filesystem tests inside the image.
# The contents of the generated file should be sinchronised with the contents that test cases expect.

sizeMB=33 # if smaller than this gparted will fail
generatedFileName="testdata/ExampleFAT32.raw"

size=$(echo $(($sizeMB*1024*1024/512)))                     # set size of disk
dd if=/dev/zero of=$generatedFileName bs=512 count=$size    # equivalent to: qemu-img create -f raw harddisk.img 100M
parted $generatedFileName mktable msdos                     # create partition table
parted $generatedFileName "mkpart primary fat32 1 -0"       # make primary partition, type fat32 from 1 to end
parted $generatedFileName mkfs y 1 fat32                    # make fat32 filesystem on partition 1, without confirmation
parted $generatedFileName toggle 1 boot                     # make partition 1 bootable
parted $generatedFileName unit b print

offset=$(parted $generatedFileName unit b print | tail -2 | head -1 | cut -f 1 --delimit="B" | cut -c 9-) 

echo $offset

loopDevice=$(losetup -f)
losetup -o $offset $loopDevice $generatedFileName
mount -t vfat $loopDevice ./mount/

# create / copy files
touch mount/empty.txt
echo zaraza >mount/ascii
echo 123456789 >mount/asciinumbers

echo " " >mount/morethanonesector
for i in {1..100}
do
   echo "1234567890abc" >>mount/morethanonesector
done

echo " " >mount/morethanonecluster
for i in {1..10000}
do
   echo "1234567890abc" >>mount/morethanonecluster
done

mkdir mount/dira
echo a >mount/dira/file.txt

mkdir mount/dirlongname
echo a >mount/dirlongname/file.txt

umount ./mount/

losetup -d $loopDevice
