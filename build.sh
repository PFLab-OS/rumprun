#!/bin/sh

./build-rr.sh raphine
./rumprun/bin/x86_64-rumprun-netbsd-gcc -o helloer-rumprun helloer.c
./rumprun/bin/rumprun-bake raphine_minimum helloer-rumprun.bin helloer-rumprun
cp helloer-rumprun.bin /vagrant/source/kernel/arch/hw/x86/rump.bin
