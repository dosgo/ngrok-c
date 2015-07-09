#!/bin/sh
DIR=build
mkdir $DIR

rm -rf $DIR/main.o
rm -rf $DIR/cJSON.o
rm -rf $DIR/sendmsg.o
rm -rf $DIR/sslbio.o
rm -rf $DIR/nonblocking.o
rm -rf $DIR/ngrokc
CC=g++
YH="-Wall -fexceptions -O2"

$CC $YH -c $1 sendmsg.cpp  -o $DIR/sendmsg.o
$CC $YH -c $1 cJSON.c   -o $DIR/cJSON.o
$CC $YH -c $1 nonblocking.cpp  -o $DIR/nonblocking.o
$CC $YH -c $1 sslbio.cpp  -o $DIR/sslbio.o
$CC $YH -c $1 main.cpp  -o $DIR/main.o
#$CC -c forkthread.cpp -o $DIR/forkthread.o

$CC -s $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/nonblocking.o  $DIR/sslbio.o libpolarssl-linux.a -o $DIR/ngrokc  -lpthread
