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
#include <stdlib.h>
using namespace std;

char *rand_str(char *str, const int len)
{
	int i;
	for (i = 0; i < len; ++i)
		str[i] = 'A' + rand() % 26;
	str[++i] = '\0';
	return str;
}

int SendReqTunnel(int sock, ssl_context *ssl, TunnelInfo *tunnelInfo)
{
	char guid[20] = {0};
	rand_str(guid, 5);
	char str[1024];
	memset(str, 0, 1024);
	memset(tunnelInfo->ReqId, 0, 20);
	//copy
	memcpy(tunnelInfo->ReqId, guid, strlen(guid));
	sprintf(str, "{\"Type\":\"ReqTunnel\",\"Payload\":{\"Protocol\":\"%s\",\"ReqId\":\"%s\",\"Hostname\": \"%s\",\"Subdomain\":\"%s\",\"HttpAuth\":\"%s\",\"RemotePort\":%d,\"authtoken\":\"%s\"}}", tunnelInfo->protocol, tunnelInfo->ReqId, tunnelInfo->hostname, tunnelInfo->subdomain, tunnelInfo->httpauth, tunnelInfo->remoteport, mainInfo.authtoken);
	tunnelInfo->regtime = getUnixTime(); //已发
	return sendpack(sock, ssl, str, 1);
}
