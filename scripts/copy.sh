#!/bin/sh -x

imageFile=SqueakNOS
if [ "$#" -ge "1" ]; then
  imageFile=$1
fi

destFile=release/bootdisk.vmdk
if [ "$#" -ge "2" ]; then
  destFile=$2
fi

vmware-mount $destFile release/mount
sudo cp ${imageFile}.image release/mount/SqueakNOS.image
sudo cp ${imageFile}.changes release/mount/SqueakNOS.changes
sudo cp boot/iso.template/SqueakNOS.kernel release/mount/SqueakNOS.kernel
vmware-mount -k $destFile
