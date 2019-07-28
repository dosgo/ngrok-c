#include "config.h"
#include <string>

#if WIN32
#include <windows.h>
#include <string>
#else
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
typedef long long __int64;
#endif
#include "bytestool.h"
#include "ngrok.h"
#include "sslbio.h"
#include <stdlib.h>

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



int ReqProxy(struct sockaddr_in server_addr){
    int proxy_fd = socket( AF_INET, SOCK_STREAM, 0 );
    int flag = 1;
    setsockopt( proxy_fd, IPPROTO_TCP, TCP_NODELAY,(char*)&flag, sizeof(flag) );
    SetBufSize(proxy_fd);
    setnonblocking( proxy_fd, 1 );
    SetKeepAlive(proxy_fd);
    connect( proxy_fd, (struct sockaddr *) &server_addr, sizeof(server_addr) );
    Sockinfo * sinfo = (Sockinfo *) malloc( sizeof(Sockinfo) );
    memset(sinfo,0,sizeof(Sockinfo));
    sinfo->istype		= 1;
    sinfo->isconnect	= 0;
    sinfo->packbuflen	= 0;
    sinfo->linktime=getUnixTime();
    sinfo->sslinfo		= NULL;
    sinfo->isconnectlocal	= 0;
    sinfo->sock=proxy_fd;
    G_SockList.insert( map<int, Sockinfo*> :: value_type( proxy_fd, sinfo ) );
    return 0;
}

int InitTunnelList(){
    list<TunnelInfo*>::iterator iter;
    for(iter = G_TunnelList.begin(); iter !=G_TunnelList.end(); iter++)
    {

        TunnelInfo *tunnelinfo =(TunnelInfo*)*iter;
        if(strcasecmp(tunnelinfo->protocol,"udp")!=0){
            tunnelinfo->regtime=0;
            tunnelinfo->regstate=0;
        }

    }

    //释放所有通道信息
    map<string, TunnelReq*>::iterator it3;
    for ( it3 = mainInfo.G_TunnelAddr.begin(); it3 != mainInfo.G_TunnelAddr.end(); )
    {
       free(it3->second);
       it3++;
    }
    mainInfo.G_TunnelAddr.clear();
    return 0;
}





int SetLocalAddrInfo(char *url,char *ReqId,int regstate){
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
    for(iter = G_TunnelList.begin(); iter !=G_TunnelList.end(); iter++)
    {
        TunnelInfo	*tunnelinfo =(TunnelInfo*)*iter;
        if(strcasecmp(ReqId,tunnelinfo->ReqId)==0){

            //memcpy(tunnelinfo->hostname,host,strlen(host));
            if(strncasecmp(protocol,"tcp",3)==0){
                tunnelinfo->remoteport=port;
            }
            #if UDPTUNNEL
            if(strncasecmp(protocol,"udp",3)==0){
                tunnelinfo->remoteport=port;
            }
            #endif // UDPTUNNEL
            if(strncasecmp(protocol,"http",4)==0){
                memset(tunnelinfo->subdomain,0,255);
                memcpy(tunnelinfo->subdomain,subdomain,strlen(subdomain));
            }
            tunnelinfo->regstate=regstate;
            TunnelReq *tunnelreq = (TunnelReq *) malloc( sizeof(TunnelReq));
            memset(tunnelreq,0,sizeof(TunnelReq));
            memcpy(tunnelreq->url,url,strlen(url));
            memcpy(tunnelreq->localhost,tunnelinfo->localhost,strlen(tunnelinfo->localhost));
            tunnelreq->localport=tunnelinfo->localport;
            memcpy(tunnelreq->hostheader,tunnelinfo->hostheader,strlen(tunnelinfo->hostheader));

            #if UDPTUNNEL
            if(strncasecmp(protocol,"udp",3)==0){
                udpInfo.G_TunnelAddr.insert( map<string,TunnelReq*> :: value_type( string(url), tunnelreq ) );
            }
            #endif // UDPTUNNEL
            if(strncasecmp(protocol,"udp",3)!=0)
            {
                mainInfo.G_TunnelAddr.insert( map<string,TunnelReq*> :: value_type( string(url), tunnelreq ) );
            }

        }

    }
    return 0;
}

int NewTunnel(cJSON	*json){
    cJSON	*Payload	= cJSON_GetObjectItem(json, "Payload" );
    char	*error		= cJSON_GetObjectItem( Payload, "Error" )->valuestring;
    if(strcmp(error,"")==0)
    {
        char	*url		= cJSON_GetObjectItem( Payload, "Url" )->valuestring;
        char	*ReqId		= cJSON_GetObjectItem( Payload, "ReqId" )->valuestring;
        char	*protocol	= cJSON_GetObjectItem( Payload, "Protocol" )->valuestring;
        SetLocalAddrInfo(url,ReqId,1);
        echo("Add tunnel ok,type:%s url:%s\r\n",protocol,url);
    }
    else
    {
        echo("Add tunnel failed,%s\r\n",error);
        return -1;
    }
    return 0;
}

