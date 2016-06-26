rem set MINGW_PATH=D:/MinGW
rem set C_INCLUDE_PATH=%MINGW_PATH%/include;%MINGW_PATH%/lib/gcc/mingw32/3.4.5/include
set OPATH=build-win32
set CC=g++
set YH=-Wall -fexceptions -O3 -DOPENSSL=1 -DOPENSSLDL=0
mkdir %OPATH%
del %OPATH%\*.o
%CC%  %YH% -c cJSON.c -o  %OPATH%/cJSON.o
%CC%  %YH% -c main.cpp -o  %OPATH%/main.o
%CC%  %YH% -c sendmsg.cpp -o  %OPATH%/sendmsg.o
%CC%  %YH% -c nonblocking.cpp -o %OPATH%/nonblocking.o
%CC%  %YH% -c sslbio.cpp -o %OPATH%/sslbio.o
%CC%  %YH%  -c ngrok.cpp  -o %OPATH%/ngrok.o

rem %CC%  %OPATH%\main.o %OPATH%\sendmsg.o %OPATH%\cJSON.o  %OPATH%\nonblocking.o %OPATH%\sslbio.o -o  %OPATH%\ngrokc.exe -L"lib"  "libpolarssl-win32.a"  -lws2_32  -lpthread

 %CC% -s %OPATH%\main.o %OPATH%\sendmsg.o  %OPATH%\ngrok.o %OPATH%\cJSON.o  %OPATH%\nonblocking.o %OPATH%\sslbio.o -o  %OPATH%\ngrokc.exe  libpolarssl-win32.a  -lws2_32  -lpthread


 
