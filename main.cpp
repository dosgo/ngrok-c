#include "config.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <map>
#include <list>
#include <iostream>
#include <iomanip>
#include <signal.h>




#if OPENSSL
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#else

#if ISMBEDTLS
#include <mbedtls/ssl.h>
#include <mbedtls/net.h>
#include <mbedtls/debug.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>
//typedef mbedtls_ssl_read ssl_read;
#else
#include <polarssl/net.h>
#include <polarssl/debug.h>
#include <polarssl/ssl.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <polarssl/error.h>
#include <polarssl/certs.h>
#endif // ISMBEDTLS

#endif


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
#define MAXBUF 2048
string VER = "1.13-(2016/3/12)";

char s_name[255]="ngrokd.ngrok.com";
int	s_port= 443;
char authtoken[255]="1zW3MqEwX4iHmbtSAk3t";
string ClientId = "";
int		proxyrun	= 0;
int		pingtime	= 0;
int		ping		= 25; //不能大于30
int		linktime	= 45;
int		mainsock=0;
int		lastdnstime=0;
int		lastdnsback;
ssl_info *mainsslinfo=NULL;
void* sockmain( void *arg );
void* proxy( void *arg );

struct sockaddr_in server_addr = { 0 };
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

map<int,sockinfo*>socklist;
map<string,TunnelInfo*>tunnellist;
map<string,int>tunneloklist;


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
    sockinfo *tempinfo;
	if ( proxyrun == 0 )
	{
		proxyrun = 1;
		pthread_t tproxy;
        pthread_attr_t attr;
        pthread_attr_init( &attr );
        pthread_attr_setdetachstate(&attr,1); //set detach
        pthread_attr_setstacksize(&attr,2048*1024); //set stacksize 2M
		pthread_create(&tproxy,&attr,&proxy,NULL);
	}
    //判断是否存在
	if (socklist.count(mainsock)==0||mainsock==0)
	{
	    //连接失败
        if(ConnectMain(MAXBUF,&mainsock,server_addr,&mainsslinfo,&ClientId,&socklist,authtoken)==-1)
        {
            printf("link err\r\n");
            return -1;
        }

	}
	else
    {   //检测心跳时间
	    tempinfo=socklist[mainsock];
		if ((tempinfo->pongtime < (pingtime-ping-10) && pingtime != 0) )
		{
			shutdown( mainsock, 2 );
			mainsock = 0;
		}
	}
	return(0);
}


int main( int argc, char **argv )
{
	#if WIN32
	#else
		#if DEBUG
		int pid = getpid();
	 	printf("The pid is:%d\n", pid);
	 	signal(SIGSEGV, OutputBacktrace);
		#endif
	#endif

    	printf("ngrokc v%s \r\n",VER.c_str());
	loadargs( argc, argv, &tunnellist, s_name, &s_port, authtoken );
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
SSL_library_init();
SSL_load_error_strings();
OpenSSL_add_all_algorithms();
#endif // OPENSSL
    init_ssl_session();

	/* init addr */
	lastdnsback	= net_dns( &server_addr, s_name, s_port );
	lastdnstime	= get_curr_unixtime();

	while ( true )
	{

		if ( lastdnsback == -1 ||(lastdnstime + 600) < get_curr_unixtime())
		{
			lastdnsback	= net_dns( &server_addr, s_name, s_port );
			lastdnstime	= get_curr_unixtime();
			printf( "update dns\r\n" );
		}
		//dns解析失败
        if (lastdnsback != -1)
        {
            CheckStatus();
        }
		sleeps(ping * 1000);
	}
	return(0);
}


