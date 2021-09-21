#!/bin/bash

_1BL_KEY=`cat 1BL.txt`

clear
make -j$(nproc)
# debug with GDB
# gdb -ex run --directory cryptopp --args "xdkbuild" "flashdmp.bin" $_1BL_KEY "output.bin"
# release build
./xdkbuild "flashdmp.bin" $_1BL_KEY "output.bin"
