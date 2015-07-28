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

$CC -c sendmsg.cpp -o $DIR/sendmsg.o
$CC -c cJSON.c -o $DIR/cJSON.o
$CC -c main.cpp -o $DIR/main.o
$CC -c nonblocking.cpp -o $DIR/nonblocking.o
$CC -c sslbio.cpp -o $DIR/sslbio.o


$CC $DIR/main.o $DIR/cJSON.o $DIR/sendmsg.o $DIR/nonblocking.o $DIR/sslbio.o  -o $DIR/ngrokc -lssl -lcrypto -lpthread
