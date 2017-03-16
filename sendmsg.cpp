#include "config.h"
#include <string>

#if WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#include <errno.h>
typedef long long __int64;
#endif
#include "sendmsg.h"
#include<stdlib.h>
using namespace std;



char *rand_str(char *str,const int len)
{
    int i;
    for(i=0;i<len;++i)
        str[i]='A'+rand()%26;
    str[++i]='\0';
    return str;
}



int SendReqTunnel(int sock,ssl_context *ssl,char *ReqId,const char *protocol,const char * HostName,const char * Subdomain,int RemotePort,char *authtoken)
{
    char guid[20]={0};
    rand_str(guid,5);
    char str[1024];
    memset(str,0,1024);
    memcpy(ReqId,guid,strlen(guid));//copy
    sprintf(str,"{\"Type\":\"ReqTunnel\",\"Payload\":{\"Protocol\":\"%s\",\"ReqId\":\"%s\",\"Hostname\": \"%s\",\"Subdomain\":\"%s\",\"HttpAuth\":\"\",\"RemotePort\":%d,\"authtoken\":\"%s\"}}",protocol,guid,HostName,Subdomain,RemotePort,authtoken);
    return sendpack(sock,ssl,str,1);
}



int loadargs( int argc, char **argv ,list<TunnelInfo*>*tunnellist,char *s_name,int * s_port,char * authtoken,char *password_c,string *ClientId)
{
	if ( argc > 1 )
	{
		char	jsonstr[1024];
		char	temp[255];
		int	pos	= 0;
		char *argvstr;
		int	xpos;
		int	run = 1;
		for ( int i = 1; i < argc; i++ )
		{
			argvstr = argv[i];
			memset( jsonstr, 0, 1024 );
			memset( temp, 0, 255 );
			pos = strpos( argvstr, '[' );
			if ( pos == -1 )
			{
				printf( "argv error:%s", argvstr );
			}else  {
				if ( strncmp( argvstr, "-SER", 4 ) == 0 )
				{
					run = 1;
					while ( run )
					{
						memset( jsonstr, 0, 1024 );
						xpos = strpos( argvstr + pos + 1, ',' );
						if ( xpos == -1 )
						{
							xpos = strpos( argvstr + pos + 1, ']' );
							memcpy( jsonstr, argvstr + pos + 1, xpos );
							run = 0;
						}else  {
							memcpy( jsonstr, argvstr + pos + 1, xpos );
						}
						getvalue(jsonstr,"Shost",s_name);
						if(getvalue(jsonstr,"Sport",temp)==0)
                        {
                            *s_port = atoi(temp);
						}
						getvalue(jsonstr,"Atoken",authtoken);
						getvalue(jsonstr,"Password",password_c);

						if(getvalue(jsonstr,"Cid",temp)==0)
                        {
                            *ClientId = string( temp );
						}

						pos = pos + xpos + 1;
					}
				}

				if ( strncmp( argvstr, "-AddTun", 7 ) == 0 )
				{
					run = 1;

					TunnelInfo *tunnelinfo = (TunnelInfo *) malloc( sizeof(TunnelInfo) );
					memset( tunnelinfo, 0, sizeof(TunnelInfo) );


					while ( run )
					{
						memset( jsonstr, 0, 1024 );
						xpos = strpos( argvstr + pos + 1, ',' );
						if ( xpos == -1 )
						{
							xpos = strpos( argvstr + pos + 1, ']' );
							memcpy( jsonstr, argvstr + pos + 1, xpos );
							run = 0;
						}else  {
							memcpy( jsonstr, argvstr + pos + 1, xpos );
						}


                        getvalue(jsonstr,"Lhost",tunnelinfo->localhost);
                        getvalue(jsonstr,"Type",tunnelinfo->protocol);
                        memset( temp, 0, strlen( temp ) );
                        if(getvalue(jsonstr,"Lport",temp)==0)
                        {
                            tunnelinfo->localport = atoi( temp );
                        }
                        memset( temp,0,strlen(temp));
                        if(getvalue(jsonstr,"Rport",temp)==0)
                        {
                            tunnelinfo->remoteport = atoi( temp );
                        }
                        getvalue(jsonstr,"Sdname",tunnelinfo->subdomain);
                        getvalue(jsonstr,"Hostname",tunnelinfo->hostname);
						pos = pos + xpos + 1;
					}

					(*tunnellist).push_back(tunnelinfo);
				}
			}
		}
	}else  {
		echo( "use " );
        echo("%s",argv[0]);
		echo( " -SER[Shost:ngrokd.ngrok.com,Sport:443,Atoken:xxxxxxx,Password:xxx] -AddTun[Type:tcp,Lhost:127.0.0.1,Lport:80,Rport:50199]" );
		echo( "\r\n" );
		exit( 1 );
	}
	return 0;
}


