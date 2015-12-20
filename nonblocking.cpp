#include "nonblocking.h"



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
      shutdown(sock,2);
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
