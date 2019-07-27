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
#if UDPTUNNEL
#include "udp.h"
#endif
using namespace std;
//string VER = "1.35-(2016/5/13)";
char VER[24]= "1.53-(2019/07/28)";


ssl_info *mainsslinfo=NULL;

#if UDPCMD
int udpsocket=0;
int udpport=1885;
#endif

void* proxy( );


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
    if (G_SockList.count(mainInfo.mainsock)==0||mainInfo.mainsock==0)
    {
        if(mainInfo.lasterrtime==0||(mainInfo.lasterrtime+60)<getUnixTime()){
            //连接失败
            if(ConnectMain(&mainInfo.mainsock,mainInfo.saddr,&mainsslinfo)==-1)
            {
                mainInfo.mainsockstatus=0;
                printf("link err\r\n");
                mainInfo.lasterrtime=getUnixTime();
                return -1;
            }
        }
    }
	return(0);
}
/*检测ping*/
int checkping(){
    if(mainInfo.pingtime+mainInfo.ping<getUnixTime()&&G_SockList.count(mainInfo.mainsock)!=0&&mainInfo.mainsock!=0)
    {
        #if OPENSSL
        int sendlen = SendPing(mainInfo.mainsock, mainsslinfo->ssl );
        #else
        int sendlen = SendPing(mainInfo.mainsock, &mainsslinfo->ssl );
        #endif

        //发送失败断开连接
        if(sendlen<1)
        {
            mainInfo.mainsockstatus=0;
        }
        mainInfo.pingtime = getUnixTime();
    }
    return 0;
}

int main( int argc, char **argv )
{
    //rand
    srand(time(0));
    InitMainInfo();//初始化结构体
    printf("ngrokc v%s \r\n",VER);
	loadargs( argc, argv);
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
	mainInfo.lastdnsback	= net_dns( &mainInfo.saddr, mainInfo.shost, mainInfo.sport );
	mainInfo.lastdnstime	= getUnixTime();
    #if UDPCMD
    udpsocket=ControlUdp(udpport);
    #endif // UDPCMD
    #if UDPTUNNEL
    initUdp();
    #endif // UDPTUNNEL
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
 //   char ReqId[20]={0};
	int backcode=0;

	while ( true )
	{
        //ping
        checkping();//ping

        #if UDPTUNNEL
         //login
        CheckUdpAuth(udpInfo.msock);
        //reg tunnel
        CheckRegTunnel(udpInfo.msock);
        CheckUdpPing(udpInfo.msock);//ping
        #endif // UDPTUNNEL

        //控制链接断开,关闭所有连接
        if(mainInfo.mainsockstatus==0){
            shutdown( mainInfo.mainsock,2);
            //释放所有连接
            for ( it3 = G_SockList.begin(); it3 != G_SockList.end(); )
            {
                clearsock(it3->second);
                it3++;
            }
            G_SockList.clear();
            mainInfo.mainsock = 0;
            //改回状态
            mainInfo.mainsockstatus=1;
            //初始化通道
            InitTunnelList();
        }

	    if (mainInfo.lastdnsback == -1 ||(mainInfo.lastdnstime + 600) < getUnixTime())
		{
			mainInfo.lastdnsback	= net_dns( &mainInfo.saddr, mainInfo.shost, mainInfo.sport );
			mainInfo.lastdnstime	= getUnixTime();
			printf( "update dns\r\n" );
		}
		//dns解析成功
        if (mainInfo.lastdnsback != -1)
        {
            CheckStatus();
        }

        //注册端口
        if(G_SockList.count(mainInfo.mainsock)!=0&&mainInfo.mainsock!=0){

            tempinfo=G_SockList[mainInfo.mainsock];
            if(tempinfo->isauth==1&&mainInfo.regtunneltime+60<getUnixTime()){
                mainInfo.regtunneltime=getUnixTime();
                for ( listit =G_TunnelList.begin(); listit != G_TunnelList.end(); ++listit )
                {
                    tunnelinfo =(TunnelInfo	*)*listit;//udp不在这里注册
                    if(tunnelinfo->regstate==0&&strcasecmp(tunnelinfo->protocol,"udp")!=0){
                        #if OPENSSL
                        SendReqTunnel(mainInfo.mainsock,mainsslinfo->ssl,tunnelinfo);
                        #else
                        SendReqTunnel(mainInfo.mainsock,&mainsslinfo->ssl,tunnelinfo);
                        #endif
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

        #if UDPTUNNEL
        if(udpInfo.msock>0){
            maxfdp = udpInfo.msock > maxfdp ? udpInfo.msock : maxfdp;
            FD_SET(udpInfo.msock,&readSet );
            maxfdp = udpInfo.lsock > maxfdp ? udpInfo.lsock : maxfdp;
            FD_SET(udpInfo.lsock,&readSet );
        }
        #endif // UDPTUNNEL

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
            #if UDPTUNNEL
               UdpRecv(&readSet);
            #endif // UDPTUNNEL


			for ( it1 = G_SockList.begin(); it1 != G_SockList.end(); )
			{
			    tempinfo = it1->second;
			      //ping
                checkping();//这里添加个ping避免超时
                //判断连接超时sock
                if((tempinfo->linktime+10)<getUnixTime()&&tempinfo->isconnect==0)
                {
					//关闭远程连接
                    if(tempinfo->istype==2){
                        echo("connect local fail\r\n");
                        shutdown( tempinfo->tosock, 2 );
                    }
                    clearsock(tempinfo);
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
                            backcode=ConnectLocal(sslinfo1,tempinfo);
                            if(backcode==-1)
                            {
                              G_SockList.erase(it1++);
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
                            backcode=RemoteToLocal(sslinfo1,tempinfo);
                            if(backcode==-1)
                            {
                              G_SockList.erase(it1++);
                              continue;
                            }
                        }
                    }
                    /* 本地的转发给远程 */
                    else if(tempinfo->istype == 2){
                        backcode=LocalToRemote(tempinfo,sslinfo1);
                        if(backcode==-1)
                        {
                          G_SockList.erase(it1++);
                          continue;
                        }
                    }
                    //控制连接
                    else if(tempinfo->istype ==3){
                         backcode=CmdSock(&mainInfo.mainsock,tempinfo,mainInfo.saddr);
                         if(backcode==-1)
                         {
                             //控制链接断开，标记清空
                             mainInfo.mainsockstatus=0;
                             mainInfo.lasterrtime=getUnixTime();
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
							clearsock(tempinfo);
							G_SockList.erase(it1++);
							continue;
						}


						/* 为远程连接 */
						if ( tempinfo->istype == 1 )
						{
						    //初始化远程连接
                            backcode=RemoteSslInit(tempinfo);
                            if(backcode==-1)
                            {
                               G_SockList.erase(it1++);
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
