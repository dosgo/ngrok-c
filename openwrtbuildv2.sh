#!/bin/sh
export PATH=$PATH:'/root/mt76x8sdk/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin'
export STAGING_DIR="/root/mt76x8sdk/staging_dir"

DIR=build-mips
mkdir $DIR
rm -rf $DIR/main.o
rm -rf $DIR/cJSON.o
rm -rf $DIR/sendmsg.o
rm -rf $DIR/param.o
rm -rf $DIR/polarsslbio.o
rm -rf $DIR/nonblocking.o
rm -rf $DIR/ngrokc

CC=mipsel-openwrt-linux-g++
YH="-Wall -fexceptions -DOPENSSL=1 -DOPENSSLDL=1   -O2 -ldl"
$CC $YH -c $1 sendmsg.cpp -o $DIR/sendmsg.o
$CC $YH -c $1 param.cpp -o $DIR/param.o
$CC $YH -c $1 udp.cpp -o $DIR/udp.o
$CC $YH -c $1 cJSON.c -o $DIR/cJSON.o
$CC $YH -c $1 main.cpp -o $DIR/main.o
$CC $YH -c $1 nonblocking.cpp  -o $DIR/nonblocking.o
$CC $YH -c $1 opensslbio.cpp  -o $DIR/opensslbio.o
$CC $YH -c $1 openssldl.cpp  -o $DIR/openssldl.o
$CC $YH -c $1 global.cpp  -o $DIR/global.o
$CC $YH -c $1 ngrok.cpp  -o $DIR/ngrok.o
$CC $YH -c $1 base64.c  -o $DIR/base64.o
$CC -s $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/param.o $DIR/nonblocking.o $DIR/global.o $DIR/opensslbio.o  $DIR/ngrok.o $DIR/openssldl.o  $DIR/udp.o $DIR/base64.o   -o $DIR/ngrokc

