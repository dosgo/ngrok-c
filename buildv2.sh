#!/bin/sh
DIR=build
mkdir $DIR

rm -rf $DIR/main.o
rm -rf $DIR/cJSON.o
rm -rf $DIR/sendmsg.o
rm -rf $DIR/nonblocking.o
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
$CC  $YH -c opensslbio.cpp -o $DIR/opensslbio.o

$CC  $YH -c ngrok.cpp  -o $DIR/ngrok.o


$CC $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/nonblocking.o $DIR/opensslbio.o   $DIR/ngrok.o $DIR/openssldl.o -o $DIR/ngrokc $YH  -lpthread -ldl
