#!/bin/bash

set -e

# Build rumprun in guest environment.

if [ ! -f /etc/vagrant_provisioned ]; then
    echo "Seems to be in host environment."
    exit 1
fi

CC=x86_64-linux-gnu-gcc
# CC=aarch64-linux-gnu-gcc
TARGET=${CC%-gcc}

# sudo DEBIAN_FRONTEND=noninteractive apt -qq install -y \
#     make \
#     gcc \
#     g++ \
#     zlib1g-dev \
#     qemu-system-x86 \
#     qemu-system-aarch64 \
#     gcc-aarch64-linux-gnu

mkdir_p() {
    for d in $@; do
        if [ ! -e $d ]; then
            mkdir $d
        fi
    done
}

rr_build="$HOME/rumprun-build" # can not create hard link in synced dir
mkdir_p $rr_build

rr_obj="../../rumprun-build/obj-$TARGET" # seems to accept only relative paths
rr_stage="../../rumprun-build/stage-$TARGET"
mkdir_p $rr_obj $rr_stage

CC=$CC ./build-rr.sh -j2 -o $rr_obj -d $rr_stage hw $@

cat > guest-bashrc << EOF
export PATH="\$HOME/rumprun-build/stage-$TARGET/bin:\$PATH"
export RUMPRUN_TOOLCHAIN_TUPLE=${TARGET%%-*}-rumprun-netbsd
EOF

if ! grep guest-bashrc ~/.bashrc; then
    echo "source $PWD/guest-bashrc" >> ~/.bashrc
fi
