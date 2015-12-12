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
#include <errno.h>
#include <netinet/in.h>
 typedef long long __int64;
 #include <string.h>
 #ifndef __UCLIBC__
 #include <sys/select.h>
 #endif
#endif
#include "bytestool.h"
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


#if WIN32

#else
inline void milliseconds_sleep( unsigned long mSec )
{
    struct timeval tv;
    tv.tv_sec   = mSec / 1000;
    tv.tv_usec  = (mSec % 1000) * 1000;
    int err;
    do
    {
        err = select( 0, NULL, NULL, NULL, &tv );
    }
    while ( err < 0 && errno == EINTR );
}
#endif

inline void sleeps(int ti)
{
    #if WIN32
        Sleep( ti);
    #else
        milliseconds_sleep( ti);
    #endif
}

inline int get_curr_unixtime()
{
    time_t now;
    return time(&now);
}

int loadargs( int argc, char **argv ,map<string, TunnelInfo*>*tunnellist,char *s_name,int * s_port,char * authtoken);

inline int strpos( char *str, char c )
{
	char *sc = strchr( str, c );
	if ( sc == NULL )
		return(-1);
	return(sc - str);
}


inline int GetProtocol(char *url,char *Protocol)
{
	int	plen= strpos( url, ':' );
	if(plen>0)
    {
        memcpy( Protocol, url, plen );
        return 0;
    }
	return -1;
}


int getlocaladdr( map<string,TunnelInfo *> *tunnellist,char *url, struct sockaddr_in* local_addr );
inline int getvalue(char * str,const char *key,char * value){
    int ypos=0;
    if ( strncmp(str,key,strlen(key)) == 0 )
    {
        ypos = strpos( str, ':' );
        if ( ypos != -1 )
        {
            memcpy(value, str + ypos + 1, strlen( str + ypos ));
            return 0;
        }
    }
    return -1;
}
inline int pack(unsigned char * buffer,const string & msgstr)
{
    #if WIN32
    unsigned __int64 packlen;
    #else
    unsigned long long packlen;
    #endif
    packlen=msgstr.length();
    packlen=LittleEndian_64(packlen);
    memcpy(buffer,&packlen,8);
    memcpy(buffer+8,msgstr.c_str(), msgstr.length());
    return  8+msgstr.length();
}
#if OPENSSL
int SendAuth(SSL* ssl,string ClientId,string user);
int SendRegProxy(SSL* ssl,string &ClientId);
inline int SendPing(SSL *ssl)
{
   	string str="{\"Type\":\"Ping\",\"Payload\":{}}";
   	unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
    int len=SSL_write(ssl,buffer,sendlen);
    return len;
}
inline int SendPong(SSL *ssl)
{
   	string str="{\"Type\":\"Pong\",\"Payload\":{}}";
   	unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
    int len=SSL_write( ssl, buffer, sendlen);
    return len;
}

int SendReqTunnel(SSL* ssl,string protocol,string HostName,string Subdomain,int RemotePort);
#else
int SendAuth(ssl_context *ssl,string ClientId,string user);
int SendRegProxy(ssl_context *ssl,string &ClientId);
inline int SendPing(ssl_context *ssl)
{
   	string str="{\"Type\":\"Ping\",\"Payload\":{}}";
   	unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
    #if ISMBEDTLS
    int len=mbedtls_ssl_write(ssl, buffer, sendlen);
    #else
    int len=ssl_write(ssl, buffer, sendlen);
    #endif // ISMBEDTLS
    return len;
}
inline int SendPong(ssl_context *ssl)
{
   	string str="{\"Type\":\"Pong\",\"Payload\":{}}";
   	unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
     #if ISMBEDTLS
    int len=mbedtls_ssl_write(ssl, buffer, sendlen);
    #else
    int len=ssl_write(ssl, buffer, sendlen);
    #endif // ISMBEDTLS
    return len;
}


int SendReqTunnel(ssl_context *ssl,string protocol,string HostName,string Subdomain,int RemotePort);
#endif


__int64 ntoh64(__int64 val );
__int64 hton64(__int64 val );
#endif
