#ifndef NONBLOCKING_H_INCLUDED
#define NONBLOCKING_H_INCLUDED
#include "config.h"
#include <pthread.h>
#if  WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <list>

#include "sslbio.h"
using namespace std;


#if OPENSSL
struct sockinfo
{
    openssl_info *sslinfo;
    int isssl;
    int isconnect;
    int linkunixtime;
    int istype; //1=remote 2=local,3=cmd
    int tosock;
    char *packbuf;
    int packbuflen;
    int isconnectlocal;
    int isauth;
    int pongtime;
};
#else
struct sockinfo
{
    ssl_info *sslinfo;
    int isssl;
    int isconnect;
    int linkunixtime;
    int istype; //1=remote 2=local
    int tosock;
    char *packbuf;
    int packbuflen;
    int isconnectlocal;
    int isauth;
    int pongtime;
};
#endif

int setnonblocking(int sServer,int _nMode);
int net_dns( struct sockaddr_in *server_addr, const char *host, int port );
int check_sock(int sock);
void clearsock(int sock,sockinfo * sock_info);
#endif // NONBLOCKING_H_INCLUDED
