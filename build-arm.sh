#!/bin/bash

set -e

CC=arm-linux-gnueabihf-gcc
TARGET=${CC%-gcc}

mkdir_p() {
    for d in $@; do
        if [ ! -e $d ]; then
            mkdir $d
        fi
    done
}

rr_build=../rumprun-build
mkdir_p $rr_build

rr_obj=$rr_build/obj-$TARGET
rr_stage=$rr_build/stage-$TARGET
mkdir_p $rr_obj $rr_stage

CC=$CC ./build-rr.sh -j4 -o $rr_obj -d $rr_stage hw -- -F ACLFLAGS=-no-pie

rr_bashrc=rr-bashrc

cat > $rr_build/$rr_bashrc << EOF
export PATH="$(cd $rr_stage; pwd)/bin:\$PATH"
export RUMPRUN_TOOLCHAIN_TUPLE=${TARGET%%-*}-rumprun-netbsd
EOF

shrc=$HOME/.${SHELL##*/}rc
if ! grep $rr_bashrc $shrc; then
    echo "source $(cd $rr_build; pwd)/$rr_bashrc" >> $shrc
fi
