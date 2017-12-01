#!/bin/bash

# Build rumprun in guest environment.

# sudo DEBIAN_FRONTEND=noninteractive apt -qq install -y \
#     make \
#     gcc \
#     g++ \
#     zlib1g-dev \
#     qemu

mkdir_p() {
    if [ ! -e $1 ]; then
        mkdir $1
    fi
}

rr_build="$HOME/rumprun-build" # can not create hard link in synced dir
mkdir_p $rr_build

rr_obj="../../rumprun-build/obj" # seems to accept only relative paths
mkdir_p $rr_obj
rr_dist="../../rumprun-build/dist"
mkdir_p $rr_dist

./build-rr.sh -qq -j2 -o $rr_obj -d $rr_dist hw $@

if ! grep guest-bashrc ~/.bashrc; then
    echo "source $PWD/guest-bashrc" >> ~/.bashrc
fi
