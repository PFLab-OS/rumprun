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
# TOOLCHAIN_TUPLE=${TARGET%%-*}-rumprun-netbsd
TOOLCHAIN_TUPLE=arm-rumprun-netbsdelf-eabihf

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

RR_BUILD=../../rumprun-build   # can not create hard link in synced dir
mkdir_p $RR_BUILD

RR_OBJ=$RR_BUILD/obj-$TARGET
RR_STAGE=$RR_BUILD/stage-$TARGET
mkdir_p $RR_OBJ $RR_STAGE

CC=$CC ./build-rr.sh -j2 -o $RR_OBJ -d $RR_STAGE hw $@

RR_SHRC=rr-bashrc

cat > $RR_BUILD/$RR_SHRC << EOF
export PATH="$(cd $RR_STAGE; pwd)/bin:\$PATH"
export RUMPRUN_TOOLCHAIN_TUPLE=$TOOLCHAIN_TUPLE
EOF

SHRC=$HOME/.${SHELL##*/}rc
if ! grep $RR_SHRC $SHRC; then
    echo "source $(cd $RR_BUILD; pwd)/$RR_SHRC" >> $SHRC
fi

RR_TEST=$RR_BUILD/test
mkdir_p $RR_TEST

cat > $RR_TEST/Makefile << EOF
SRCS = hello.c
PRODUCT = hello

RR_CC = $TOOLCHAIN_TUPLE-gcc
RR_OUT = \$(PRODUCT)-rr.out
BAKERY = rumprun-bake hw_generic
RR_BIN = \$(PRODUCT)-rr.bin

HOST_OUT = \$(PRODUCT)-host.out

.PHONY: all clean

all: \$(RR_BIN) \$(HOST_OUT)

\$(RR_BIN): \$(RR_OUT)
	\$(BAKERY) \$(RR_BIN) \$(RR_OUT)

\$(RR_OUT): \$(SRCS)
	\$(RR_CC) \$(SRCS) -o \$(RR_OUT)

\$(HOST_OUT): \$(SRCS)
	\$(CC) \$(SRCS) -o \$(HOST_OUT)

clean:
	rm -f \$(HOST_OUT) \$(RR_OUT) \$(RR_BIN)
EOF

cat > $RR_TEST/hello.c << EOF
#include <stdio.h>

int main(void) {
    puts("Hello, world!");
    return 0;
}
EOF
