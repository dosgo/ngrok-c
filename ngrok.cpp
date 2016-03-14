#include "config.h"
#include <string>

#if WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
typedef long long __int64;
#endif
#include "bytestool.h"
#include "ngrok.h"
#include "sslbio.h"
#include<stdlib.h>
using namespace std;




int ReqProxy(struct sockaddr_in server_addr,map<int,sockinfo*>*socklist){
    int proxy_fd = socket( AF_INET, SOCK_STREAM, 0 );
    int flag = 1;
    setsockopt( proxy_fd, IPPROTO_TCP, TCP_NODELAY,(char*)&flag, sizeof(flag) );
    setnonblocking( proxy_fd, 1 );
    SetKeepAlive(proxy_fd);
    connect( proxy_fd, (struct sockaddr *) &server_addr, sizeof(server_addr) );
    sockinfo * sinfo = (sockinfo *) malloc( sizeof(sockinfo) );
    sinfo->istype		= 1;
    sinfo->isconnect	= 0;
    sinfo->packbuflen	= 0;
    sinfo->sslinfo		= NULL;
    sinfo->linkunixtime	= get_curr_unixtime();
    sinfo->isconnectlocal	= 0;
    (*socklist).insert( map<int, sockinfo*> :: value_type( proxy_fd, sinfo ) );
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
        echo("Add tunnel ok,type:%s url:%s\r\n",protocol,url);
    }
    else
    {
        echo("Add tunnel failed,%s\r\n",error);
        return -1;
    }
    return 0;
}

int RemoteSslInit(map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo,string &ClientId,map<int,sockinfo*>*socklist){
   ssl_info *sslinfo = (ssl_info *) malloc( sizeof(ssl_info) );
   tempinfo->sslinfo = sslinfo;

   #if OPENSSL

    if ( openssl_init_info((*it1)->first, sslinfo ) != -1 )
    {
        setnonblocking((*it1)->first,1);
        SendRegProxy((*it1)->first,sslinfo->ssl, ClientId);
    }
    else
    {
        setnonblocking((*it1)->first,1);
        /* ssl ��ʼ��ʧ�ܣ��Ƴ����� */
        clearsock( (*it1)->first, tempinfo );
        (*socklist).erase((*it1)++);
        return -1;
    }
    #else
    if (ssl_init_info((int *)&(*it1)->first, sslinfo ) != -1 )
    {

        SendRegProxy((*it1)->first,&sslinfo->ssl, ClientId );
    }
    else
    {
        echo( "getsockoptclose sock:%d\r\n", (*it1)->first );
        /* ssl ��ʼ��ʧ�ܣ��Ƴ����� */
        clearsock( (*it1)->first, tempinfo );
        (*socklist).erase((*it1)++);
        return -1;
    }
    #endif
    return 0;
}

int LocalToRemote(map<int, sockinfo*>::iterator *it1,char *buf,int maxbuf,sockinfo *tempinfo,ssl_info *sslinfo,map<int,sockinfo*>*socklist){
    int readlen;
    #if WIN32
    readlen = recv( (*it1)->first, (char *) buf, maxbuf-1, 0 );
    #else
    readlen = recv( (*it1)->first, buf, maxbuf-1, 0 );
    #endif
    if ( readlen > 0&&sslinfo!=NULL )
    {
        setnonblocking( tempinfo->tosock, 0 );
        #if OPENSSL
        SSL_write( sslinfo->ssl, buf, readlen );
        #else
        #if ISMBEDTLS
        mbedtls_ssl_write( &sslinfo->ssl, (unsigned char *)buf, readlen );
        #else
        ssl_write( &sslinfo->ssl,(unsigned char *) buf, readlen );
        #endif // ISMBEDTLS
        #endif
        setnonblocking( tempinfo->tosock, 1 );
    }else  {
        shutdown( tempinfo->tosock, 2 );
        clearsock( (*it1)->first, tempinfo);
        (*socklist).erase((*it1)++);
        return -1;
    }
    return 0;
}

int RemoteToLocal(ssl_info *sslinfo1,int maxbuf,char *buf,sockinfo *tempinfo1,map<int, sockinfo*>::iterator *it1,map<int,sockinfo*>*socklist){
   int readlen,sendlen;
   #if OPENSSL
    readlen =  SslRecv(sslinfo1->ssl,buf,maxbuf);
   #else
    readlen =  SslRecv( &sslinfo1->ssl, (unsigned char *) buf, maxbuf );
   #endif



    if ( readlen ==0||readlen ==-2)
    {
        /* close to sock */
        int tosock=tempinfo1->tosock;
        shutdown( tosock, 2 );
        clearsock( (*it1)->first, tempinfo1 );
        (*socklist).erase((*it1)++);
        if((*socklist).count(tosock)==1)
        {
            (*socklist)[tosock]->sslinfo=NULL;
        }
        return -1;
    }
    else if(readlen >0)
    {
        setnonblocking( tempinfo1->tosock, 0 );
        #if WIN32
        sendlen=send( tempinfo1->tosock, (char *) buf, readlen, 0 );
        #else
        sendlen=send( tempinfo1->tosock, buf, readlen, 0 );
        #endif
        setnonblocking( tempinfo1->tosock, 1 );
        if(sendlen<1)
        {
            shutdown((*it1)->first,2);
            shutdown(tempinfo1->tosock,2);
        }
    }
    return 0;
}

