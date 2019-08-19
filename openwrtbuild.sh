#!/bin/sh
###路径自行更改为SDK实际路径
export PATH=$PATH:'/root/op/staging_dir/toolchain-mipsel_1004kc+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin'
export STAGING_DIR="/root/op/staging_dir"
DIR=build-mips
mkdir $DIR
rm -rf $DIR/main.o
rm -rf $DIR/cJSON.o
rm -rf $DIR/sendmsg.o
rm -rf $DIR/udp.o
rm -rf $DIR/polarsslbio.o
rm -rf $DIR/nonblocking.o
rm -rf $DIR/ngrokc

###编译器改成你SDK的
CC=mipsel-openwrt-linux-uclibc-g++
YH="-Wall -fexceptions -DOPENSSL=0 -DISMBEDTLS=1 -O2"
$CC $YH -c $1 sendmsg.cpp -o $DIR/sendmsg.o -L/root/ngrok-c -lmbedtls -lmbedx509 -lmbedcrypto
$CC $YH -c $1 udp.cpp -o $DIR/udp.o -L/root/ngrok-c -lmbedtls -lmbedx509 -lmbedcrypto
$CC $YH -c $1 base64.c -o $DIR/base64.o -L/root/ngrok-c -lmbedtls -lmbedx509 -lmbedcrypto
$CC $YH -c $1 cJSON.c -o $DIR/cJSON.o -L/root/ngrok-c -lmbedtls -lmbedx509 -lmbedcrypto
$CC $YH -c $1 main.cpp -o $DIR/main.o -L/root/ngrok-c -lmbedtls -lmbedx509 -lmbedcrypto
$CC $YH -c $1 nonblocking.cpp  -o $DIR/nonblocking.o -L/root/ngrok-c -lmbedtls -lmbedx509 -lmbedcrypto
$CC $YH -c $1 polarsslbio.cpp  -o $DIR/polarsslbio.o -L/root/ngrok-c -lmbedtls -lmbedx509 -lmbedcrypto
$CC $YH -c $1 global.cpp  -o $DIR/global.o -L/root/ngrok-c -lmbedtls -lmbedx509 -lmbedcrypto
$CC $YH -c $1 ngrok.cpp  -o $DIR/ngrok.o -L/root/ngrok-c -lmbedtls -lmbedx509 -lmbedcrypto
$CC -s $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/udp.o $DIR/base64.o $DIR/nonblocking.o $DIR/global.o $DIR/ngrok.o $DIR/polarsslbio.o libmbedtls.a  libmbedx509.a libmbedcrypto.a -o $DIR/ngrokc