int RemoteSslInit(Sockinfo *tempinfo){
   ssl_info *sslinfo = (ssl_info *) malloc( sizeof(ssl_info) );
   tempinfo->sslinfo = sslinfo;


    if ( ssl_init_info(&tempinfo->sock, sslinfo ) != -1 )
    {
        #if OPENSSL
        setnonblocking(tempinfo->sock,1);
        SendRegProxy(tempinfo->sock,sslinfo->ssl);
        #else
        SendRegProxy(tempinfo->sock,&sslinfo->ssl);
        #endif
    }


    else
    {
        #if OPENSSL
        setnonblocking(tempinfo->sock,1);
        #endif
        /* ssl 初始化失败，移除连接 */
        clearsock(tempinfo );
        return -1;
    }
    return 0;
}

int LocalToRemote(Sockinfo *tempinfo,ssl_info *sslinfo){
    int readlen;
    int bufsize=1024*15;//15K  //oolarssl SSL_MAX_CONTENT_LEN 16384
    //oolarssl 最大发送长度不能超过16K。。还是改成15吧
    char buf[bufsize+1];
    memset(buf,0,bufsize+1);
    #if WIN32
    readlen = recv( tempinfo->sock, (char *) buf, bufsize, 0 );
    #else
    readlen = recv(  tempinfo->sock, buf, bufsize, 0 );
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
        clearsock(tempinfo);
        return -1;
    }
    return 0;
}

int RemoteToLocal(ssl_info *sslinfo,Sockinfo *tempinfo){
   int readlen,sendlen;
   int bufsize=1024*15;//15K  //oolarssl SSL_MAX_CONTENT_LEN 16384
   //oolarssl 最大发送长度不能超过16K。。还是改成15吧
   char buf[bufsize+1];
   memset(buf,0,bufsize+1);
    #if OPENSSL
    readlen =  SslRecv(sslinfo->ssl,(unsigned char *)buf,bufsize);
    #else
    readlen =  SslRecv(&sslinfo->ssl,(unsigned char *)buf,bufsize);
    #endif




    if ( readlen ==0||readlen ==-2)
    {
        /* close to sock */
        int tosock=tempinfo->tosock;
        shutdown( tosock, 2 );
        clearsock(tempinfo );
        //这行绝对不能删除，用标记ssl已经销毁，删除会导致崩溃。
        if(G_SockList.count(tosock)==1)
        {
            G_SockList[tosock]->sslinfo=NULL;
        }
        return -1;
    }
    else if(readlen >0)
    {

        TunnelReq *tunnelreq =  tempinfo->tunnelreq;
        char protocol[32] = { 0 };
        char remotehost[256] = { 0 };
        char httpline[3]="\r\n";
        sscanf(tunnelreq->url,"%[^:]://%[^\n]",protocol,remotehost);
        //不是tcp才需要转发
        if(strncmp(protocol,"tcp",3)!=0){
            //需要host头转发
            if(strlen(tunnelreq->hostheader)>0){
                //拼接\r\n用于识别http头rfc协议描述的
                memcpy(remotehost+strlen(remotehost),httpline,2);
                char *p=strstr(buf,remotehost);

                char srchost[256] = { 0 };
                memcpy(srchost,tunnelreq->hostheader,strlen(tunnelreq->hostheader));
                memcpy(srchost+strlen(srchost),httpline,2);
                //查到了。。
                if(p!=NULL){
                    //替换http请求头
                    str_replace(p,strlen(remotehost),srchost);
                }
            }
        }


        sendlen=sendlocal(tempinfo->tosock,buf,readlen,1);
        if(sendlen<1)
        {
            shutdown(tempinfo->sock,2);
            shutdown(tempinfo->tosock,2);
        }
    }
    return 0;
}

