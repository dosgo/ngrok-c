#ifndef NONBLOCKING_H_INCLUDED
#define NONBLOCKING_H_INCLUDED
#include "config.h"
#include <pthread.h>
#include <string.h>
#if  WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
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

inline int setnonblocking(int sServer,int _nMode)
{
    #if WIN32
    DWORD nMode = _nMode;
    return ioctlsocket( sServer, FIONBIO,&nMode);
    #else
    if(_nMode==1)
    {
       return fcntl(sServer,F_SETFL,O_NONBLOCK);
    }
    else
    {
      return fcntl(sServer,F_SETFL, _nMode);
    }
    #endif
}
inline int net_dns( struct sockaddr_in *server_addr, const char *host, int port )
{
    struct hostent *server_host;
    if((server_host = gethostbyname(host)) == NULL )
    {
        return -1;
    }
    memcpy((void*)&server_addr->sin_addr,(void*)server_host->h_addr,server_host->h_length);
    server_addr->sin_family = AF_INET;
    server_addr->sin_port   = htons( port );
    return 0;
}

inline int check_sock(int sock)
{
    int error=-1;
    #if WIN32
    int len ;
    #else
    socklen_t len;
    #endif
    len = sizeof(error);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
    return error;
}

void clearsock(int sock,sockinfo * sock_info);
#endif // NONBLOCKING_H_INCLUDED
