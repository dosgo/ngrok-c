#include "config.h"
#include <string>

#if WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#include <errno.h>
typedef long long __int64;
#endif
#include "bytestool.h"
#include "ngrok.h"

#include<stdlib.h>
using namespace std;




int ReqProxy(pthread_mutex_t mutex,struct sockaddr_in server_addr,map<int,sockinfo*>*socklist){
    int proxy_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
    setnonblocking( proxy_fd, 1 );
    connect( proxy_fd, (struct sockaddr *) &server_addr, sizeof(server_addr) );
    sockinfo * sinfo = (sockinfo *) malloc( sizeof(sockinfo) );
    sinfo->istype		= 1;
    sinfo->isconnect	= 0;
    sinfo->packbuflen	= 0;
    sinfo->sslinfo		= NULL;
    sinfo->linkunixtime	= get_curr_unixtime();
    sinfo->isconnectlocal	= 0;
    pthread_mutex_lock( &mutex );
    (*socklist).insert( map<int, sockinfo*> :: value_type( proxy_fd, sinfo ) );
    pthread_mutex_unlock( &mutex );
    return 0;
}
int NewTunnel(cJSON	*json,map<string,int>*tunneloklist){
    cJSON	*Payload	= cJSON_GetObjectItem(json, "Payload" );
    char	*error		= cJSON_GetObjectItem( Payload, "Error" )->valuestring;
    if(strcmp(error,"")==0)
    {
        char	*url		= cJSON_GetObjectItem( Payload, "Url" )->valuestring;
        char	*protocol	= cJSON_GetObjectItem( Payload, "Protocol" )->valuestring;
        (*tunneloklist)[string(protocol)]=1;
        printf("Add tunnel ok,type:%s url:%s\r\n",protocol,url);
    }
    else
    {
        printf("Add tunnel failed,%s\r\n",error);
        return -1;
    }
    return 0;
}

int RemoteSslInit(map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo,string ClientId,pthread_mutex_t mutex,map<int,sockinfo*>*socklist){
   ssl_info *sslinfo = (ssl_info *) malloc( sizeof(ssl_info) );
   tempinfo->sslinfo = sslinfo;
   #if OPENSSL
   // setnonblocking((*it1)->first,0);
    if ( openssl_init_info((*it1)->first, sslinfo ) != -1 )
    {
      //  setnonblocking((*it1)->first,1);
        //tempinfo->sslinfo = sslinfo;
        SendRegProxy(sslinfo->ssl, ClientId);
        pthread_mutex_lock( &mutex );
        (*it1)->second = tempinfo;
        pthread_mutex_unlock( &mutex );
    }
    else
    {
        setnonblocking((*it1)->first,1);
        printf( "getsockoptclose sock:%d\r\n", (*it1)->first );
        /* ssl 初始化失败，移除连接 */
        clearsock( (*it1)->first, tempinfo );
        pthread_mutex_lock( &mutex );
        (*socklist).erase((*it1)++);
        pthread_mutex_unlock( &mutex );
        return -1;
    }
    #else
    if (ssl_init_info((int *)&(*it1)->first, sslinfo ) != -1 )
    {
        SendRegProxy( &sslinfo->ssl, ClientId );
        pthread_mutex_lock( &mutex );
        (*it1)->second = tempinfo;
        pthread_mutex_unlock( &mutex );
    }
    else
    {
        printf( "getsockoptclose sock:%d\r\n", (*it1)->first );
        /* ssl 初始化失败，移除连接 */
        clearsock( (*it1)->first, tempinfo );
        pthread_mutex_lock( &mutex );
        (*socklist).erase((*it1)++);
        pthread_mutex_unlock( &mutex );
        return -1;
    }
    #endif
    return 0;
}

