#this build polarssl
export PATH=$PATH:'/home/dosgo/OpenWrt-SDK/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin'
export STAGING_DIR="/home/dosgo/OpenWrt-SDK/staging_dir"
make clean
make CC=mips-openwrt-linux-gcc CXX=mips-openwrt-linux-g++ AR=mips-openwrt-linux-ar RANLIB=mips-openwrt-linux-ranlib




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
