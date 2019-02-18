#ifndef UDP_H_INCLUDED
#define UDP_H_INCLUDED
#include <string>
#include <map>
#include <time.h>
#include <list>
#if WIN32
#include <winsock.h>//这个得放bytestool.h前面
#endif
#include "string.h"
#include "cJSON.h"
#include "sendmsg.h"
#include "base64.h"
#include "bytestool.h"


using namespace std;

int SendUdpReqTunnel(int sock, struct sockaddr_in servAddr,TunnelInfo *tunnelinfo);
int SendUdpPack(int sock,  struct sockaddr_in servAddr,const char *msgstr);
int SendUdpPing(int sock, struct sockaddr_in servAddr);
int CheckUdpPing(int sock, struct sockaddr_in servAddr);
int SendUdpAuth(int sock,struct sockaddr_in servAddr);
int SendUdpProxy(int sock,struct sockaddr_in servAddr,char* Data,char* Url,const char* ClientAddr);
int GetUdpRemoteAddr(int localport,char *url);
int UdpClient();
int initUdp();
int CheckUdpPing(int sock);
int CheckUdpAuth(int sock);
int CheckRegTunnel(int sock);
int UdpRecv(fd_set* readSet);
#endif // UDP_H_INCLUDED
