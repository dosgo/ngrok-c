#!/bin/sh
DIR=build
mkdir -p $DIR

## If both openssl 1.0.X and 1.1.X libraries exist, libssl.so/libcrypto.so will point to 1.1 version
## but currently ngrokc-c does not support openssl 1.1
## so we force the link to 1.0.X library
sed -i '/dlopen/s/libssl.so"/libssl.so.1.0.0"/' openssldl.cpp
sed -i '/dlopen/s/libcrypto.so"/libcrypto.so.1.0.0"/' openssldl.cpp

rm -rf $DIR/main.o
rm -rf $DIR/cJSON.o
rm -rf $DIR/sendmsg.o
rm -rf $DIR/nonblocking.o
rm -ff $DIR/udp.o
rm -rf $DIR/base64.o
rm -rf $DIR/ngrokc
rm -rf $DIR/opensslbio.o
rm -rf $DIR/openssldl.o
CC=g++
YH="-g  -rdynamic -DOPENSSL=1 -DOPENSSLDL=1"
$CC  $YH  -c sendmsg.cpp -o $DIR/sendmsg.o
$CC  $YH  -c openssldl.cpp -o $DIR/openssldl.o
$CC  $YH -c cJSON.c -o $DIR/cJSON.o
$CC  $YH -c main.cpp -o $DIR/main.o
$CC  $YH  -c nonblocking.cpp -o $DIR/nonblocking.o
$CC  $YH  -c global.cpp -o $DIR/global.o
$CC  $YH -c opensslbio.cpp -o $DIR/opensslbio.o
$CC  $YH -c base64.c -o $DIR/base64.o 
$CC  $YH -c udp.cpp -o $DIR/udp.o
$CC  $YH -c ngrok.cpp  -o $DIR/ngrok.o


$CC $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/nonblocking.o $DIR/global.o  $DIR/opensslbio.o $DIR/udp.o $DIR/base64.o $DIR/ngrok.o $DIR/openssldl.o -o $DIR/ngrokc $YH  -lpthread -ldl
