#!/bin/bash -x

vmware-mount disksbien/SqueakNOS.vmdk disksbien/mount
echo $1 | sudo tee disksbien/mount/SqueakNOS.config
echo $1 > SqueakNOS.config
vmware-mount -k disksbien/SqueakNOS.vmdk
