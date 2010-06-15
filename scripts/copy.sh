#!/bin/sh -x

imageFile=SqueakNOS
if [ "$#" -ge "1" ]; then
  imageFile=$1
fi

destFile=release/bootdisk.vmdk
if [ "$#" -ge "2" ]; then
  destFile=$2
fi

sourcesFile=SqueakV41
if [ "$#" -ge "3"]; then
  sourcesFile=$3
fi

vmware-mount $destFile release/mount
sudo cp ${imageFile}.image release/mount/SqueakNOS.image
sudo cp ${imageFile}.changes release/mount/SqueakNOS.changes
sudo cp boot/iso.template/SqueakNOS.kernel release/mount/SqueakNOS.kernel
sudo cp ${sourcesFile}.sources release/mount/${sourcesFile}.sources
vmware-mount -k $destFile
