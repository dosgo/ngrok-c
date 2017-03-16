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

/*控制udp*/
int ControlUdp(int port){
    int sockfd;
    struct sockaddr_in my_addr;
    if((sockfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
    {
        return -2;
    }
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(port);
    my_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    memset(my_addr.sin_zero,0,8);
    int re_flag=1;
    int re_len=sizeof(int);
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&re_flag,re_len);
    if(bind(sockfd,(const struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1)
    {
        return -3;
    }
    setnonblocking( sockfd, 1 );
    return sockfd;
}
/**/
int UdpCmd(int udpsocket){
    struct sockaddr udpaddr;
    #if WIN32
    int udplen=sizeof(udpaddr);
    #else
    socklen_t udplen=sizeof(udpaddr);
    #endif
	char buf[1024]={0};
    int udpbuflen=recvfrom(udpsocket,buf,1024,MSG_PEEK ,&udpaddr,&udplen);
    if(udpbuflen>0){
         cJSON *json = cJSON_Parse( buf );
         cJSON *cmd = cJSON_GetObjectItem( json, "cmd" );
         //退出
         if ( strcmp( cmd->valuestring, "exit" ) == 0 ){
            cJSON_Delete( json );
            exit(0);
         }
         if ( strcmp( cmd->valuestring, "ping" ) == 0 ){
            //回复
            char sendbuf[255]="{\"cmd\":\"pong\"}";
            sendto(udpsocket,sendbuf,strlen(sendbuf),0,(struct sockaddr *)&udpaddr,sizeof(udpaddr));
            cJSON_Delete( json );
         }
        cJSON_Delete( json );
    }
    return 0;
}



int ReqProxy(struct sockaddr_in server_addr,map<int,sockinfo*>*socklist){
    int proxy_fd = socket( AF_INET, SOCK_STREAM, 0 );
    int flag = 1;
    setsockopt( proxy_fd, IPPROTO_TCP, TCP_NODELAY,(char*)&flag, sizeof(flag) );
    SetBufSize(proxy_fd);
    setnonblocking( proxy_fd, 1 );
    SetKeepAlive(proxy_fd);
    connect( proxy_fd, (struct sockaddr *) &server_addr, sizeof(server_addr) );
    sockinfo * sinfo = (sockinfo *) malloc( sizeof(sockinfo) );
    memset(sinfo,0,sizeof(sockinfo));
    sinfo->istype		= 1;
    sinfo->isconnect	= 0;
    sinfo->packbuflen	= 0;
    sinfo->linktime=get_curr_unixtime();
    sinfo->sslinfo		= NULL;
    sinfo->isconnectlocal	= 0;
    (*socklist).insert( map<int, sockinfo*> :: value_type( proxy_fd, sinfo ) );
    return 0;
}

int InitTunnelList(list<TunnelInfo*>*tunnellist,map<string,TunnelReq*>*tunneladdr){
    list<TunnelInfo*>::iterator iter;
    for(iter = (*tunnellist).begin(); iter !=(*tunnellist).end(); iter++)
    {
        TunnelInfo *tunnelinfo =(TunnelInfo*)*iter;
        tunnelinfo->regtime=0;
        tunnelinfo->regstate=0;
    }

    //释放所有通道信息
    map<string, TunnelReq*>::iterator it3;
    for ( it3 = (*tunneladdr).begin(); it3 != (*tunneladdr).end(); )
    {
       free(it3->second);
       it3++;
    }
    (*tunneladdr).clear();
    return 0;
}



int GetLocalAddr(char *url,struct sockaddr_in *local_addr,map<string,TunnelReq*> *tunneladdr){
    if((*tunneladdr).count(string(url))!=0)
    {
            TunnelReq *tunnelreq =(*tunneladdr)[string(url)];
            memset(local_addr,0,sizeof(sockaddr_in));
            int		l1		= inet_addr( tunnelreq->localhost );
            local_addr->sin_family	= AF_INET;
            local_addr->sin_port	= htons(tunnelreq->localport );
            memcpy(&local_addr->sin_addr, &l1, 4 );
            return 0;
    }
    return -1;
}

int SetLocalAddrInfo(char *url,char *ReqId,int regstate,list<TunnelInfo*>*tunnellist,map<string,TunnelReq*> *tunneladdr){
    list<TunnelInfo*>::iterator iter;
    char protocol[32] = { 0 };
    char host[256] = { 0 };
    char portstr[8]={0};
    char subdomain[128]={0};
    int port =0;
    sscanf(url,"%[^:]://%[^:]:%[0-9]",protocol,host,portstr);
    port=atoi(portstr);
    sscanf(host,"%[^.].",subdomain);
      //进行迭代遍历
    for(iter = (*tunnellist).begin(); iter !=(*tunnellist).end(); iter++)
    {
        TunnelInfo	*tunnelinfo =(TunnelInfo*)*iter;
        if(strcasecmp(ReqId,tunnelinfo->ReqId)==0){

            //memcpy(tunnelinfo->hostname,host,strlen(host));
            if(strncmp(protocol,"tcp",3)==0){
                tunnelinfo->remoteport=port;
            }
            if(strncmp(protocol,"http",4)==0){
                memset(tunnelinfo->subdomain,0,255);
                memcpy(tunnelinfo->subdomain,subdomain,strlen(subdomain));
            }
            tunnelinfo->regstate=regstate;
            TunnelReq *tunnelreq = (TunnelReq *) malloc( sizeof(TunnelReq));
            memset(tunnelreq,0,sizeof(TunnelReq));
            memcpy(tunnelreq->localhost,tunnelinfo->localhost,strlen(tunnelinfo->localhost));
            tunnelreq->localport=tunnelinfo->localport;
            (*tunneladdr).insert( map<string,TunnelReq*> :: value_type( string(url), tunnelreq ) );
        }

    }
    return 0;
}

int NewTunnel(cJSON	*json,list<TunnelInfo*>*tunnellist,map<string,TunnelReq*> *tunneladdr){
    cJSON	*Payload	= cJSON_GetObjectItem(json, "Payload" );
    char	*error		= cJSON_GetObjectItem( Payload, "Error" )->valuestring;
    if(strcmp(error,"")==0)
    {
        char	*url		= cJSON_GetObjectItem( Payload, "Url" )->valuestring;
        char	*ReqId		= cJSON_GetObjectItem( Payload, "ReqId" )->valuestring;
        char	*protocol	= cJSON_GetObjectItem( Payload, "Protocol" )->valuestring;
        SetLocalAddrInfo(url,ReqId,1,tunnellist,tunneladdr);
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
        /* ssl 初始化失败，移除连接 */
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
        /* ssl 初始化失败，移除连接 */
        clearsock( (*it1)->first, tempinfo );
        (*socklist).erase((*it1)++);
        return -1;
    }
    #endif
    return 0;
}

int LocalToRemote(map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo,ssl_info *sslinfo,map<int,sockinfo*>*socklist){
    int readlen;
    int bufsize=1024*15;//15K  //oolarssl SSL_MAX_CONTENT_LEN 16384
    //oolarssl 最大发送长度不能超过16K。。还是改成15吧
    char buf[bufsize+1];
    memset(buf,0,bufsize+1);
    #if WIN32
    readlen = recv( (*it1)->first, (char *) buf, bufsize, 0 );
    #else
    readlen = recv( (*it1)->first, buf, bufsize, 0 );
    #endif
    if ( readlen > 0&&sslinfo!=NULL )
    {
        //发送到远程
        #if OPENSSL
        sendremote(tempinfo->tosock,sslinfo->ssl,buf,readlen,1);
        #else
         sendremote(tempinfo->tosock,&sslinfo->ssl,buf,readlen,1);
        #endif
    }else  {
        shutdown( tempinfo->tosock, 2 );
        clearsock( (*it1)->first, tempinfo);
        (*socklist).erase((*it1)++);
        return -1;
    }
    return 0;
}

int RemoteToLocal(ssl_info *sslinfo1,sockinfo *tempinfo1,map<int, sockinfo*>::iterator *it1,map<int,sockinfo*>*socklist){
   int readlen,sendlen;
   int bufsize=1024*15;//15K  //oolarssl SSL_MAX_CONTENT_LEN 16384
   //oolarssl 最大发送长度不能超过16K。。还是改成15吧
   char buf[bufsize+1];
   memset(buf,0,bufsize+1);
   #if OPENSSL
    readlen =  SslRecv(sslinfo1->ssl,buf,bufsize);
   #else
    readlen =  SslRecv( &sslinfo1->ssl, (unsigned char *) buf, bufsize );
   #endif



    if ( readlen ==0||readlen ==-2)
    {
        /* close to sock */
        int tosock=tempinfo1->tosock;
        shutdown( tosock, 2 );
        clearsock( (*it1)->first, tempinfo1 );
        (*socklist).erase((*it1)++);
        //这行绝对不能删除，用标记ssl已经销毁，删除会导致崩溃。
        if((*socklist).count(tosock)==1)
        {
            (*socklist)[tosock]->sslinfo=NULL;
        }
        return -1;
    }
    else if(readlen >0)
    {
        sendlen=sendlocal(tempinfo1->tosock,buf,readlen,1);
        if(sendlen<1)
        {
            shutdown((*it1)->first,2);
            shutdown(tempinfo1->tosock,2);
        }
    }
    return 0;
}

int ConnectLocal(ssl_info *sslinfo,map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo1,map<int,sockinfo*>*socklist,map<string,TunnelReq*> *tunneladdr){
    //避免指针为空崩溃
    if(sslinfo==NULL){
         clearsock( (*it1)->first, tempinfo1 );
        (*socklist).erase((*it1)++);
        return -1;
    }
    int readlen;
    #if WIN32
    unsigned __int64		packlen;
    #else
    unsigned long long packlen;
    #endif
    char	tempjson[MAXBUF + 1];

    char buf[MAXBUF +1]={0};
    #if OPENSSL
    readlen =  SslRecv(sslinfo->ssl,buf,MAXBUF-1);
    #else
    readlen =  SslRecv(&sslinfo->ssl,(unsigned char *)buf,MAXBUF-1);
    #endif // OPENSSL

    struct sockaddr_in local_addr={0};
    if ( readlen ==0||readlen ==-2)
    {
        clearsock( (*it1)->first, tempinfo1 );
        (*socklist).erase((*it1)++);
        return -1;
    }

    if ( readlen ==-1)
    {
        return 0;//这里不能用-1不然会导致，map迭代器不自增
    }
    //有时候readlen变成-76导致崩溃
    if ( readlen <1)
    {
        clearsock( (*it1)->first, tempinfo1 );
        (*socklist).erase((*it1)++);
        return -1;
    }

    /* copy到临时缓存区 */
    if ( tempinfo1->packbuflen == 0 )
    {
        tempinfo1->packbuf = (unsigned char *) malloc( MAXBUF );
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
            memset( tempjson, 0, MAXBUF+1 );
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
                 /*
                 * 清除
                 */
                int backinfo=GetLocalAddr(Url,&local_addr,tunneladdr);
                cJSON_Delete( json );
                if(backinfo==0)
                {
                    int tcp = socket( AF_INET, SOCK_STREAM, 0 );
                    int flag = 1;
                    setsockopt( tcp, IPPROTO_TCP, TCP_NODELAY,(char*)&flag, sizeof(flag) );
                    SetBufSize(tcp);
                   // SOL_SOCKET
                    setnonblocking( tcp, 1 );
                    connect( tcp, (struct sockaddr *) &local_addr, sizeof(local_addr));
                    sockinfo *sinfo = (sockinfo *) malloc( sizeof(sockinfo) );
                    sinfo->istype		= 2;
                    sinfo->isconnect	= 0;
                    sinfo->sslinfo		= sslinfo;
                    sinfo->linktime=get_curr_unixtime();
                    sinfo->tosock		= (*it1)->first;
                    (*socklist).insert( map<int, sockinfo*> :: value_type( tcp, sinfo ) );
                    /* 远程的带上本地链接 */
                    tempinfo1->tosock = tcp;
                    tempinfo1->isconnectlocal= 1;
                }
                else
                {
                    clearsock( (*it1)->first, tempinfo1 );
                    (*socklist).erase((*it1)++);
                    return -1;
                }
            }
        }
    }
    return 0;
}


int CmdSock(int *mainsock,sockinfo *tempinfo,map<int,sockinfo*>*socklist,struct sockaddr_in server_addr,string *ClientId,char * authtoken,list<TunnelInfo*>*tunnellist,map<string,TunnelReq*> *tunneladdr){
   //检测是否断开
   if(check_sock(*mainsock)!= 0)
   {
        return -1;
   }

    ssl_info *sslinfo=tempinfo->sslinfo;
    int readlen;
    char buf[MAXBUF+1]={0};
    #if WIN32
    unsigned __int64 packlen;
    #else
    unsigned long long packlen;
    #endif
    char	tempjson[MAXBUF + 1];
    #if OPENSSL
    readlen =  SslRecv(sslinfo->ssl,buf,MAXBUF);
    #else
    readlen =  SslRecv(&sslinfo->ssl,(unsigned char *)buf,MAXBUF);
    #endif // OPENSSL

    if ( readlen ==0||readlen ==-2)
    {
        return -1;
    }

    if ( readlen ==-1||readlen<1)
    {
        return 0;
    }



    /* copy到临时缓存区 */
    if ( tempinfo->packbuflen == 0 )
    {
        tempinfo->packbuf = (unsigned char *) malloc( MAXBUF );
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
                memset( tempjson, 0, MAXBUF+1 );
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
				else if ( strcmp( Type->valuestring, "AuthResp" ) == 0 )
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
                    }
                    else
                    {
                        cJSON_Delete( json );
                        echo("Auth failed ,Please check authtoken. ");
                        return -1;
                    }

				}

				else if ( strcmp( Type->valuestring, "Ping" ) == 0 )
				{
					#if OPENSSL
                    SendPong( *mainsock,sslinfo->ssl );
                    #else
                    SendPong( *mainsock,&sslinfo->ssl );
                    #endif
				}
				else if ( strcmp( Type->valuestring, "Pong" ) == 0 )
				{

				}
				else if ( strcmp( Type->valuestring, "NewTunnel" ) == 0 )
				{
				    NewTunnel(json,tunnellist,tunneladdr);
				}
				cJSON_Delete( json );
			}

		}
    return 0;
}

int ConnectMain(int *mainsock,struct sockaddr_in server_addr,ssl_info **mainsslinfo,string *ClientId,map<int,sockinfo*>*socklist,char *authtoken,char *password_c)
{
	*mainsock = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
    SetBufSize(*mainsock);
	if(connect(*mainsock, (struct sockaddr *) &server_addr, sizeof(server_addr) ) != 0 )
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
        #if OPENSSL
        openssl_free_info(*mainsslinfo);
        #else
        ssl_free_info(*mainsslinfo);
        #endif
		free(*mainsslinfo);
		return -1;
	}

    #if OPENSSL
	SendAuth(*mainsock,(*mainsslinfo)->ssl, *ClientId, authtoken,password_c);
    #else
	SendAuth(*mainsock, &(*mainsslinfo)->ssl, *ClientId, authtoken,password_c);
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
