#include "config.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <iomanip>
#include <signal.h>


#if  WIN32
#include <windows.h>
#else
#if DEBUG
#include "segment.h"
#endif
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#endif

#include "cJSON.h"
#include "mytime.h"
#include "bytestool.h"
#include "ngrok.h"
#include "sslbio.h"
#include "nonblocking.h"

using namespace std;
//string VER = "1.35-(2016/5/13)";
char VER[24]= "1.46-(2018/05/8)";

char s_name[255]="ngrokd.ngrok.com";
int s_port= 443;
char authtoken[255]="";
char password_c[255]={0};//
string ClientId = "";
int pingtime	= 0;
int ping		= 25; //不能大于30
int mainsock=0;
int lastdnstime=0;
int mainsockstatus=1;
int regtunneltime=0;
int lastdnsback;
int lasterrtime=0;
ssl_info *mainsslinfo=NULL;

#if UDPCMD
int udpsocket=0;
int udpport=1885;
#endif

void* proxy( );
struct sockaddr_in server_addr = { 0 };

void cs( int n )
{
	if(n == SIGINT )
	{
		printf( "key down ctrl+c\r\n" );
		exit( 0 );
	}
}



int CheckStatus()
{

    //判断是否存在
    if (G_SockList.count(mainsock)==0||mainsock==0)
    {
        if(lasterrtime==0||(lasterrtime+60)<get_curr_unixtime()){
            //连接失败
            if(ConnectMain(&mainsock,server_addr,&mainsslinfo,&ClientId,authtoken,password_c)==-1)
            {
                mainsockstatus=0;
                printf("link err\r\n");
                lasterrtime=get_curr_unixtime();
                return -1;
            }
        }
    }
	return(0);
}
/*检测ping*/
int checkping(){
    if(pingtime+ping<get_curr_unixtime()&&G_SockList.count(mainsock)!=0&&mainsock!=0)
    {
        #if OPENSSL
        int sendlen = SendPing(mainsock, mainsslinfo->ssl );
        #else
        int sendlen = SendPing(mainsock, &mainsslinfo->ssl );
        #endif

        //发送失败断开连接
        if(sendlen<1)
        {
            mainsockstatus=0;
        }
        pingtime = get_curr_unixtime();
    }
    return 0;
}

int main( int argc, char **argv )
{
    printf("ngrokc v%s \r\n",VER);
	loadargs( argc, argv, s_name, &s_port, authtoken,password_c,&ClientId );
    #if WIN32
	signal( SIGINT, cs );
    #else
	/* socket close hui fa song xing hao dao zhi tui chu */
	signal( SIGPIPE, SIG_IGN );
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = cs;
	sigemptyset( &sigIntHandler.sa_mask );
	sigIntHandler.sa_flags = 0;
	sigaction( SIGINT, &sigIntHandler, NULL );
    #endif


    #if WIN32
	WSADATA wsaData = { 0 };
	if ( 0 != WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) )
	{
		printf( "WSAStartup failed. errno=[%d]\n", WSAGetLastError() );
		return(-1);
	}
    #endif

    #if OPENSSL
    ssl_lib_init();
    #else
    init_ssl_session();
    #endif // OPENSSL

	/* init addr */
	lastdnsback	= net_dns( &server_addr, s_name, s_port );
	lastdnstime	= get_curr_unixtime();
    #if UDPCMD
    udpsocket=ControlUdp(udpport);
    #endif // UDPCMD
    proxy();
	return(0);
}


