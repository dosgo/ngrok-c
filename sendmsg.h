#ifndef __SENDMSG_H__
#define __SENDMSG_H__
#include "config.h"
#include "nonblocking.h"
#include <string>
#include <map>
#include "sslbio.h"

#if WIN32
#include <windows.h>
#else
#include <errno.h>
#include <netinet/in.h>
 //typedef unsigned long long __int64;
 #include <string.h>
 #ifndef __UCLIBC__
 #include <sys/select.h>
 #endif
#endif
#include "bytestool.h"
using namespace std;



inline int getUnixTime()
{
    time_t now;
    return time(&now);
}

int loadargs( int argc, char **argv);
char *rand_str(char *str,const int len);

inline int strpos( char *str, char c )
{
	char *sc = strchr( str, c );
	if ( sc == NULL )
		return(-1);
	return(sc - str);
}

inline void str_replace(char * cp, int n, char * str)
{
    int lenofstr;
    char * tmp;
    lenofstr = strlen(str);
    //str3比str2短，往前移动
    if(lenofstr < n)
    {
        tmp = cp+n;
        while(*tmp)
        {
            *(tmp-(n-lenofstr)) = *tmp; //n-lenofstr是移动的距离
            tmp++;
        }
        *(tmp-(n-lenofstr)) = *tmp; //move '\0'
    }
    else
            //str3比str2长，往后移动
        if(lenofstr > n)
        {
            tmp = cp;
            while(*tmp) tmp++;
            while(tmp>=cp+n)
            {
                *(tmp+(lenofstr-n)) = *tmp;
                tmp--;
            }
        }
    strncpy(cp,str,lenofstr);
}



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


inline int sendremote(int sock,ssl_context *ssl,const char *buf,int buflen,int isblock){
        int sendlen=0;
        if(isblock)
        {
            setnonblocking(sock,0);
        }
        #if OPENSSL
        #if OPENSSLDL
        sendlen=SslWrite(ssl, buf, buflen );
        #else
        sendlen=SSL_write(ssl, buf, buflen );
        #endif // OPENSSLDL
        #else
        #if ISMBEDTLS
        sendlen=mbedtls_ssl_write( ssl, (unsigned char *)buf, buflen );
        #else
        sendlen=ssl_write( ssl,(unsigned char *) buf, buflen );
        #endif // ISMBEDTLS
        #endif
        if(isblock)
        {
            setnonblocking(sock,1);
        }
        return sendlen;
}

inline int sendlocal(int sock,const char *buf,int buflen,int isblock){
    int sendlen=0;
    if(isblock)
    {
        setnonblocking(sock,0);
    }
    #if WIN32
    sendlen=send( sock, (char *) buf, buflen, 0 );
    #else
    sendlen=send( sock, buf, buflen, 0 );
    #endif
    if(isblock)
    {
        setnonblocking(sock,1);
    }
    return sendlen;
}


inline int sendpack(int sock,ssl_context *ssl,const char *msgstr,int isblock)
{
    unsigned char buffer[strlen(msgstr)+9];
    memset(buffer,0,strlen(msgstr)+9);
    #if WIN32
    unsigned __int64 packlen;
    #else
    unsigned long long packlen;
    #endif
    packlen=strlen(msgstr);
    packlen=LittleEndian_64(packlen);
    memcpy(buffer,&packlen,8);
    memcpy(buffer+8,msgstr,strlen(msgstr));
    if(isblock)
    {
        setnonblocking(sock,0);
    }
    #if OPENSSL
        #if OPENSSLDL
        int len=SslWrite( ssl, buffer, 8+strlen(msgstr));
        #else
        int len=SSL_write( ssl, buffer, 8+strlen(msgstr));
        #endif // OPENSSL
    #else
        #if ISMBEDTLS
            int len=mbedtls_ssl_write(ssl, buffer, 8+strlen(msgstr));
        #else
            int len=ssl_write(ssl, buffer, 8+strlen(msgstr));
        #endif // ISM
    #endif // ISM
    if(isblock)
    {
        setnonblocking(sock,1);
    }
    return  len;
}

inline int SendAuth(int sock,ssl_context *ssl)
{
   // string str="{\"Type\":\"Auth\",\"Payload\":{\"Version\":\"2\",\"MmVersion\":\"1.7\",\"User\":\""+user+"\",\"Password\": \"\",\"OS\":\"darwin\",\"Arch\":\"amd64\",\"ClientId\":\""+ClientId+"\"}}";
    char str[1024];
    memset(str,0,1024);
    sprintf(str,"{\"Type\":\"Auth\",\"Payload\":{\"Version\":\"2\",\"MmVersion\":\"1.7\",\"User\":\"%s\",\"Password\": \"%s\",\"OS\":\"darwin\",\"Arch\":\"amd64\",\"ClientId\":\"%s\"}}",mainInfo.authtoken,mainInfo.pwdc,mainInfo.ClientId.c_str());

    return sendpack(sock,ssl,str,1);
}


inline int SendRegProxy(int sock,ssl_context *ssl)
{
    char str[255];
    memset(str,0,255);
    sprintf(str,"{\"Type\":\"RegProxy\",\"Payload\":{\"ClientId\":\"%s\"}}",mainInfo.ClientId.c_str());
    return sendpack(sock,ssl,str,1);
}


inline int SendPing(int sock,ssl_context *ssl)
{
    return sendpack(sock,ssl,"{\"Type\":\"Ping\",\"Payload\":{}}",1);

}
inline int SendPong(int sock,ssl_context *ssl)
{
    return sendpack(sock,ssl,"{\"Type\":\"Pong\",\"Payload\":{}}",1);
}


int SendReqTunnel(int sock,ssl_context *ssl,TunnelInfo *tunnelInfo);
//#endif



#endif
