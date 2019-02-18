#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <string>
#include <map>
#include <list>

#include "sslbio.h"
using namespace std;

#if WIN32
#else
#include <netinet/in.h>
#endif

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
    char shost[255];
    char udphost[255];
    sockaddr_in saddr;
    int sport;
    int udpport;
    int udp;
    char authtoken[255];
    char pwdc[255];//
    string ClientId ;
    int pingtime	;
    int ping	; //
    int mainsock;
    int lastdnstime;
    int mainsockstatus;
    int regtunneltime;
    int lastdnsback;
    int lasterrtime;
    map<string,TunnelReq*> G_TunnelAddr;
};
#if UDPTUNNEL
struct UdpInfo
 {
      int auth;  //login  flag
      int authtime;  //login  flag
      int regTunnel;//login  flag
      int pingtime;
      int pongtime;
      int ping; //不能大于30
      int msock;
      int lsock;
      sockaddr_in servAddr;
      string ClientId;
      string authtoken;
      string password_c;
      map<string,TunnelReq*>G_TunnelAddr;
};
extern  UdpInfo udpInfo;
#endif

extern MainInfo mainInfo;
extern  map<int,Sockinfo*>G_SockList;
extern  list<TunnelInfo*> G_TunnelList;


inline void InitMainInfo(){
    memset(mainInfo.authtoken,0,255);
    memset(mainInfo.pwdc,0,255);
    memset(mainInfo.shost,0,255);
    memset(mainInfo.udphost,0,255);
    mainInfo.ping=25;
    mainInfo.sport=443;
    mainInfo.udpport=8001;
    mainInfo.udp=0;
    mainInfo.pingtime=0;
    mainInfo.mainsock=0;
    mainInfo.lastdnstime=0;
    mainInfo.regtunneltime=0;
    mainInfo.lastdnsback=0;
    mainInfo.lasterrtime=0;
    sprintf(mainInfo.shost,"ngrokd.ngrok.com");
    sprintf(mainInfo.udphost,"127.0.0.1");
    #if UDPTUNNEL
    udpInfo.auth=0;
    udpInfo.authtime=0;
    udpInfo.ping=25;
    udpInfo.regTunnel=0;
    udpInfo.pongtime=0;
    udpInfo.pingtime=0;
    #endif
}

#endif