int LocalToRemote(map<int, sockinfo*>::iterator *it1,char *buf,int maxbuf,sockinfo *tempinfo1,ssl_info *sslinfo1,map<int,sockinfo*>*socklist,pthread_mutex_t mutex){
    int readlen;
    #if WIN32
    readlen = recv( (*it1)->first, (char *) buf, maxbuf, 0 );
    #else
    readlen = recv( (*it1)->first, buf, maxbuf, 0 );
    #endif
    if ( readlen > 0 )
    {
        setnonblocking( tempinfo1->tosock, 0 );
        #if OPENSSL
        SSL_write( sslinfo1->ssl, buf, readlen );
        #else
        #if ISMBEDTLS
        mbedtls_ssl_write( &sslinfo1->ssl, (unsigned char *)buf, readlen );
        #else
        ssl_write( &sslinfo1->ssl,(unsigned char *) buf, readlen );
        #endif // ISMBEDTLS
        #endif
        setnonblocking( tempinfo1->tosock, 1 );
    }else  {
        shutdown( tempinfo1->tosock, 2 );
        clearsock( (*it1)->first, tempinfo1 );
        pthread_mutex_lock( &mutex );
        (*socklist).erase((*it1)++);
        pthread_mutex_unlock( &mutex );
        return -1;
    }
    return 0;
}

int RemoteToLocal(ssl_info *sslinfo1,int maxbuf,char *buf,sockinfo *tempinfo1,map<int, sockinfo*>::iterator *it1,map<int,sockinfo*>*socklist,pthread_mutex_t mutex){
   int readlen;
   #if OPENSSL
   readlen = SSL_read( sslinfo1->ssl, (unsigned char *) buf, maxbuf );
   #else
     #if ISMBEDTLS
      readlen = mbedtls_ssl_read( &sslinfo1->ssl, (unsigned char *) buf, maxbuf );
     #else
     readlen = ssl_read( &sslinfo1->ssl, (unsigned char *) buf, maxbuf );
     #endif // ISMBEDTLS
   #endif

    if ( readlen < 1 )
    {
        /* close to sock */
        shutdown( tempinfo1->tosock, 2 );
        clearsock( (*it1)->first, tempinfo1 );
        pthread_mutex_lock( &mutex );
        (*socklist).erase((*it1)++);
        pthread_mutex_unlock( &mutex );
        return -1;
    }
    else
    {
        setnonblocking( tempinfo1->tosock, 0 );
        #if WIN32
        send( tempinfo1->tosock, (char *) buf, readlen, 0 );
        #else
        send( tempinfo1->tosock, buf, readlen, 0 );
        #endif
        setnonblocking( tempinfo1->tosock, 1 );
    }
    return 0;
}

