#!/bin/sh
DIR=build
mkdir -p $DIR

export  MY_INCLUDE=$(pwd)/include
export  C_INCLUDE_PATH=$MY_INCLUDE
export  CPLUS_INCLUDE_PATH=$MY_INCLUDE
rm -rf $DIR/main.o
rm -rf $DIR/cJSON.o
rm -rf $DIR/sendmsg.o
rm -rf $DIR/nonblocking.o
rm -ff $DIR/udp.o
rm -rf $DIR/base64.o
rm -rf $DIR/ngrokc
rm -rf $DIR/opensslbio.o
rm -rf $DIR/openssldl.o
rm -rf $DIR/param.o
CC=g++
YH="-g  -rdynamic -DOPENSSL=1 -DOPENSSLDL=1"
$CC  $YH  -c sendmsg.cpp -o $DIR/sendmsg.o
$CC  $YH  -c param.cpp -o $DIR/param.o
$CC  $YH  -c openssldl.cpp -o $DIR/openssldl.o
$CC  $YH -c cJSON.c -o $DIR/cJSON.o
$CC  $YH -c main.cpp -o $DIR/main.o
$CC  $YH  -c nonblocking.cpp -o $DIR/nonblocking.o
$CC  $YH  -c global.cpp -o $DIR/global.o
$CC  $YH -c opensslbio.cpp -o $DIR/opensslbio.o
$CC  $YH -c base64.c -o $DIR/base64.o 
$CC  $YH -c udp.cpp -o $DIR/udp.o
$CC  $YH -c ngrok.cpp  -o $DIR/ngrok.o


$CC $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/param.o $DIR/nonblocking.o $DIR/global.o  $DIR/opensslbio.o $DIR/udp.o $DIR/base64.o $DIR/ngrok.o $DIR/openssldl.o -o $DIR/ngrokc $YH   -ldl
