#!/bin/bash

set -e

# Build rumprun in guest environment.

if [ ! -d /vagrant ]; then
    echo "# Error: Seems to be in host environment."
    exit 1
fi

# CC=x86_64-linux-gnu-gcc
# CC=aarch64-linux-gnu-gcc
CC=arm-linux-gnueabihf-gcc
TARGET=${CC%-gcc}

# sudo DEBIAN_FRONTEND=noninteractive apt -qq install -y \
#     make \
#     gcc \
#     zlib1g-dev \
#     qemu-system-x86 \
#     qemu-system-aarch64 \
#     gcc-aarch64-linux-gnu \
#     gcc-arm-linux-gnueabihf

mkdir_p() {
    for d in $@; do
        if [ ! -e $d ]; then
            mkdir $d
        fi
    done
}

rr_build=../../rumprun-build   # can not create hard link in synced dir
mkdir_p $rr_build

rr_obj=$rr_build/obj-$TARGET
rr_stage=$rr_build/stage-$TARGET
mkdir_p $rr_obj $rr_stage

CC=$CC ./build-rr.sh -j2 -o $rr_obj -d $rr_stage hw $@

rr_bashrc=rr-bashrc

cat > $rr_build/$rr_bashrc << EOF
export PATH="$(cd $rr_stage; pwd)/bin:\$PATH"
export RUMPRUN_TOOLCHAIN_TUPLE=${TARGET%%-*}-rumprun-netbsd
EOF

shrc=$HOME/.${SHELL##*/}rc
if ! grep $rr_bashrc $shrc; then
    echo "source $(cd $rr_build; pwd)/$rr_bashrc" >> $shrc
fi
