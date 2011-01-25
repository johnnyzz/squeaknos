#!/bin/sh -x


help() {
    cat <<EOF
Usage: $0 [options...]
Options include:
  --help                print this message and then exit
  -i, --image=<file>     copy <file>.changes and <file>.image
  -s, --sources=<file>   copy <file>
  -d, --dest=<file>     use <file> as target disk image file.
EOF
    stop=true
}

#default params:

imageFile=SqueakNOS
destFile=release/bootdisk.vmdk
sourcesFile=SqueakV41

while [ $# -gt 0 ]; do
    case "$1" in
	--help)			help; exit 0;;
	--image=*)		imageFile="`echo \"$1\" | sed 's/\-\-image=//'`";;
	-i=*)			imageFile="`echo \"$1\" | sed 's/\-i=//'`";;
	--sources=*)	sourcesFile="`echo \"$1\" | sed 's/\-\-sources=//'`";;
	-s=*)			sourcesFile="`echo \"$1\" | sed 's/\-s=//'`";;
	--dest=*)		destFile="`echo \"$1\" | sed 's/\-\-dest=//'`";;
	-d=*)			destFile="`echo \"$1\" | sed 's/\-d=//'`";;
	*)			error "unknown option: $1";;
    esac
    shift
done


vmware-mount $destFile disks/mount
sudo cp "${imageFile}.image" -T disks/mount/SqueakNOS.image
sudo cp "${imageFile}.changes" -T disks/mount/SqueakNOS.changes
sudo cp release/SqueakNOS.kernel disks/mount/SqueakNOS.kernel
sudo cp ${sourcesFile}.sources disks/mount/${sourcesFile}.sources
sudo cp SqueakNOS.config disks/mount/SqueakNOS.config
sleep 2
vmware-mount -k $destFile
