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

int RemoteSslInit(map<int, Sockinfo*>::iterator *it1,Sockinfo *tempinfo,string &ClientId);

int LocalToRemote(map<int, Sockinfo*>::iterator *it1,Sockinfo *tempinfo1,ssl_info *sslinfo1);

int RemoteToLocal(ssl_info *sslinfo1,Sockinfo *tempinfo1,map<int, Sockinfo*>::iterator *it1);

int ConnectLocal(ssl_info *sslinfo1,map<int, Sockinfo*>::iterator *it1,Sockinfo *tempinfo1);
int CmdSock(int *mainsock,Sockinfo *tempinfo,struct sockaddr_in server_addr,string *ClientId,char * authtoken);
int ConnectMain(int *mainsock,struct sockaddr_in server_addr,ssl_info **mainsslinfo,string *ClientId,char *authtoken,char *password_c);
#endif