int ConnectLocal(ssl_info *sslinfo,char *buf,int maxbuf,map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo1,map<int,sockinfo*>*socklist,char *tempjson,map<string,TunnelInfo*>*tunnellist,TunnelInfo	*tunnelinfo){
    int readlen;
    #if WIN32
    unsigned __int64		packlen;
    #else
    unsigned long long packlen;
    #endif
    char Protocol[10]={0};
    #if OPENSSL
    readlen =  SslRecv(sslinfo->ssl,buf,maxbuf-1);
    #else
    readlen =  SslRecv(&sslinfo->ssl,(unsigned char *)buf,maxbuf-1);
    #endif // OPENSSL

    if ( readlen ==0||readlen ==-2)
    {
        clearsock( (*it1)->first, tempinfo1 );
        (*socklist).erase((*it1)++);
        return -1;
    }

    if ( readlen ==-1)
    {
        return -1;
    }
    //��ʱ��readlen���-76���±���
    if ( readlen <1)
    {
        clearsock( (*it1)->first, tempinfo1 );
        (*socklist).erase((*it1)++);
        return -1;
    }

    /* copy����ʱ������ */
    if ( tempinfo1->packbuflen == 0 )
    {
        tempinfo1->packbuf = (unsigned char *) malloc( maxbuf );
    }



    memcpy( tempinfo1->packbuf + tempinfo1->packbuflen, buf, readlen );
    tempinfo1->packbuflen = tempinfo1->packbuflen + readlen;



    if ( tempinfo1->packbuflen > 8 )
    {
        memcpy( &packlen, tempinfo1->packbuf, 8 );
        if ( BigEndianTest() == BigEndian )
        {
            packlen = Swap64( packlen );
        }
        if ( tempinfo1->packbuflen == packlen + 8 )
        {
            memset( tempjson, 0, maxbuf+1 );
            memcpy( tempjson, tempinfo1->packbuf + 8, packlen );
            free( tempinfo1->packbuf );
            echo("%s\r\n",tempjson);
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
                 * ���
                 */
                cJSON_Delete( json );
                if((*tunnellist).count( string( Protocol ) ) > 0 )
                {
                    tunnelinfo = (*tunnellist)[string( Protocol )];
                    int tcp = socket( AF_INET, SOCK_STREAM, 0 );
                    int flag = 1;
                    setsockopt( tcp, IPPROTO_TCP, TCP_NODELAY,(char*)&flag, sizeof(flag) );
                   // SOL_SOCKET
                  //  setnonblocking( tcp, 1 );
                    if(connect( tcp, (struct sockaddr *) &tunnelinfo->local_addr, sizeof(tunnelinfo->local_addr))==0)
                    {
                        setnonblocking( tcp, 1 );
                        sockinfo *sinfo = (sockinfo *) malloc( sizeof(sockinfo) );
                        sinfo->istype		= 2;
                        sinfo->isconnect	= 1;
                        sinfo->sslinfo		= sslinfo;
                        sinfo->tosock		= (*it1)->first;
                        (*socklist).insert( map<int, sockinfo*> :: value_type( tcp, sinfo ) );
                        /* Զ�̵Ĵ��ϱ������� */
                        tempinfo1->tosock = tcp;
                        tempinfo1->isconnectlocal= 1;
                    }
                    else
                    {
                        #if WIN32
                        echo("GetLastError:%d\r\n",GetLastError());
                        #else
                        echo("errno:%d\r\n",errno);
                        #endif
                         clearsock( (*it1)->first, tempinfo1 );
                        (*socklist).erase((*it1)++);
                        return -1;
                    }

                }
            }
        }
    }
    return 0;
}


