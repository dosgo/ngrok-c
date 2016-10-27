#ifndef __NGROK_H__
#define __NGROK_H__
#include "config.h"
#include <string>
#include <map>
#include "sendmsg.h"
#include "nonblocking.h"
#include "cJSON.h"
#if OPENSSL
typedef SSL ssl_context;
#include "openssl/ssl.h"
#else
#if ISMBEDTLS
#include <mbedtls/ssl.h>
typedef mbedtls_ssl_context ssl_context;

#else
#include "polarssl/ssl.h"
#endif // ISMBEDTLS
#endif

int ControlUdp(int port);
int UdpCmd(int udpsocket);

int ReqProxy(struct sockaddr_in server_addr,map<int,sockinfo*>*socklist);

int NewTunnel(cJSON	*json,list<TunnelInfo*>*tunnellist,map<string,TunnelReq*> *tunneladdr);

int InitTunnelList(list<TunnelInfo*>*tunnellist,map<string,TunnelReq*>*tunneladdr);

int SetLocalAddrInfo(char *url,char *ReqId,int istunnel,list<TunnelInfo*>*tunnellist,map<string,TunnelReq*> *tunneladdr);

int RemoteSslInit(map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo,string &ClientId,map<int,sockinfo*>*socklist);

int LocalToRemote(map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo1,ssl_info *sslinfo1,map<int,sockinfo*>*socklist);

int RemoteToLocal(ssl_info *sslinfo1,sockinfo *tempinfo1,map<int, sockinfo*>::iterator *it1,map<int,sockinfo*>*socklist);

int ConnectLocal(ssl_info *sslinfo1,map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo1,map<int,sockinfo*>*socklist,map<string,TunnelReq*> *tunneladdr);
int CmdSock(int *mainsock,sockinfo *tempinfo,map<int,sockinfo*>*socklist,struct sockaddr_in server_addr,string *ClientId,char * authtoken,list<TunnelInfo*>*tunnellist,map<string,TunnelReq*> *tunneladdr);
int ConnectMain(int *mainsock,struct sockaddr_in server_addr,ssl_info **mainsslinfo,string *ClientId,map<int,sockinfo*>*socklist,char *authtoken,char *password_c);
#endif
