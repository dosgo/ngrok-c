#ifndef OPENSSLBIO_H_INCLUDED
#define OPENSSLBIO_H_INCLUDED
#include "config.h"
#include<malloc.h>
#include<openssl/ssl.h>
#include<openssl/bio.h>
#include<openssl/err.h>
struct openssl_info
{
    SSL *ssl;
    SSL_CTX *ctx;

};
int ssl_init_info(int server_fd,openssl_info *sslinfo);
int ssl_free_info(openssl_info *sslinfo);
typedef openssl_info ssl_info;
typedef SSL ssl_context;

#if OPENSSLDL
#include "openssldl.h"
#endif // OPENSSLDL
inline int SslRecv(SSL* ssl, char* buffer, int ilen)
{
  #if OPENSSLDL
   int  r=SslRead(ssl,buffer,ilen);
   switch(SslGetError(ssl,r)){
  #else
     int  r=SSL_read(ssl,buffer,ilen);
     switch(SSL_get_error(ssl,r)){
  #endif

    case SSL_ERROR_NONE:
      return r;
    case SSL_ERROR_ZERO_RETURN:
      return 0;
    case SSL_ERROR_WANT_READ:
      return -1;
    case SSL_ERROR_WANT_WRITE:
      return -1;
  }
  return -2;
}


inline int ssl_free_info(openssl_info *sslinfo)
{
    #if OPENSSLDL
    SslShutdown( sslinfo->ssl );
    SslFree( sslinfo->ssl );
    SslCtxFree( sslinfo->ctx  );
    #else
    SSL_shutdown( sslinfo->ssl );
    SSL_free( sslinfo->ssl );
    SSL_CTX_free( sslinfo->ctx  );
    #endif //
    return 0;
}


inline int init_ssl_session(){

    return 0;
}
#endif // OPENSSLBIO_H_INCLUDED