int ConnectLocal(ssl_info *sslinfo1,char *buf,int maxbuf,map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo1,map<int,sockinfo*>*socklist,pthread_mutex_t mutex,char *tempjson,map<string,TunnelInfo*>*tunnellist,TunnelInfo	*tunnelinfo){
    int readlen;
    __int64		packlen;
    char Protocol[10]={0};
    #if OPENSSL
    readlen = SSL_read( sslinfo1->ssl, buf, maxbuf );
    #else
    #if ISMBEDTLS
    readlen = mbedtls_ssl_read( &sslinfo1->ssl,(unsigned char *) buf, maxbuf );
    #else
    readlen = ssl_read( &sslinfo1->ssl,(unsigned char *) buf, maxbuf );
    #endif  // ISMBEDTLS
    #endif // OPENSSL
    if ( readlen < 1 )
    {
        clearsock( (*it1)->first, tempinfo1 );
         pthread_mutex_lock( &mutex );
        (*socklist).erase((*it1)++);
         pthread_mutex_unlock( &mutex );
        return -1;
    }

    /* copy到临时缓存区 */
    if ( tempinfo1->packbuflen == 0 )
    {
        tempinfo1->packbuf = (char *) malloc( maxbuf );
    }
    memcpy( tempinfo1->packbuf + tempinfo1->packbuflen, buf, readlen );
    tempinfo1->packbuflen = tempinfo1->packbuflen + readlen;

    /*
     * 放到数组里面去
     * EnterCriticalSection(&g_cs);
     */
    pthread_mutex_lock( &mutex );
    (*it1)->second = tempinfo1;
    pthread_mutex_unlock( &mutex );

    if ( tempinfo1->packbuflen > 8 )
    {
        memcpy( &packlen, tempinfo1->packbuf, 8 );
        if ( BigEndianTest() == BigEndian )
        {
            packlen = Swap64( packlen );
        }
        if ( tempinfo1->packbuflen == packlen + 8 )
        {
            memset( tempjson, 0, 1025 );
            memcpy( tempjson, tempinfo1->packbuf + 8, packlen );
            free( tempinfo1->packbuf );
            tempinfo1->packbuf	= NULL;
            tempinfo1->packbuflen	= 0;
            cJSON	*json	= cJSON_Parse( tempjson );
            cJSON	*Type	= cJSON_GetObjectItem( json, "Type" );
            if ( strcmp( Type->valuestring, "StartProxy" ) == 0 )
            {
                cJSON	*Payload	= cJSON_GetObjectItem( json, "Payload" );
                char	*Url		= cJSON_GetObjectItem( Payload, "Url" )->valuestring;
                GetProtocol( Url, Protocol );
                 /*
                 * 清除
                 */
                cJSON_Delete( json );
                if((*tunnellist).count( string( Protocol ) ) > 0 )
                {
                    tunnelinfo = (*tunnellist)[string( Protocol )];
                    int tcp = socket( AF_INET, SOCK_STREAM, 0 );
                    setnonblocking( tcp, 1 );
                    connect( tcp, (struct sockaddr *) &tunnelinfo->local_addr, sizeof(tunnelinfo->local_addr));

                    sockinfo *sinfo = (sockinfo *) malloc( sizeof(sockinfo) );
                    sinfo->istype		= 2;
                    sinfo->isconnect	= 0;
                    sinfo->sslinfo		= sslinfo1;
                    sinfo->tosock		= (*it1)->first;
                    pthread_mutex_lock( &mutex );
                    (*socklist).insert( map<int, sockinfo*> :: value_type( tcp, sinfo ) );
                    pthread_mutex_unlock( &mutex );


                    /* 远程的带上本地链接 */
                    tempinfo1->tosock		= tcp;
                    tempinfo1->isconnectlocal	= 1;
                    pthread_mutex_lock( &mutex );
                    (*it1)->second = tempinfo1;
                    pthread_mutex_unlock( &mutex );

                }
            }
        }
    }
    return 0;
}


