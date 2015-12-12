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






char *random_uuid( char buf[37] )
{
    srand((unsigned) time(NULL));
    const char *c = "89ab";
    char *p = buf;
    int n;
    for( n = 0; n < 16; ++n )
    {
        int b = rand()%255;
        switch( n )
        {
            case 6:
                sprintf(p, "4%x", b%15 );
            break;
            case 8:
                sprintf(p, "%c%x", c[rand()%strlen(c)], b%15 );
            break;
            default:
                sprintf(p, "%02x", b);
            break;
        }

        p += 2;

        switch( n )
        {
            case 3:
            case 5:
            case 7:
            case 9:
                break;
        }
    }
    *p = 0;
    return buf;
}

#if OPENSSL

int SendAuth(SSL *ssl,string ClientId,string user)
{
    string str="{\"Type\":\"Auth\",\"Payload\":{\"Version\":\"2\",\"MmVersion\":\"1.7\",\"User\":\""+user+"\",\"Password\": \"\",\"OS\":\"darwin\",\"Arch\":\"amd64\",\"ClientId\":\""+ClientId+"\"}}";
   // printf( "SendAuthstr:%s\r\n",str.c_str());
    unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
    int len=SSL_write(ssl, buffer, sendlen);
    return len;
}

int SendRegProxy(SSL *ssl,string &ClientId)
{
    string str="{\"Type\":\"RegProxy\",\"Payload\":{\"ClientId\":\""+ClientId+"\"}}";
    //printf( "SendRegProxystr:%s\r\n",str.c_str());
    unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
    int len=SSL_write( ssl, buffer, sendlen);
    return len;
}



int SendReqTunnel(SSL *ssl,string protocol,string HostName,string Subdomain,int RemotePort)
{
    char RemotePortStr[10];
    sprintf(RemotePortStr,"%d",RemotePort);
    char guid[37];
    random_uuid(guid);
    guid[9]='\0';
    string guid_str=string(guid);
    string str="{\"Type\":\"ReqTunnel\",\"Payload\":{\"Protocol\":\""+protocol+"\",\"ReqId\":\""+guid_str+"\",\"Hostname\": \""+HostName+"\",\"Subdomain\":\""+Subdomain+"\",\"HttpAuth\":\"\",\"RemotePort\":"+string(RemotePortStr)+"}}";
    //printf("SendReqTunnelstr:%s\r\n",str.c_str());
    unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
    int len=SSL_write(ssl, buffer,sendlen);
    return len;
}






#else
int SendAuth(ssl_context *ssl,string ClientId,string user)
{
    string str="{\"Type\":\"Auth\",\"Payload\":{\"Version\":\"2\",\"MmVersion\":\"1.7\",\"User\":\""+user+"\",\"Password\": \"\",\"OS\":\"darwin\",\"Arch\":\"amd64\",\"ClientId\":\""+ClientId+"\"}}";
   // printf( "SendAuthstr:%s\r\n",str.c_str());
    unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
    #if ISMBEDTLS
    int len=mbedtls_ssl_write(ssl, buffer, sendlen);
    #else
    int len=ssl_write(ssl, buffer, sendlen);
    #endif // ISMBEDTLS
    return len;
}

int SendRegProxy(ssl_context *ssl,string &ClientId)
{
    string str="{\"Type\":\"RegProxy\",\"Payload\":{\"ClientId\":\""+ClientId+"\"}}";
    //printf( "SendRegProxystr:%s\r\n",str.c_str());
    unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
    #if ISMBEDTLS
    int len=mbedtls_ssl_write(ssl, buffer, sendlen);
    #else
    int len=ssl_write(ssl, buffer, sendlen);
    #endif // ISMBEDTLS
    return len;
}



int SendReqTunnel(ssl_context *ssl,string protocol,string HostName,string Subdomain,int RemotePort)
{
    char RemotePortStr[10];
    sprintf(RemotePortStr,"%d",RemotePort);
    char guid[37];
    random_uuid(guid);
    guid[9]='\0';
    string guid_str=string(guid);
    string str="{\"Type\":\"ReqTunnel\",\"Payload\":{\"Protocol\":\""+protocol+"\",\"ReqId\":\""+guid_str+"\",\"Hostname\": \""+HostName+"\",\"Subdomain\":\""+Subdomain+"\",\"HttpAuth\":\"\",\"RemotePort\":"+string(RemotePortStr)+"}}";
    //printf("SendReqTunnelstr:%s\r\n",str.c_str());
    unsigned char buffer[str.length()+9];
    int sendlen=pack(buffer,str);
     #if ISMBEDTLS
    int len=mbedtls_ssl_write(ssl, buffer, sendlen);
    #else
    int len=ssl_write(ssl, buffer, sendlen);
    #endif // ISMBEDTLS
    return len;
}





#endif





int getlocaladdr( map<string,TunnelInfo *> *tunnellist,char *url, struct sockaddr_in* local_addr )
{
	char	Protocol[10]	= { 0 };
	int	plen		= strpos( url, ':' );
	memcpy( Protocol, url, plen );
	if ( tunnellist->count( string( Protocol ) ) > 0 )
	{
		TunnelInfo	*tunnelinfo	= (*tunnellist)[string( Protocol )];
		int		l1		= inet_addr( tunnelinfo->localhost );
		printf( "tunnelinfo->localhost %s\r\n", tunnelinfo->localhost );
		printf( "tunnelinfo->localhost \r\n" );
		local_addr->sin_family	= AF_INET;
		local_addr->sin_port	= htons(tunnelinfo->localport );
		memcpy( &local_addr->sin_addr, &l1, 4 );
		return 0;
	}
	return -1;
}



int loadargs( int argc, char **argv ,map<string, TunnelInfo*>*tunnellist,char *s_name,int * s_port,char * authtoken)
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
						pos = pos + xpos + 1;
					}
				}

				if ( strncmp( argvstr, "-AddTun", 7 ) == 0 )
				{
					run = 1;

					TunnelInfo *tunnelinfo = (TunnelInfo *) malloc( sizeof(TunnelInfo) );
					memset( tunnelinfo, 0, sizeof(TunnelInfo) );
					char Type[255] = { 0 };

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

						getvalue(jsonstr,"Type",Type);
                        getvalue(jsonstr,"Lhost",tunnelinfo->localhost);
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



                    int		l1		= inet_addr( tunnelinfo->localhost );
                    (&tunnelinfo->local_addr)->sin_family	= AF_INET;
                     (&tunnelinfo->local_addr)->sin_port	= htons(tunnelinfo->localport );
                     memcpy(&(&tunnelinfo->local_addr)->sin_addr, &l1, 4 );
					(*tunnellist)[string( Type )] = tunnelinfo;
				}
			}
		}
	}else  {
		printf( "use " );
        printf("%s",argv[0]);
		printf( " -SER[Shost:ngrokd.ngrok.com,Sport:443,Atoken:xxxxxxx] -AddTun[Type:tcp,Lhost:127.0.0.1,Lport:80,Rport:50199]" );
		printf( "\r\n" );
		exit( 1 );
	}
	return 0;
}


