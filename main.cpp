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
#include "sslbio.h"



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

#include "sendmsg.h"
#include "cJSON.h"
#include "bytestool.h"
#include "nonblocking.h"
#include "ngrok.h"


using namespace std;
#define MAXBUF 2048

char s_name[255]="ngrokd.ngrok.com";
int	s_port= 443;
char authtoken[255]="1zW3MqEwX4iHmbtSAk3t";
string ClientId = "";
int		proxyrun	= 0;
int		pingtime	= 0;
int		ping		= 25;
int		linktime	= 45;
int		mainsock;
int		lastdnstime;
int		lastdnsback;
ssl_info *mainsslinfo=NULL;
void* sockmain( void *arg );
void* proxy( void *arg );

struct sockaddr_in server_addr = { 0 };
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
    pthread_attr_t attr;
    pthread_attr_init( &attr );
    pthread_attr_setdetachstate(&attr,1); //set detach
    pthread_attr_setstacksize(&attr,2048*1024); //set stacksize 2M
    sockinfo *tempinfo;
    TunnelInfo	*tunnelinfo;
	if ( proxyrun == 0 )
	{
		proxyrun = 1;
		pthread_t tproxy;
		pthread_create(&tproxy,&attr,&proxy,NULL);
	}


    //判断是否存在
	if (socklist.count(mainsock)==0)
	{
	    //连接失败
		if(ConnectMain(MAXBUF,&mainsock,server_addr,&mainsslinfo,&ClientId,mutex,&socklist,authtoken)==-1)
        {
            return -1;
        }
	}else  {
	    tempinfo=socklist[mainsock];
	    #if OPENSSL
		int sendlen = SendPing( mainsslinfo->ssl );
		#else
		int sendlen = SendPing( &mainsslinfo->ssl );
		#endif

		if ( sendlen < 1 || (tempinfo->pongtime < (pingtime - 35) && pingtime != 0) )
		{
			shutdown( mainsock, 2 );
			mainsock = 0;
		}
		pingtime = get_curr_unixtime();
	}
	//sleep 3ms
    sleeps(3000);
    tempinfo=socklist[mainsock];
	//检查是否
	if(socklist.count(mainsock)>0&&tempinfo->isauth==1)
    {
          /* 遍历添加 */
          map<string, TunnelInfo*>::iterator it;
          for ( it = tunnellist.begin(); it != tunnellist.end(); ++it )
          {
              if(tunneloklist.count(it->first)==0)
              {
                tunnelinfo = it->second;
                #if OPENSSL
                SendReqTunnel(mainsslinfo->ssl, it->first,tunnelinfo->hostname,tunnelinfo->subdomain, tunnelinfo->remoteport );
                #else
                SendReqTunnel(&mainsslinfo->ssl, it->first,tunnelinfo->hostname,tunnelinfo->subdomain, tunnelinfo->remoteport );
                #endif

              }
          }
    }
	return(0);
}


int main( int argc, char **argv )
{

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
	/* init addr */
	lastdnsback	= net_dns( &server_addr, s_name, s_port );
	lastdnstime	= get_curr_unixtime();

	while ( true )
	{
		if ( lastdnsback == -1 || (lastdnstime + 600) < get_curr_unixtime() )
		{
			lastdnsback	= net_dns( &server_addr, s_name, s_port );
			lastdnstime	= get_curr_unixtime();
			printf( "update dns\r\n" );
		}
		CheckStatus();
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
	int		cunixtime;
	TunnelInfo	*tunnelinfo	= NULL;
	int		maxfdp		= 0;
	int ret=0;
	ssl_info *sslinfo1;
	sockinfo *tempinfo ;
	map<int, sockinfo*>::iterator it;
	map<int, sockinfo*>::iterator it1;
	int backcode=0;
	while ( proxyrun )
	{
		timeout.tv_sec	= 0;
		timeout.tv_usec = 5000;
		FD_ZERO( &readSet );
		maxfd	= 0;
		maxfdp	= 0;
		FD_ZERO( &writeSet );
		cunixtime = get_curr_unixtime();
		/* 遍历添加 */
		//map<int, sockinfo*>::iterator it;
		for ( it = socklist.begin(); it != socklist.end();  )
		{
			tempinfo = it->second;
			/* 清理超时的错误的链接 */
			if ( tempinfo->istype == 1 )
			{
				if ( (tempinfo->linkunixtime + linktime) < cunixtime && tempinfo->isconnectlocal == 0 )
				{
					clearsock( it->first, it->second );
					pthread_mutex_lock( &mutex );
					socklist.erase(it++);
					pthread_mutex_unlock( &mutex );
					continue;
				}
			}

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
            printf("select error\r\n");
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

                    sslinfo1 = tempinfo->sslinfo;
                    /* 远程的转发给本地 */
                    if ( tempinfo->istype == 1 )
                    {
                        if ( tempinfo->isconnectlocal == 0 )
                        {
                            backcode=ConnectLocal(sslinfo1,(char *)buf,MAXBUF,&it1,tempinfo,&socklist,mutex,tempjson,&tunnellist,tunnelinfo);
                            if(backcode==-1)
                            {
                              continue;
                            }
                        }
                        else
                        {
                            backcode=RemoteToLocal(sslinfo1,MAXBUF,(char *)buf,tempinfo,&it1,&socklist,mutex);
                            if(backcode==-1)
                            {
                              continue;
                            }
                        }
                    }
                    /* 本地的转发给远程 */
                    else if(tempinfo->istype == 2){
                        backcode=LocalToRemote(&it1,(char*)buf,MAXBUF,tempinfo,sslinfo1,&socklist,mutex);
                        if(backcode==-1)
                        {
                          continue;
                        }
                    }
                    //控制连接
                    else if(tempinfo->istype ==3){
                         backcode=CmdSock(MAXBUF,(char *)buf,tempinfo,&socklist,mutex,tempjson,server_addr,&ClientId,&tunneloklist);
                         if(backcode==-1)
                         {
                            clearsock( it1->first, tempinfo );
                            pthread_mutex_lock( &mutex );
							socklist.erase(it1++);
							pthread_mutex_unlock( &mutex );
							mainsock=0;
							tunneloklist.clear();
							printf("main sock exit\r\n");
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
							clearsock( it1->first, tempinfo );
							pthread_mutex_lock( &mutex );
							socklist.erase(it1++);
							pthread_mutex_unlock( &mutex );
							continue;
						}

						/* 置为1 */
						tempinfo->isconnect = 1;

						/* 为远程连接 */
						if ( tempinfo->istype == 1 )
						{
						    //初始化远程连接
                            backcode=RemoteSslInit(&it1,tempinfo,ClientId,mutex,&socklist);
                            if(backcode==-1)
                            {
                              continue;
                            }
						}
						//本地连接
						else if(tempinfo->istype == 2) {
							pthread_mutex_lock( &mutex );
							it1->second = tempinfo;
							pthread_mutex_unlock( &mutex );
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