void* proxy(  )
{

	fd_set	writeSet;
	fd_set	readSet;
	int		maxfd = 0;
	//unsigned char	buf[MAXBUF];
	struct timeval	timeout;

	int		maxfdp		= 0;
	int ret=0;
	ssl_info *sslinfo1;
	Sockinfo *tempinfo ;
	Sockinfo *tempinfo1;
	map<int, Sockinfo*>::iterator it;
	map<int, Sockinfo*>::iterator it1;
	map<int, Sockinfo*>::iterator it3;
    list<TunnelInfo*>::iterator listit;
    TunnelInfo *tunnelinfo;
    char ReqId[20]={0};
	int backcode=0;

	while ( true )
	{
        //ping
        checkping();//ping

        //控制链接断开,关闭所有连接
        if(mainsockstatus==0){
            shutdown( mainsock,2);
            //释放所有连接
            for ( it3 = G_SockList.begin(); it3 != G_SockList.end(); )
            {
                clearsock( it3->first,  it3->second);
                it3++;
            }
            G_SockList.clear();
            mainsock = 0;
            //改回状态
            mainsockstatus=1;
            //初始化通道
            InitTunnelList();
        }

	    if (lastdnsback == -1 ||(lastdnstime + 600) < get_curr_unixtime())
		{
			lastdnsback	= net_dns( &server_addr, s_name, s_port );
			lastdnstime	= get_curr_unixtime();
			printf( "update dns\r\n" );
		}
		//dns解析成功
        if (lastdnsback != -1)
        {
            CheckStatus();
        }

        //注册端口
        if(G_SockList.count(mainsock)!=0&&mainsock!=0){

            tempinfo=G_SockList[mainsock];
            if(tempinfo->isauth==1&&regtunneltime+60<get_curr_unixtime()){
                regtunneltime=get_curr_unixtime();
                for ( listit = G_TunnelList.begin(); listit != G_TunnelList.end(); ++listit )
                {
                    tunnelinfo =(TunnelInfo	*)*listit;
                    if(tunnelinfo->regstate==0){
                        memset(ReqId,0,20);
                        memset(tunnelinfo->ReqId,0,20);
                        #if OPENSSL
                        SendReqTunnel(mainsock,mainsslinfo->ssl,ReqId,tunnelinfo->protocol,tunnelinfo->hostname,tunnelinfo->subdomain, tunnelinfo->remoteport ,authtoken);
                        #else
                        SendReqTunnel(mainsock,&mainsslinfo->ssl,ReqId,tunnelinfo->protocol,tunnelinfo->hostname,tunnelinfo->subdomain, tunnelinfo->remoteport ,authtoken);
                        #endif
                        //copy
                        memcpy(tunnelinfo->ReqId,ReqId,strlen(ReqId));
                        tunnelinfo->regtime=get_curr_unixtime();//已发
                    }
                }
            }
        }


		timeout.tv_sec	= 0;
		timeout.tv_usec = 5000;
		FD_ZERO( &readSet );
		maxfd	= 0;
		maxfdp	= 0;
		FD_ZERO( &writeSet );

		/* 遍历添加 */
		//map<int, sockinfo*>::iterator it;
		for ( it = G_SockList.begin(); it != G_SockList.end();  )
		{

			tempinfo = it->second;
			/* 如果未连接才添加，写入监听 */
			if ( tempinfo->isconnect == 0 )
			{
				FD_SET( it->first, &writeSet );
			}
			else{
				FD_SET( it->first, &readSet );
			}
			maxfdp = it->first > maxfdp ? it->first : maxfdp;
			maxfd++;
			//继续遍历
			sleeps(1);
			++it;
		}

        #if UDPCMD
        if(udpsocket>0){
            maxfdp = udpsocket > maxfdp ? udpsocket : maxfdp;
            FD_SET(udpsocket,&readSet );
        }
        #endif //

		if(maxfd==0)
		{
			sleeps( 500 );
		}

		/*  printf("add ok \r\n"); */
        ret = select( maxfdp + 1, &readSet, &writeSet, NULL, &timeout ); /* 为等待时间传入NULL，则永久等待。传入0立即返回。不要勿用。 */
		if ( ret == -1 && maxfd != 0 )
		{
            echo("select error\r\n");
			continue;
		}

		if ( ret > 0 )
		{


            #if UDPCMD
            if(FD_ISSET(udpsocket,&readSet))
            {
                UdpCmd(udpsocket);
            }
            #endif

			for ( it1 = G_SockList.begin(); it1 != G_SockList.end(); )
			{
			    tempinfo = it1->second;
			      //ping
                checkping();//这里添加个ping避免超时
                //判断连接超时sock
                if((tempinfo->linktime+10)<get_curr_unixtime()&&tempinfo->isconnect==0)
                {
					//关闭远程连接
                    if(tempinfo->istype==2){
                        echo("connect local fail\r\n");
                        shutdown( tempinfo->tosock, 2 );
                    }
                    clearsock(it1->first,tempinfo);
                    G_SockList.erase(it1++);
                    continue;
                }

			    /*等于1才是添加到 readSet的*/
				if (FD_ISSET( it1->first, &readSet )&&tempinfo->isconnect==1 )
				{

                    sslinfo1 = tempinfo->sslinfo;
                    /* 远程的转发给本地 */
                    if ( tempinfo->istype == 1 )
                    {
                        //未连接本地
                        if ( tempinfo->isconnectlocal == 0 )
                        {
                            backcode=ConnectLocal(sslinfo1,&it1,tempinfo);
                            if(backcode==-1)
                            {
                              continue;
                            }
                        }
                        //等待本地连接完成
                        if( tempinfo->isconnectlocal == 1 )
                        {
                            sleeps( 1 );//避免频繁触发cpu过高
                        }
                        //本地连接完成转发
                        if( tempinfo->isconnectlocal == 2 )
                        {
                            backcode=RemoteToLocal(sslinfo1,tempinfo,&it1);
                            if(backcode==-1)
                            {
                              continue;
                            }
                        }
                    }
                    /* 本地的转发给远程 */
                    else if(tempinfo->istype == 2){
                        backcode=LocalToRemote(&it1,tempinfo,sslinfo1);
                        if(backcode==-1)
                        {
                          continue;
                        }
                    }
                    //控制连接
                    else if(tempinfo->istype ==3){
                         backcode=CmdSock(&mainsock,tempinfo,server_addr,&ClientId,authtoken);
                         if(backcode==-1)
                         {
                             //控制链接断开，标记清空
                             mainsockstatus=0;
                             lasterrtime=get_curr_unixtime();
                             break;
                         }
                    }


				}
				//可写，表示连接上了
				if ( FD_ISSET( it1->first, &writeSet )&&tempinfo->isconnect==0 )
				{
					if ( tempinfo->isconnect == 0 )
					{
					    //检测连接是否可用
						if (check_sock(it1->first)!= 0 )
						{
						    	//关闭远程连接
							if(tempinfo->istype==2){
                                echo("连接本地失败");
							    shutdown( tempinfo->tosock, 2 );
							}
							clearsock(it1->first,tempinfo);
							G_SockList.erase(it1++);
							continue;
						}


						/* 为远程连接 */
						if ( tempinfo->istype == 1 )
						{
						    //初始化远程连接
                            backcode=RemoteSslInit(&it1,tempinfo,ClientId);
                            if(backcode==-1)
                            {
                               continue;
                            }
                            /* 置为1 */
                            tempinfo->isconnect = 1;
						}

                        //本地连接
                        if ( tempinfo->istype == 2 )
						{
                            if(G_SockList.count(tempinfo->tosock)>0)
                            {
                                tempinfo1 = G_SockList[tempinfo->tosock];
                                tempinfo1->isconnectlocal=2;
                            }
                            /* 置为1 */
                            tempinfo->isconnect = 1;
						}

					}
				}
				//继续遍历
				++it1;
			}
		}
		//睡1豪秒，避免CPU过高
		sleeps(2);
	}
	return(0);
}
