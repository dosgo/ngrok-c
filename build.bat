set MY_INCLUDE=%~dp0include
set C_INCLUDE_PATH=%MY_INCLUDE%
set CPLUS_INCLUDE_PATH=%MY_INCLUDE%
set OPATH=build-win32
set CC=g++
set YH=-g -DOPENSSL=1 -DOPENSSLDL=1
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
%CC%  %YH%  -c param.cpp  -o %OPATH%/param.o

rem %CC%  %OPATH%\main.o %OPATH%\sendmsg.o %OPATH%\cJSON.o %OPATH%/opensslbio.o  %OPATH%\base64.o %OPATH%\udp.o %OPATH%\nonblocking.o  -o  %OPATH%\ngrokc.exe -L"lib"  "libpolarssl-win32.a"  -lws2_32  -lpthread

 %CC% -s %OPATH%\main.o %OPATH%\sendmsg.o %OPATH%\param.o  %OPATH%\ngrok.o  %OPATH%/opensslbio.o %OPATH%\cJSON.o %OPATH%\base64.o  %OPATH%\udp.o  %OPATH%\nonblocking.o  %OPATH%\openssldl.o %OPATH%\global.o -o  %OPATH%\ngrokc.exe   -lws2_32 


 
