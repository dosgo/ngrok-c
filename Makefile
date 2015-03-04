.PHONY: all clean

all : backtrace.dll test.exe

backtrace.dll : backtrace.c
	gcc -O2 -shared -Wall -o $@ $^ -lbfd -lintl -liberty -limagehlp

test.exe : test.c
	gcc -g -Wall -o $@ $^

clean :
	-del -f backtrace.dll test.exe