rem set MINGW_PATH=D:/MinGW
rem set C_INCLUDE_PATH=%MINGW_PATH%/include;%MINGW_PATH%/lib/gcc/mingw32/3.4.5/include

del *.o
gcc -c -g cJSON.c -o cJSON.o
g++ -c -g main.cpp -o main.o
g++ -c -g msjexhnd.cpp -o msjexhnd.o
g++ -c -g sendmsg.cpp -o sendmsg.o
g++ -g main.o sendmsg.o cJSON.o msjexhnd.o -o "ngrokpolarssl.exe" -L"lib"  "libpolarssl.a"  -lws2_32  -lpthread