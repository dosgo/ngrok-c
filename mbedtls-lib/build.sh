#!/bin/sh
export PATH=$PATH:'/home/dosgo/OpenWrt-SDK/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin'
export STAGING_DIR="/home/dosgo/OpenWrt-SDK/staging_dir"
make CC=mips-openwrt-linux-gcc AR=mips-openwrt-linux-ar RANLIB=mips-openwrt-linux-ranlib CXX=mips-openwrt-linux-g++ LD=mips-openwrt-linux-ld