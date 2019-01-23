#ifndef __NGROK_H__
#define __NGROK_H__
#include "config.h"
#include <string>
#include <map>
#include "sendmsg.h"
#include "nonblocking.h"
#include "cJSON.h"



int ControlUdp(int port);
int UdpCmd(int udpsocket);
int ReqProxy(struct sockaddr_in server_addr);
int NewTunnel(cJSON	*json);
int InitTunnelList();
int SetLocalAddrInfo(char *url,char *ReqId,int istunnel);
int RemoteSslInit(Sockinfo *tempinfo);
int LocalToRemote(Sockinfo *tempinfo,ssl_info *sslinfo);
int RemoteToLocal(ssl_info *sslinfo,Sockinfo *tempinfo);
int ConnectLocal(ssl_info *sslinfo,Sockinfo *tempinfo);
int CmdSock(int *mainsock,Sockinfo *tempinfo,struct sockaddr_in server_addr);
int ConnectMain(int *mainsock,struct sockaddr_in server_addr,ssl_info **mainsslinfo);
#endif
