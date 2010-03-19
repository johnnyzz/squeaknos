#!/bin/bash -x

# this script copies sources file to the image disk passed as parameter at the offset passed.
# Usually you'll say something like 30 (which means 30mb), so if the image is ~20mb you get a
# 10mb empty space buffer so they don't clash.

# Example: 
# copy-sources.sh disk.img PharoV10.sources 30


imageFile=$1
sourcesFile=$2
offset=$(echo $(($3*1024*1024)))

# uso loop5 porque me place
losetup -o $offset /dev/loop5 $imageFile

dd if=$sourcesFile of=/dev/loop5

losetup -d /dev/loop5

