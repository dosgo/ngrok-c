#include "nonblocking.h"
int setnonblocking(int sServer,int _nMode)
{
    #if WIN32
    DWORD nMode = _nMode;
    return ioctlsocket( sServer, FIONBIO,&nMode);
    #else
    if(_nMode==1)
    {
       return fcntl(sServer,F_SETFL,O_NONBLOCK);
    }
    else
    {
      return fcntl(sServer,F_SETFL, _nMode);
    }
    #endif
}


#if OPENSSL
void clearsock(int sock,sockinfo * sock_info)
{

      if(sock_info->istype==1)
      {
            if(sock_info->packbuflen>0&&sock_info->packbuf!=NULL)
           {
              free(sock_info->packbuf);
              sock_info->packbuf=NULL;
           }

          if(sock_info->sslinfo!=NULL)
          {
              openssl_free_info(sock_info->sslinfo);
              free(sock_info->sslinfo);
              sock_info->sslinfo=NULL;
          }
      }
   //   net_close(sock);

      shutdown(sock,2);
      //closesocket(sock);
      #if WIN32
      closesocket(sock);
      #else
      close(sock);
      #endif
      //ÊÍ·ÅÄÚ´æ
      if(sock_info!=NULL)
      {
        free(sock_info);
        sock_info=NULL;
      }

}
#else
void clearsock(int sock,sockinfo * sock_info)
{

      if(sock_info->istype==1)
      {
            if(sock_info->packbuflen>0&&sock_info->packbuf!=NULL)
           {
              free(sock_info->packbuf);
              sock_info->packbuf=NULL;
           }

          if(sock_info->sslinfo!=NULL)
          {
               #if ISMBEDTLS
               mbedtls_ssl_close_notify(&sock_info->sslinfo->ssl);
               #else
               ssl_close_notify(&sock_info->sslinfo->ssl);
               #endif // ISMBEDTLS
               ssl_free_info(sock_info->sslinfo);
               free(sock_info->sslinfo);
               sock_info->sslinfo=NULL;
          }
      }

      #if ISMBEDTLS
      shutdown(sock,2);
      //closesocket(sock);
      #if WIN32
      closesocket(sock);
      #else
      close(sock);
      #endif

      #else
      net_close(sock);
      #endif

      //ÊÍ·ÅÄÚ´æ
      if(sock_info!=NULL)
      {
        free(sock_info);
        sock_info=NULL;
      }

}
#endif



int check_sock(int sock)
{
    int error=-1;
    #if WIN32
    int len ;
    #else
    socklen_t len;
    #endif
    len = sizeof(error);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
    return error;
}



int net_dns( struct sockaddr_in *server_addr, const char *host, int port )
{
    struct hostent *server_host;
    if((server_host = gethostbyname(host)) == NULL )
    {
        return -1;
    }
    memcpy((void*)&server_addr->sin_addr,(void*)server_host->h_addr,server_host->h_length);
    server_addr->sin_family = AF_INET;
    server_addr->sin_port   = htons( port );
    return 0;
}
