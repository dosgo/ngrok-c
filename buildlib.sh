#this build polarssl
###修改成SDK路径
export PATH=$PATH:'/root/op/staging_dir/toolchain-mipsel_1004kc+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin'
export STAGING_DIR="/root/op/staging_dir"
make clean
###修改成SDK编译器
make CC=mipsel-openwrt-linux-uclibc-gcc CXX=mipsel-openwrt-linux-uclibc-g++ AR=mipsel-openwrt-linux-uclibc-ar RANLIB=mipsel-openwrt-linux-uclibc-ranlib




#buill openssl
#export PATH=$PATH:'/home/dosgo/OpenWrt-SDK/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin'
#export STAGING_DIR="/home/dosgo/OpenWrt-SDK/staging_dir"
#CC=mips-openwrt-linux-gcc
#CXX=mips-openwrt-linux-g++
#AR=mips-openwrt-linux-ar
#RANLIB=mips-openwrt-linux-ranlib 
#./Configure no-asm shared --prefix=`pwd`/../out/openssl linux-mips32
#make clean
#make
#make install
