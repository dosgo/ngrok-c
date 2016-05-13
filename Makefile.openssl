CC?=gcc
CXX?=g++
CFLAGS+=-fexceptions -DOPENSSL=1 -O2
CXXFLAGS=-fexceptions -DOPENSSL=1 -O2
LIBS=sendmsg.o openssldl.o cJSON.o nonblocking.o sslbio.o ngrok.o main.o
all:: ngrokc $(LIBS)
ngrokc: $(LIBS)
	$(CXX) -s $(LIBS) -o ngrokc -lssl -lcrypto -ldl
sendmsg.o: sendmsg.h
openssldl.o: openssldl.h
cJSON.o: cJSON.h
nonblocking.o: nonblocking.h
sslbio.o: sslbio.h
ngrok.o: ngrok.h
main.o: sendmsg.h openssldl.h sslbio.h ngrok.h cJSON.h bytestool.h nonblocking.h

clean:
	rm -f $(LIBS) ngrokc
