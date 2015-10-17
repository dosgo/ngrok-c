#ifndef __SENDMSG_H__
#define __SENDMSG_H__
#include "config.h"
#include <string>
#include <map>
#if OPENSSL
#include "openssl/ssl.h"
#else
#if ISMBEDTLS
#include <mbedtls/ssl.h>
typedef mbedtls_ssl_context ssl_context;

#else
#include "polarssl/ssl.h"
#endif // ISMBEDTLS
#endif

#if WIN32
#include <windows.h>
#else
#include <netinet/in.h>
 typedef long long __int64;
 void milliseconds_sleep( unsigned long mSec);
 #include <string.h>
 #ifndef __UCLIBC__
 #include <sys/select.h>
 #endif
#endif
using namespace std;
struct TunnelInfo
{
    char localhost[255];
    char subdomain[255];
    char hostname[255];
    char httpauth[255];
    int localport;
    struct sockaddr_in local_addr;
    int remoteport;
};

void sleeps(int ti);
int get_curr_unixtime() ;
int loadargs( int argc, char **argv ,map<string, TunnelInfo*>*tunnellist,char *s_name,int * s_port,char * authtoken);
int GetProtocol(char *url,char *Protocol);
int strpos( char *str, char c );
int getlocaladdr( map<string,TunnelInfo *> *tunnellist,char *url, struct sockaddr_in* local_addr );
int getvalue(char * str,const char *key,char * value);
int pack(unsigned char * buffer,string msgstr);
#if OPENSSL
int SendAuth(SSL* ssl,string ClientId,string user);
int SendRegProxy(SSL* ssl,string ClientId);
int SendPing(SSL* ssl);
int SendPong(SSL* ssl);
int SendReqTunnel(SSL* ssl,string protocol,string HostName,string Subdomain,int RemotePort);
int readlen(SSL* ssl,unsigned char *buffer, int readlen,int bufferlen);
#else
int SendAuth(ssl_context *ssl,string ClientId,string user);
int SendRegProxy(ssl_context *ssl,string ClientId);
int SendPing(ssl_context *ssl);
int SendPong(ssl_context *ssl);
int SendReqTunnel(ssl_context *ssl,string protocol,string HostName,string Subdomain,int RemotePort);
int readlen(ssl_context *ssl,unsigned char *buffer, int readlen,int bufferlen);
#endif

int IsLittleEndian();
__int64 ntoh64(__int64 val );
__int64 hton64(__int64 val );
#endif
