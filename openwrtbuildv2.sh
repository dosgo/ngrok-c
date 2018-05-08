#!/bin/sh
export PATH=$PATH:'/root/ledesdk/staging_dir/toolchain-mips_24kc_gcc-5.4.0_musl-1.1.16/bin'
export STAGING_DIR="/root/ledesdk/staging_dir"
DIR=build-mips
mkdir $DIR
rm -rf $DIR/main.o
rm -rf $DIR/cJSON.o
rm -rf $DIR/sendmsg.o
rm -rf $DIR/polarsslbio.o
rm -rf $DIR/nonblocking.o
rm -rf $DIR/ngrokc

CC=mips-openwrt-linux-g++
YH="-Wall -fexceptions -DOPENSSL=1 -DOPENSSLDL=1   -O2"
$CC $YH -c $1 sendmsg.cpp -o $DIR/sendmsg.o
$CC $YH -c $1 cJSON.c -o $DIR/cJSON.o
$CC $YH -c $1 main.cpp -o $DIR/main.o
$CC $YH -c $1 nonblocking.cpp  -o $DIR/nonblocking.o
$CC $YH -c $1 opensslbio.cpp  -o $DIR/opensslbio.o
$CC $YH -c $1 openssldl.cpp  -o $DIR/openssldl.o
$CC $YH -c $1 global.cpp  -o $DIR/global.o
$CC $YH -c $1 ngrok.cpp  -o $DIR/ngrok.o
$CC -s $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/nonblocking.o $DIR/global.o $DIR/opensslbio.o  $DIR/ngrok.o $DIR/openssldl.o    -o $DIR/ngrokc

