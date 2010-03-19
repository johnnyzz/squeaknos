#!/bin/bash -x

# this script generates a vmdk from a raw image

if [ $# -eq 0 ]
then
	echo Usage: `basename $0` raw-image vmdk-image
	echo Example: `basename $0` disk.img disk.vmdk
	exit
fi

rawFile=$1
vmdkFile=$2

qemu-img convert -f raw $rawFile -O vmdk $vmdkFile
