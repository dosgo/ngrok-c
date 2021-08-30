#include "config.h"
#include <string>

#if WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#include <errno.h>
typedef long long __int64;
#endif
#include "param.h"
#include <stdlib.h>
using namespace std;




int loadargs(int argc, char **argv)
{
	if (argc > 1)
	{
		char jsonstr[1024];
		char temp[255];
		int pos = 0;
		char *argvstr;
		int xpos;
		int run = 1;
		for (int i = 1; i < argc; i++)
		{
			argvstr = argv[i];
			memset(jsonstr, 0, 1024);
			memset(temp, 0, 255);
			pos = strpos(argvstr, '[');
			if (pos == -1)
			{
				printf("argv error:%s", argvstr);
			}
			else
			{
				if (strncmp(argvstr, "-SER", 4) == 0)
				{
					run = 1;
					while (run)
					{
						memset(jsonstr, 0, 1024);
						xpos = strpos(argvstr + pos + 1, ',');
						if (xpos == -1)
						{
							xpos = strpos(argvstr + pos + 1, ']');
							memcpy(jsonstr, argvstr + pos + 1, xpos);
							run = 0;
						}
						else
						{
							memcpy(jsonstr, argvstr + pos + 1, xpos);
						}
						getvalue(jsonstr, "Shost", mainInfo.shost);
						if (getvalue(jsonstr, "Sport", temp) == 0)
						{
							mainInfo.sport = atoi(temp);
						}
						getvalue(jsonstr, "Atoken", mainInfo.authtoken);
						getvalue(jsonstr, "Password", mainInfo.pwdc);

						if (getvalue(jsonstr, "Cid", temp) == 0)
						{
							mainInfo.ClientId = string(temp);
						}

						pos = pos + xpos + 1;
					}
				}
#if UDPTUNNEL
				if (strncmp(argvstr, "-UDPSER", 7) == 0)
				{
					run = 1;
					while (run)
					{
						memset(jsonstr, 0, 1024);
						xpos = strpos(argvstr + pos + 1, ',');
						if (xpos == -1)
						{
							xpos = strpos(argvstr + pos + 1, ']');
							memcpy(jsonstr, argvstr + pos + 1, xpos);
							run = 0;
						}
						else
						{
							memcpy(jsonstr, argvstr + pos + 1, xpos);
						}
						getvalue(jsonstr, "Shost", mainInfo.udphost);
						if (getvalue(jsonstr, "Sport", temp) == 0)
						{
							mainInfo.udpport = atoi(temp);
						}
						pos = pos + xpos + 1;
					}
					mainInfo.udp = 1;
				}
#endif
				if (strncmp(argvstr, "-AddTun", 7) == 0)
				{
					run = 1;

					TunnelInfo *tunnelinfo = (TunnelInfo *)malloc(sizeof(TunnelInfo));
					memset(tunnelinfo, 0, sizeof(TunnelInfo));

					while (run)
					{
						memset(jsonstr, 0, 1024);
						xpos = strpos(argvstr + pos + 1, ',');
						if (xpos == -1)
						{
							xpos = strpos(argvstr + pos + 1, ']');
							memcpy(jsonstr, argvstr + pos + 1, xpos);
							run = 0;
						}
						else
						{
							memcpy(jsonstr, argvstr + pos + 1, xpos);
						}

						getvalue(jsonstr, "Lhost", tunnelinfo->localhost);
						getvalue(jsonstr, "Type", tunnelinfo->protocol);
						memset(temp, 0, strlen(temp));
						if (getvalue(jsonstr, "Lport", temp) == 0)
						{
							tunnelinfo->localport = atoi(temp);
						}
						if (getvalue(jsonstr, "Ltls", temp) == 0)
						{
							tunnelinfo->localtls = atoi(temp);
						}
						memset(temp, 0, strlen(temp));
						if (getvalue(jsonstr, "Rport", temp) == 0)
						{
							tunnelinfo->remoteport = atoi(temp);
						}

						getvalue(jsonstr, "Sdname", tunnelinfo->subdomain);
						getvalue(jsonstr, "Hostheader", tunnelinfo->hostheader);
						getvalue(jsonstr, "Httpauth", tunnelinfo->httpauth);
						//printf("httpAuth:%s\r\n",tunnelinfo->httpauth);
						getvalue(jsonstr, "Hostname", tunnelinfo->hostname);
						pos = pos + xpos + 1;
					}

					G_TunnelList.push_back(tunnelinfo);
				}
			}
		}
	}
	else
	{
		echo("use ");
		echo("%s", argv[0]);
		echo(" -SER[Shost:ngrokd.ngrok.com,Sport:443,Atoken:xx,Password:xx] -AddTun[Type:tcp,Lhost:127.0.0.1,Lport:80,Rport:50199,Hostheader:localhost,Httpauth:\"test:test\"]");
		echo("\r\n");
		exit(1);
	}
	return 0;
}
