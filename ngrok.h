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


int ReqProxy(struct sockaddr_in server_addr,map<int,sockinfo*>*socklist);

int NewTunnel(cJSON	*json,map<string,int>*tunneloklist);

int RemoteSslInit(map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo,string &ClientId,map<int,sockinfo*>*socklist);

int LocalToRemote(map<int, sockinfo*>::iterator *it1,char *buf,int maxbuf,sockinfo *tempinfo1,ssl_info *sslinfo1,map<int,sockinfo*>*socklist);

int RemoteToLocal(ssl_info *sslinfo1,int maxbuf,char *buf,sockinfo *tempinfo1,map<int, sockinfo*>::iterator *it1,map<int,sockinfo*>*socklist);

int ConnectLocal(ssl_info *sslinfo1,char *buf,int maxbuf,map<int, sockinfo*>::iterator *it1,sockinfo *tempinfo1,map<int,sockinfo*>*socklist,char *tempjson,map<string,TunnelInfo*>*tunnellist,TunnelInfo *tunnelinfo);
int CmdSock(int *mainsock,int maxbuf,char *buf,sockinfo *tempinfo,map<int,sockinfo*>*socklist,char *tempjson,struct sockaddr_in server_addr,string *ClientId,map<string,int>*tunneloklist,map<string,TunnelInfo*>*tunnellist);
int ConnectMain(int maxbuf,int *mainsock,struct sockaddr_in server_addr,ssl_info **mainsslinfo,string *ClientId,map<int,sockinfo*>*socklist,char *authtoken);
#endif
