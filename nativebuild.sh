
export PATH=$PATH:'/home/dosgo/OpenWrt-SDK/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin'
export STAGING_DIR="/home/dosgo/OpenWrt-SDK/staging_dir"
BUILDIR=build-mips
mkdir $BUILDIR
rm -rf $BUILDIR/main.o
rm -rf $BUILDIR/cJSON.o
rm -rf $BUILDIR/sendmsg.o
rm -rf $BUILDIR/sslbio.o
rm -rf $BUILDIR/nonblocking.o
rm -rf $BUILDIR/ngrokc

CC=../usr/bin/mips-openwrt-linux-uclibc-g++
YH="-Wall -fexceptions -DOPENSSL=1 -DOPENSSLDL=1"
$CC $YH -c  sendmsg.cpp -o $BUILDIR/sendmsg.o
$CC $YH -c  cJSON.c -o $BUILDIR/cJSON.o
$CC $YH -c  main.cpp -o $BUILDIR/main.o
$CC $YH -c  nonblocking.cpp  -o $BUILDIR/nonblocking.o
$CC $YH -c  sslbio.cpp  -o $BUILDIR/sslbio.o
$CC $YH -c  ngrok.cpp  -o $BUILDIR/ngrok.o
$CC -s $BUILDIR/main.o $BUILDIR/cJSON.o $BUILDIR/sendmsg.o $BUILDIR/nonblocking.o $BUILDIR/ngrok.o $BUILDIR/sslbio.o  -o $BUILDIR/ngrokc   









