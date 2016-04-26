
export PATH=$PATH:'/home/dosgo/OpenWrt-SDK/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin'
export STAGING_DIR="/home/dosgo/OpenWrt-SDK/staging_dir"
mkdir build-mips
rm -rf build-mips/main.o
rm -rf build-mips/cJSON.o
rm -rf build-mips/sendmsg.o
rm -rf build-mips/sslbio.o
rm -rf build-mips/nonblocking.o
rm -rf build-mips/openssldl.o
rm -rf build-mips/ngrokc

CC=../usr/bin/mips-openwrt-linux-uclibc-g++
YH="-Wall -fexceptions -DOPENSSL=1 -DOPENSSLDL=1"
$CC $YH -c  sendmsg.cpp -o build-mips/sendmsg.o
$CC $YH -c  cJSON.c -o build-mips/cJSON.o
$CC $YH -c  main.cpp -o build-mips/main.o
$CC $YH -c  nonblocking.cpp  -o build-mips/nonblocking.o
$CC $YH -c  sslbio.cpp  -o build-mips/sslbio.o
$CC $YH -c  openssldl.cpp  -o build-mips/openssldl.o
$CC $YH -c  ngrok.cpp  -o build-mips/ngrok.o
$CC -s build-mips/main.o build-mips/cJSON.o build-mips/sendmsg.o build-mips/nonblocking.o build-mips/ngrok.o build-mips/sslbio.o  build-mips/openssldl.o -o build-mips/ngrokc   









