#include "nonblocking.h"



#if OPENSSL
void clearsock(Sockinfo * sock_info)
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
              ssl_free_info(sock_info->sslinfo);
              free(sock_info->sslinfo);
              sock_info->sslinfo=NULL;
          }
      }
   //   net_close(sock);

      shutdown(sock_info->sock,2);
      //closesocket(sock);
      #if WIN32
      closesocket(sock_info->sock);
      #else
      close(sock_info->sock);
      #endif
      //ÊÍ·ÅÄÚ´æ
      if(sock_info!=NULL)
      {
        free(sock_info);
        sock_info=NULL;
      }

}
#else
void clearsock(Sockinfo * sock_info)
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
      shutdown(sock_info->sock,2);
      //closesocket(sock_info->sock);
      #if WIN32
      closesocket(sock_info->sock);
      #else
      close(sock_info->sock);
      #endif

      #else
      shutdown(sock_info->sock,2);
      net_close(sock_info->sock);
      #endif

      //ÊÍ·ÅÄÚ´æ
      if(sock_info!=NULL)
      {
        free(sock_info);
        sock_info=NULL;
      }

}
#endif