int CmdSock(int *mainsock,int maxbuf,char *buf,sockinfo *tempinfo,map<int,sockinfo*>*socklist,char *tempjson,struct sockaddr_in server_addr,string *ClientId,char * authtoken,map<string,int>*tunneloklist,map<string,TunnelInfo*>*tunnellist){
   //����Ƿ�Ͽ�
   if(check_sock(*mainsock)!= 0)
   {
        return -1;
   }

    ssl_info *sslinfo=tempinfo->sslinfo;
    TunnelInfo	*tunnelinfo;
    int readlen;

    #if WIN32
    unsigned __int64 packlen;
    #else
    unsigned long long packlen;
    #endif

    #if OPENSSL
    readlen =  SslRecv(sslinfo->ssl,buf,maxbuf);
    #else
    readlen =  SslRecv(&sslinfo->ssl,(unsigned char *)buf,maxbuf);
    #endif // OPENSSL

    if ( readlen ==0||readlen ==-2)
    {
        return -1;
    }

    if ( readlen ==-1||readlen<1)
    {
        return 0;
    }



    /* copy����ʱ������ */
    if ( tempinfo->packbuflen == 0 )
    {
        tempinfo->packbuf = (unsigned char *) malloc( maxbuf );
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
                memset( tempjson, 0, maxbuf+1 );
                memcpy( tempjson, tempinfo->packbuf + 8, packlen );
                free( tempinfo->packbuf );
                tempinfo->packbuf	= NULL;
                tempinfo->packbuflen	= 0;
                echo("%s\r\n",tempjson);
                cJSON *json = cJSON_Parse( tempjson );
				cJSON *Type = cJSON_GetObjectItem( json, "Type" );
				if ( strcmp( Type->valuestring, "ReqProxy" ) == 0 )
				{
					ReqProxy(server_addr,socklist);
				}
				if ( strcmp( Type->valuestring, "AuthResp" ) == 0 )
				{

                    cJSON	*Payload	= cJSON_GetObjectItem( json, "Payload" );
					char	*error		= cJSON_GetObjectItem( Payload, "Error" )->valuestring;
					if(strcmp(error,"")==0)
                    {

                        char	*cid		= cJSON_GetObjectItem( Payload, "ClientId" )->valuestring;
                        *ClientId = string( cid );
                        #if OPENSSL
                        SendPing( *mainsock,sslinfo->ssl );
                        #else
                        SendPing(*mainsock, &sslinfo->ssl );
                        #endif
                        tempinfo->isauth=1;

                        /*��֤�ɹ���ע��ͨ��*/
                        map<string, TunnelInfo*>::iterator it;
                        for ( it = (*tunnellist).begin(); it != (*tunnellist).end(); ++it )
                        {
                            tunnelinfo = it->second;
                            #if OPENSSL
                            SendReqTunnel(*mainsock,sslinfo->ssl, it->first.c_str(),tunnelinfo->hostname,tunnelinfo->subdomain, tunnelinfo->remoteport ,authtoken);
                            #else
                            SendReqTunnel(*mainsock,&sslinfo->ssl,it->first.c_str(),tunnelinfo->hostname,tunnelinfo->subdomain, tunnelinfo->remoteport,authtoken );
                            #endif
                        }
                    }
                    else
                    {
                        cJSON_Delete( json );
                        echo("Auth failed ,Please check authtoken. ");
                        return -1;
                    }

				}

				if ( strcmp( Type->valuestring, "Ping" ) == 0 )
				{
					#if OPENSSL
                    SendPong( *mainsock,sslinfo->ssl );
                    #else
                    SendPong( *mainsock,&sslinfo->ssl );
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

int ConnectMain(int maxbuf,int *mainsock,struct sockaddr_in server_addr,ssl_info **mainsslinfo,string *ClientId,map<int,sockinfo*>*socklist,char *authtoken)
{
	*mainsock = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
	if(connect( *mainsock, (struct sockaddr *) &server_addr, sizeof(server_addr) ) != 0 )
	{
        echo("connect failed...!\r\n");
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
    #else
    if(ssl_init_info(mainsock, *mainsslinfo ) == -1 )
    #endif

	{
		echo( "ssl init failed!\r\n" );
        #if WIN32
        closesocket(*mainsock);
        #else
        close(*mainsock);
        #endif
		free(*mainsslinfo);
		return -1;
	}

    #if OPENSSL
	SendAuth(*mainsock,(*mainsslinfo)->ssl, *ClientId, authtoken);
    #else
	SendAuth(*mainsock, &(*mainsslinfo)->ssl, *ClientId, authtoken );
    #endif

    setnonblocking( *mainsock,1);
    sockinfo * sinfo = (sockinfo *) malloc( sizeof(sockinfo) );
    sinfo->istype		= 3;
    sinfo->isconnect	= 1;
    sinfo->packbuflen	= 0;
    sinfo->sslinfo		= *mainsslinfo;
    (*socklist).insert( map<int, sockinfo*> :: value_type( *mainsock, sinfo ) );
    return 0;
}
