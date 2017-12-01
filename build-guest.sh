#!/bin/bash

# Build rumprun in guest environment.

CC=x86_64-linux-gnu-gcc
TARGET=x86_64-linux-gnu
# CC=aarch64-linux-gnu-gcc
# TARGET=aarch64-linux-gnu

# sudo DEBIAN_FRONTEND=noninteractive apt -qq install -y \
#     make \
#     gcc \
#     g++ \
#     zlib1g-dev \
#     qemu-system-x86 \
#     qemu-system-aarch64 \
#     gcc-aarch64-linux-gnu

mkdir_p() {
    if [ ! -e $1 ]; then
        mkdir $1
    fi
}

rr_build="$HOME/rumprun-build" # can not create hard link in synced dir
mkdir_p $rr_build

rr_obj="../../rumprun-build/obj-$TARGET" # seems to accept only relative paths
mkdir_p $rr_obj
rr_stage="../../rumprun-build/stage-$TARGET"
mkdir_p $rr_stage

CC=$CC ./build-rr.sh -j2 -o $rr_obj -d $rr_stage hw $@

cat > guest-bashrc << EOF
export PATH="\$HOME/rumprun-build/stage-$TARGET/bin:\$PATH"
export RUMPRUN_TOOLCHAIN_TUPLE=${TARGET%%-*}-rumprun-netbsd
EOF

if ! grep guest-bashrc ~/.bashrc; then
    echo "source $PWD/guest-bashrc" >> ~/.bashrc
fi