int ConnectLocal(ssl_info *sslinfo,Sockinfo *tempinfo){
    //避免指针为空崩溃
    if(sslinfo==NULL){
         clearsock(tempinfo);
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
    readlen =  SslRecv(sslinfo->ssl,(unsigned char *)buf,MAXBUF-1);
    #else
    readlen =  SslRecv(&sslinfo->ssl,(unsigned char *)buf,MAXBUF-1);
    #endif



    if ( readlen ==0||readlen ==-2)
    {
        clearsock(tempinfo);
        return -1;
    }

    if ( readlen ==-1)
    {
        return 0;//这里不能用-1不然会导致，map迭代器不自增
    }
    //有时候readlen变成-76导致崩溃
    if ( readlen <1)
    {
        clearsock(tempinfo);
        return -1;
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
            echo("tcp:%s\r\n",tempjson);
            tempinfo->packbuf	= NULL;
            tempinfo->packbuflen	= 0;
            cJSON	*json	= cJSON_Parse( tempjson );
            cJSON	*Type	= cJSON_GetObjectItem( json, "Type" );
            if ( strcmp( Type->valuestring, "StartProxy" ) == 0 )
            {
                cJSON	*Payload	= cJSON_GetObjectItem( json, "Payload" );
                char	*Url		= cJSON_GetObjectItem( Payload, "Url" )->valuestring;

                if(mainInfo.G_TunnelAddr.count(string(Url))!=0)
                {
                        TunnelReq *tunnelreq =mainInfo.G_TunnelAddr[string(Url)];
                        struct sockaddr_in local_addr={0};
                        local_addr.sin_family	= AF_INET;
                        local_addr.sin_port	= htons(tunnelreq->localport );
                        local_addr.sin_addr.s_addr=inet_addr( tunnelreq->localhost );

                        int tcp = socket( AF_INET, SOCK_STREAM, 0 );
                        int flag = 1;
                        setsockopt( tcp, IPPROTO_TCP, TCP_NODELAY,(char*)&flag, sizeof(flag) );
                        SetBufSize(tcp);
                       // SOL_SOCKET
                        setnonblocking( tcp, 1 );
                        connect( tcp, (struct sockaddr *) &local_addr, sizeof(local_addr));
                        Sockinfo *sinfo = (Sockinfo *) malloc( sizeof(Sockinfo) );
                        sinfo->istype		= 2;
                        sinfo->isconnect	= 0;
                        sinfo->sslinfo		= sslinfo;
                        sinfo->linktime=getUnixTime();
                        sinfo->tosock		= tempinfo->sock;
                        sinfo->sock         = tcp;
                        G_SockList.insert( map<int, Sockinfo*> :: value_type( tcp, sinfo ) );
                        /* 远程的带上本地链接 */
                        tempinfo->tosock = tcp;
                        tempinfo->tunnelreq    = tunnelreq;
                        tempinfo->isconnectlocal= 1;
                        cJSON_Delete( json );
                }
                else
                {

                    clearsock(tempinfo);
                    cJSON_Delete( json );
                    return -1;
                }
            }
        }
    }
    return 0;
}


int CmdSock(int *mainsock,Sockinfo *tempinfo,struct sockaddr_in server_addr){
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
    readlen =  SslRecv(sslinfo->ssl,(unsigned char *)buf,MAXBUF);
    #else
    readlen =  SslRecv(&sslinfo->ssl,(unsigned char *)buf,MAXBUF);
    #endif



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
                echo("tcp:%s\r\n",tempjson);
                cJSON *json = cJSON_Parse( tempjson );
				cJSON *Type = cJSON_GetObjectItem( json, "Type" );
				if ( strcmp( Type->valuestring, "ReqProxy" ) == 0 )
				{
					ReqProxy(server_addr);
				}
				else if ( strcmp( Type->valuestring, "AuthResp" ) == 0 )
				{

                    cJSON	*Payload	= cJSON_GetObjectItem( json, "Payload" );
					char	*error		= cJSON_GetObjectItem( Payload, "Error" )->valuestring;
					if(strcmp(error,"")==0)
                    {

                        char	*cid		= cJSON_GetObjectItem( Payload, "ClientId" )->valuestring;
                        mainInfo.ClientId = string( cid );
                        #if OPENSSL
                         SendPing( *mainsock,sslinfo->ssl );
                        #else
                        SendPing( *mainsock,&sslinfo->ssl );
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
                    SendPing( *mainsock,&sslinfo->ssl );
                    #endif

				}
				else if ( strcmp( Type->valuestring, "Pong" ) == 0 )
				{

				}
				else if ( strcmp( Type->valuestring, "NewTunnel" ) == 0 )
				{
				    NewTunnel(json);
				}
				cJSON_Delete( json );
			}

		}
    return 0;
}

int ConnectMain(int *mainsock,struct sockaddr_in server_addr,ssl_info **mainsslinfo)
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


    if(ssl_init_info(mainsock, *mainsslinfo ) == -1 )
	{
		echo( "ssl init failed!\r\n" );
        #if WIN32
        closesocket(*mainsock);
        #else
        close(*mainsock);
        #endif
        ssl_free_info(*mainsslinfo);
		free(*mainsslinfo);
		return -1;
	}

    #if OPENSSL
	SendAuth(*mainsock,(*mainsslinfo)->ssl);
	#else
	SendAuth(*mainsock,&(*mainsslinfo)->ssl);
	#endif


    setnonblocking( *mainsock,1);
    Sockinfo * sinfo = (Sockinfo *) malloc( sizeof(Sockinfo) );
    sinfo->istype		= 3;
    sinfo->isconnect	= 1;
    sinfo->packbuflen	= 0;
    sinfo->sock=*mainsock;
    sinfo->sslinfo		= *mainsslinfo;
    G_SockList.insert( map<int, Sockinfo*> :: value_type( *mainsock, sinfo ) );
    return 0;
}