int CmdSock(int *mainsock,int maxbuf,char *buf,sockinfo *tempinfo,map<int,sockinfo*>*socklist,pthread_mutex_t mutex,char *tempjson,struct sockaddr_in server_addr,string *ClientId,map<string,int>*tunneloklist){
   //检测是否断开
   if(check_sock(*mainsock)!= 0)
   {
       printf("duan kai \r\n");
        return -1;
   }

   ssl_info *sslinfo=tempinfo->sslinfo;
    int readlen;
    __int64		packlen;
    #if OPENSSL
    readlen = SSL_read( sslinfo->ssl, buf, maxbuf );
    #else
    #if ISMBEDTLS
    readlen = mbedtls_ssl_read( &sslinfo->ssl,(unsigned char *) buf, maxbuf );
    #else
    readlen = ssl_read( &sslinfo->ssl,(unsigned char *) buf, maxbuf );
    #endif  // ISMBEDTLS
    #endif // OPENSSL
    if ( readlen < 1)
    {
        return -1;
    }

    /* copy到临时缓存区 */
    if ( tempinfo->packbuflen == 0 )
    {
        tempinfo->packbuf = (char *) malloc( maxbuf );
    }
    memcpy( tempinfo->packbuf + tempinfo->packbuflen, buf, readlen );
    tempinfo->packbuflen = tempinfo->packbuflen + readlen;
    if ( tempinfo->packbuflen > 8 )
    {
        memcpy( &packlen, tempinfo->packbuf, 8 );
        if ( BigEndianTest() == BigEndian )
        {
            packlen = Swap64( packlen );
        }
        if ( tempinfo->packbuflen == packlen + 8 )
        {
                memset( tempjson, 0, 1025 );
                memcpy( tempjson, tempinfo->packbuf + 8, packlen );
                free( tempinfo->packbuf );
                tempinfo->packbuf	= NULL;
                tempinfo->packbuflen	= 0;
                printf("%s\r\n",tempjson);
                cJSON *json = cJSON_Parse( tempjson );
				cJSON *Type = cJSON_GetObjectItem( json, "Type" );
				if ( strcmp( Type->valuestring, "ReqProxy" ) == 0 )
				{
					ReqProxy(mutex,server_addr,socklist);
				}
				if ( strcmp( Type->valuestring, "AuthResp" ) == 0 )
				{

					cJSON	*Payload	= cJSON_GetObjectItem( json, "Payload" );
					char	*cid		= cJSON_GetObjectItem( Payload, "ClientId" )->valuestring;
					char	*error		= cJSON_GetObjectItem( Payload, "Error" )->valuestring;
					if(strcmp(error,"")==0)
                    {
                        *ClientId = string( cid );
                        #if OPENSSL
                        SendPing( sslinfo->ssl );
                        #else
                        SendPing( &sslinfo->ssl );
                        #endif
                        tempinfo->isauth=1;
                    }
                    else
                    {
                        cJSON_Delete( json );
                        printf("Auth failed ,Please check authtoken. ");
                        return -1;
                    }

				}

				if ( strcmp( Type->valuestring, "Ping" ) == 0 )
				{

					#if OPENSSL
                    SendPong( sslinfo->ssl );
                    #else
                    SendPong( &sslinfo->ssl );
                    #endif
				}
				if ( strcmp( Type->valuestring, "Pong" ) == 0 )
				{
					tempinfo->pongtime = get_curr_unixtime();
				}
				if ( strcmp( Type->valuestring, "NewTunnel" ) == 0 )
				{
				    if(NewTunnel(json,tunneloklist)==-1)
                    {
                        return -1;
                    }
				}
				cJSON_Delete( json );
			}
		}
    return 0;
}

int ConnectMain(int maxbuf,int *mainsock,struct sockaddr_in server_addr,ssl_info **mainsslinfo,string *ClientId,pthread_mutex_t mutex,map<int,sockinfo*>*socklist,char *authtoken)
{
	*mainsock = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
	if(connect( *mainsock, (struct sockaddr *) &server_addr, sizeof(server_addr) ) != 0 )
	{
        printf("connect failed...!\r\n");
        #if WIN32
        closesocket(*mainsock);
        #else
        close(*mainsock);
        #endif
        return -1;
	}
    *mainsslinfo = (ssl_info *) malloc( sizeof(ssl_info) );
    #if OPENSSL
    if(openssl_init_info(*mainsock, *mainsslinfo ) == -1 )
	{
		printf( "ssl init failed!\r\n" );
        #if WIN32
        closesocket(*mainsock);
        #else
        close(*mainsock);
        #endif
		free(*mainsslinfo);
		return -1;
	}
	SendAuth((*mainsslinfo)->ssl, *ClientId, authtoken);
    #else
    if(ssl_init_info(mainsock, *mainsslinfo ) == -1 )
	{
		printf( "ssl init failed!\r\n" );
        #if WIN32
        closesocket(*mainsock);
        #else
        close(*mainsock);
        #endif
		free(*mainsslinfo);
		return -1;
	}
	SendAuth( &(*mainsslinfo)->ssl, *ClientId, authtoken );
    #endif
    setnonblocking( *mainsock,1);
    sockinfo * sinfo = (sockinfo *) malloc( sizeof(sockinfo) );
    sinfo->istype		= 3;
    sinfo->isconnect	= 1;
    sinfo->packbuflen	= 0;
    sinfo->sslinfo		= *mainsslinfo;
    pthread_mutex_lock( &mutex );
    (*socklist).insert( map<int, sockinfo*> :: value_type( *mainsock, sinfo ) );
    pthread_mutex_unlock( &mutex );
    return 0;
}