void* proxy( void *arg )
{
	pthread_detach( pthread_self() );
	fd_set	writeSet;
	fd_set	readSet;
	int		maxfd = 0;
	unsigned char	buf[MAXBUF];
	char		tempjson[MAXBUF + 1];
	struct timeval	timeout;
	TunnelInfo	*tunnelinfo	= NULL;
	int		maxfdp		= 0;
	int ret=0;
	ssl_info *sslinfo1;
	sockinfo *tempinfo ;
	sockinfo *tempinfo1 ;
	map<int, sockinfo*>::iterator it;
	map<int, sockinfo*>::iterator it1;
	map<int, sockinfo*>::iterator it2;
	int backcode=0;
	while ( proxyrun )
	{

	    if(pingtime+ping<get_curr_unixtime()&&socklist.count(mainsock)!=0&&mainsock!=0)
        {

            #if OPENSSL
            int sendlen = SendPing(mainsock, mainsslinfo->ssl );
            #else
            int sendlen = SendPing(mainsock, &mainsslinfo->ssl );
            #endif
            //发送失败断开连接
            if(sendlen==-1)
            {
                shutdown( mainsock,2);
                mainsock = 0;
                proxyrun=0;
            }
            pingtime = get_curr_unixtime();
	    }

		timeout.tv_sec	= 0;
		timeout.tv_usec = 5000;
		FD_ZERO( &readSet );
		maxfd	= 0;
		maxfdp	= 0;
		FD_ZERO( &writeSet );

		/* 遍历添加 */
		//map<int, sockinfo*>::iterator it;
		for ( it = socklist.begin(); it != socklist.end();  )
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
			++it;
		}





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
			for ( it1 = socklist.begin(); it1 != socklist.end(); )
			{
			    tempinfo = it1->second;
			    /*等于1才是添加到 readSet的*/
				if (FD_ISSET( it1->first, &readSet )&&tempinfo->isconnect==1 )
				{
                    //先清空
                    memset((char *)buf,0,MAXBUF);
                    sslinfo1 = tempinfo->sslinfo;
                    /* 远程的转发给本地 */
                    if ( tempinfo->istype == 1 )
                    {
                        if ( tempinfo->isconnectlocal == 0 )
                        {
                            backcode=ConnectLocal(sslinfo1,(char *)buf,MAXBUF,&it1,tempinfo,&socklist,tempjson,&tunnellist,tunnelinfo);
                            if(backcode==-1)
                            {
                              continue;
                            }
                        }

                        if( tempinfo->isconnectlocal == 2||tempinfo->isconnectlocal == 1  )
                        {
                            backcode=RemoteToLocal(sslinfo1,MAXBUF,(char *)buf,tempinfo,&it1,&socklist);
                            if(backcode==-1)
                            {
                              continue;
                            }
                        }
                    }
                    /* 本地的转发给远程 */
                    else if(tempinfo->istype == 2){
                        backcode=LocalToRemote(&it1,(char*)buf,MAXBUF,tempinfo,sslinfo1,&socklist);
                        if(backcode==-1)
                        {
                          continue;
                        }
                    }
                    //控制连接
                    else if(tempinfo->istype ==3){
                         backcode=CmdSock(&mainsock,MAXBUF,(char *)buf,tempinfo,&socklist,tempjson,server_addr,&ClientId,authtoken,&tunneloklist,&tunnellist);
                         if(backcode==-1)
                         {

                            clearsock( it1->first, tempinfo );
							socklist.erase(it1++);
							mainsock=0;
							tunneloklist.clear();
                            continue;
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
							socklist.erase(it1++);
							continue;
						}

						/* 置为1 */
						tempinfo->isconnect = 1;

						/* 为远程连接 */
						if ( tempinfo->istype == 1 )
						{
						    //初始化远程连接
                            backcode=RemoteSslInit(&it1,tempinfo,ClientId,&socklist);
                            if(backcode==-1)
                            {
                              continue;
                            }
						}

						//本地连接
                        if ( tempinfo->istype == 2 )
						{
                            it2 = socklist.find(tempinfo->tosock);
                            if(it2 != socklist.end())
                            {
                                tempinfo1 = it2->second;
                                tempinfo1->isconnectlocal=2;
                                /* copy到临时缓存区 */
                                if ( tempinfo1->packbuflen > 0 )
                                {
                                    setnonblocking( it1->first, 0 );
                                    #if WIN32
                                    send(it1->first,(char *)tempinfo1->packbuf,tempinfo1->packbuflen, 0 );
                                    #else
                                    send( it1->first, tempinfo1->packbuf, tempinfo1->packbuflen, 0 );
                                    #endif
                                    setnonblocking(it1->first, 1 );
                                    free( tempinfo1->packbuf );
                                    tempinfo1->packbuf	= NULL;
                                    tempinfo1->packbuflen	= 0;
                                }
                            }
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
	/* 退出 */
	proxyrun = 0;
	return(0);
}
