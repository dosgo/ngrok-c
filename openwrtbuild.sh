#!/bin/sh
export PATH=$PATH:'/home/dosgo/OpenWrt-SDK/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin'
export STAGING_DIR="/home/dosgo/OpenWrt-SDK/staging_dir"
DIR=build-mips
mkdir $DIR
rm -rf $DIR/main.o
rm -rf $DIR/cJSON.o
rm -rf $DIR/sendmsg.o
rm -rf $DIR/sslbio.o
rm -rf $DIR/nonblocking.o
rm -rf $DIR/ngrokc

CC=mips-openwrt-linux-g++

$CC -c $1 sendmsg.cpp -o $DIR/sendmsg.o
$CC -c $1 cJSON.c -o $DIR/cJSON.o
$CC -c $1 main.cpp -o $DIR/main.o
$CC -c $1 nonblocking.cpp -g -o $DIR/nonblocking.o
$CC -c $1 sslbio.cpp -g -o $DIR/sslbio.o
$CC $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/nonblocking.o  $DIR/sslbio.o libpolarssl-mips.a -o $DIR/ngrokc-polarssl   -lpthread 

#buill openssl
#CC=mips-openwrt-linux-gcc
#CXX=mips-openwrt-linux-g++
#AR=mips-openwrt-linux-ar
#RANLIB=mips-openwrt-linux-ranlib 
#./Configure no-asm shared --prefix=`pwd`/../out/openssl linux-mips32
#make
#make install
