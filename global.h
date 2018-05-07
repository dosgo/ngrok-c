#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <string>
#include <map>
#include <list>

#include "sslbio.h"
using namespace std;

struct TunnelInfo
{
    char localhost[255];
    int localport;
    char hostheader[255];
    char subdomain[255];
    char hostname[255];
    char httpauth[255];
    char protocol[10];
    char ReqId[20];
    int remoteport;
    int regtime;
    int regstate;
};

struct TunnelReq
{
    char localhost[255];
    char url[255];
    char hostheader[255];
    int localport;
    int regtime;
};


struct Sockinfo
{
    ssl_info *sslinfo;
    TunnelReq *tunnelreq;
    int isconnect;
    int istype; //1=remote 2=local,3=cmd
    int tosock;
    unsigned char *packbuf;
    unsigned long long packbuflen;
    int isconnectlocal;
    int linktime;
    int isauth;
};


extern  map<int,Sockinfo*>G_SockList;
extern  list<TunnelInfo*> G_TunnelList;
extern  map<string,TunnelReq*> G_TunnelAddr;
#endif
