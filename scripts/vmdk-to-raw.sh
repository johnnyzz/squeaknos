#!/bin/sh -x


if [ $# -eq 0 ]; then
  echo "vmdk to raw. Converts a vmdk disk file to a raw one."
  echo "Usage: vmdk-to-raw source.vmdk dest.raw"
fi

qemu-img convert -f vmdk $1 -O raw $2
