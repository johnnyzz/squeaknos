#!/bin/sh -x

imageFile=SqueakNOS
if [ "$#" -eq "1" ]; then
  imageFile=$1
fi

vmware-mount release/bootdisk.vmdk release/mount
sudo cp ${imageFile}.image release/mount/SqueakNOS.image
sudo cp ${imageFile}.changes release/mount/SqueakNOS.changes
vmware-mount -k release/bootdisk.vmdk
