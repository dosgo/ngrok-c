#include "global.h"

  map<int,Sockinfo*>G_SockList;
  list<TunnelInfo*> G_TunnelList;
  MainInfo mainInfo;
#if UDPTUNNEL
UdpInfo udpInfo;
#endif
