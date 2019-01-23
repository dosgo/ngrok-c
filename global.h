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
    int sock;
    unsigned char *packbuf;
    unsigned long long packbuflen;
    int isconnectlocal;
    int linktime;
    int isauth;
};

struct MainInfo
{
    char s_name[255];
    int s_port;
    char authtoken[255];
    char password_c[255];//
    string ClientId ;
    int pingtime	;
    int ping	; //
    int mainsock;
    int lastdnstime;
    int mainsockstatus;
    int regtunneltime;
    int lastdnsback;
    int lasterrtime;
};


extern MainInfo mainInfo;
extern  map<int,Sockinfo*>G_SockList;
extern  list<TunnelInfo*> G_TunnelList;
extern  map<string,TunnelReq*> G_TunnelAddr;

inline void InitMainInfo(){
    memset(mainInfo.authtoken,0,255);
    memset(mainInfo.password_c,0,255);
    memset(mainInfo.s_name,0,255);
    mainInfo.ping=25;
    mainInfo.s_port=443;
    mainInfo.pingtime=0;
    mainInfo.mainsock=0;
    mainInfo.lastdnstime=0;
    mainInfo.regtunneltime=0;
    mainInfo.lastdnsback=0;
    mainInfo.lasterrtime=0;
    sprintf(mainInfo.s_name,"ngrokd.ngrok.com");
}

#endif
