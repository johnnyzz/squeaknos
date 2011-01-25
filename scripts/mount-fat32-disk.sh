#!/bin/bash -x

filename="testdata/ExampleFAT32.raw"
if [ $# -eq 1 ]
then
	filename=$1
fi

loopDevice=$(losetup -f)
losetup -o 0 $loopDevice $filename

offset=$(parted $loopDevice unit b print | tail -2 | head -1 | cut -f 1 --delimit="B" | cut -c 9-) 

echo $offset

losetup -d $loopDevice

loopDevice2=$(losetup -f)
losetup -o $offset $loopDevice2 $filename

mount -t vfat $loopDevice2 ./testdata/mount/

# umount ./testdata/mount/
# losetup -d $loopDevice2

