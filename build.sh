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

$CC -c $1 sendmsg.cpp -g -o $DIR/sendmsg.o
$CC -c $1 cJSON.c  -g -o $DIR/cJSON.o
$CC -c $1 nonblocking.cpp -g -o $DIR/nonblocking.o
$CC -c $1 sslbio.cpp -g -o $DIR/sslbio.o
$CC -c $1 main.cpp -g -o $DIR/main.o
#$CC -c forkthread.cpp -o $DIR/forkthread.o

$CC $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/nonblocking.o  $DIR/sslbio.o linuxlib/libpolarssl.a -o $DIR/ngrokc  -lpthread
