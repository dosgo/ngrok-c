#!/bin/sh
DIR=build
mkdir $DIR

rm -rf $DIR/main.o
rm -rf $DIR/cJSON.o
rm -rf $DIR/sendmsg.o
rm -rf $DIR/nonblocking.o
rm -rf $DIR/ngrokc
rm -rf $DIR/sslbio.o
CC=g++
YH="-g  -rdynamic"
$CC  $YH  -c sendmsg.cpp -o $DIR/sendmsg.o
$CC  $YH -c cJSON.c -o $DIR/cJSON.o
$CC  $YH -c main.cpp -o $DIR/main.o
$CC  $YH  -c nonblocking.cpp -o $DIR/nonblocking.o
$CC  $YH -c sslbio.cpp -o $DIR/sslbio.o
$CC  $YH -c ngrok.cpp  -o $DIR/ngrok.o


$CC $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/nonblocking.o $DIR/sslbio.o  $DIR/ngrok.o -o $DIR/ngrokc $YH -lssl -lcrypto  libpolarssl-linux.a
