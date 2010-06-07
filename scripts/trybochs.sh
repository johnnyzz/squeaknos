#!/bin/bash -x

cp boot/bochsrc.cd release/bochsrc
cd release
~/applications/bochs/bin/bochs -q
cd ..

