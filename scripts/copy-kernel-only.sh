#!/bin/sh -x


help() {
    cat <<EOF
Usage: $0 [options...]
Options include:
  --help                print this message and then exit
  -d, --dest=<file>     use <file> as target disk image file.
EOF
    stop=true
}

#default params:

destFile=release/bootdisk.vmdk

while [ $# -gt 0 ]; do
    case "$1" in
	--help)			help; exit 0;;
	--dest=*)		destFile="`echo \"$1\" | sed 's/\-\-dest=//'`";;
	-d=*)			destFile="`echo \"$1\" | sed 's/\-d=//'`";;
	*)			error "unknown option: $1";;
    esac
    shift
done


vmware-mount $destFile disks/mount
sudo cp release/SqueakNOS.kernel disks/mount/SqueakNOS.kernel
sleep 2
vmware-mount -k $destFile
