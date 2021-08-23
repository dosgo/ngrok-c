set MY_INCLUDE=%~dp0include
set C_INCLUDE_PATH=%MY_INCLUDE%
set CPLUS_INCLUDE_PATH=%MY_INCLUDE%

rem aarch64_be-linux-gnu
rem aarch64_be-linux-musl
rem aarch64_be-windows-gnu
rem aarch64-linux-gnu
rem aarch64-linux-musl
rem aarch64-windows-gnu
rem armeb-linux-gnueabi
rem armeb-linux-gnueabihf
rem armeb-linux-musleabi
rem armeb-linux-musleabihf
rem armeb-windows-gnu
rem arm-linux-gnueabi
rem arm-linux-gnueabihf
rem arm-linux-musleabi
rem arm-linux-musleabihf
rem arm-windows-gnu
rem i386-linux-gnu
rem i386-linux-musl
rem i386-windows-gnu
rem mips64el-linux-gnuabi64
rem mips64el-linux-gnuabin32
rem mips64el-linux-musl
rem mips64-linux-gnuabi64
rem mips64-linux-gnuabin32
rem mips64-linux-musl
rem mipsel-linux-gnu
rem mipsel-linux-musl
rem mips-linux-gnu
rem mips-linux-musl
rem powerpc64le-linux-gnu
rem powerpc64le-linux-musl
rem powerpc64-linux-gnu
rem powerpc64-linux-musl
rem powerpc-linux-gnu
rem powerpc-linux-musl
rem riscv64-linux-gnu
rem riscv64-linux-musl
rem s390x-linux-gnu
rem s390x-linux-musl
rem sparc-linux-gnu
rem sparcv9-linux-gnu
rem wasm32-freestanding-musl
rem x86_64-linux-gnu
rem x86_64-linux-gnux32
rem x86_64-linux-musl
rem x86_64-windows-gnu


set CC=zig c++
set TARGET=mipsel-linux-musl
set OPATH=%TARGET%
set YH=-target %TARGET% -Wall -fexceptions -O3 -DOPENSSL=1 -DOPENSSLDL=1
mkdir %OPATH%
del %OPATH%\*.o
%CC%  %YH% -c cJSON.c -o  %OPATH%/cJSON.o
%CC%  %YH% -c main.cpp -o  %OPATH%/main.o
%CC%  %YH% -c sendmsg.cpp -o  %OPATH%/sendmsg.o
%CC%  %YH% -c udp.cpp -o  %OPATH%/udp.o
%CC%  %YH% -c base64.c -o  %OPATH%/base64.o 
%CC%  %YH% -c nonblocking.cpp -o %OPATH%/nonblocking.o
%CC%  %YH% -c global.cpp -o %OPATH%/global.o
%CC%  %YH% -c opensslbio.cpp -o %OPATH%/opensslbio.o
%CC%  %YH% -c openssldl.cpp -o %OPATH%/openssldl.o
%CC%  %YH%  -c ngrok.cpp  -o %OPATH%/ngrok.o

rem %CC%  %OPATH%\main.o %OPATH%\sendmsg.o %OPATH%\cJSON.o %OPATH%/opensslbio.o  %OPATH%\base64.o %OPATH%\udp.o %OPATH%\nonblocking.o  -o  %OPATH%\ngrokc 

 %CC% %YH%  -s %OPATH%\main.o %OPATH%\sendmsg.o  %OPATH%\ngrok.o  %OPATH%/opensslbio.o %OPATH%\cJSON.o %OPATH%\base64.o  %OPATH%\udp.o  %OPATH%\nonblocking.o  %OPATH%\openssldl.o %OPATH%\global.o -o  %OPATH%\ngrokc  


 
