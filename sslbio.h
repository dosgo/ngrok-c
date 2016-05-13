#ifndef SSLBIO_H_INCLUDED
#define SSLBIO_H_INCLUDED
#include "config.h"
#include<malloc.h>




#if OPENSSL

#include<openssl/ssl.h>
#include<openssl/bio.h>
#include<openssl/err.h>
struct openssl_info
{
    SSL *ssl;
    SSL_CTX *ctx;

};
int openssl_init_info(int server_fd,openssl_info *sslinfo);
int openssl_free_info(openssl_info *sslinfo);
typedef openssl_info ssl_info;

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

inline int openssl_free_info(openssl_info *sslinfo)
{
    #if OPENSSLDL
    SslShutdown( sslinfo->ssl );
    SslFree( sslinfo->ssl );
    SslCtxFree( sslinfo->ctx  );
    #else
    SSL_shutdown( sslinfo->ssl );
    SSL_free( sslinfo->ssl );
    SSL_CTX_free( sslinfo->ctx  );
    #endif // OPENSSLDL
   //CRYPTO_cleanup_all_ex_data();
    //ERR_remove_state();
    return 0;
}


#else

#if ISMBEDTLS
#include <mbedtls/ssl.h>
#include <mbedtls/net.h>
#include <mbedtls/debug.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>
typedef mbedtls_entropy_context entropy_context;
typedef mbedtls_ctr_drbg_context ctr_drbg_context;
typedef mbedtls_ssl_context ssl_context;
typedef mbedtls_ssl_session ssl_session;
typedef mbedtls_x509_crt x509_crt;
typedef mbedtls_x509_crt x509_crt;
static mbedtls_ssl_session ssn;
struct ssl_info
{
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    ssl_context ssl;
    mbedtls_ssl_config conf;
    x509_crt cacert;
};

inline int SslRecv( ssl_context *ssl,unsigned char* buffer, int ilen)
{
    int ret = mbedtls_ssl_read(ssl, buffer, ilen );
    if( ret == MBEDTLS_ERR_SSL_WANT_READ ||ret == MBEDTLS_ERR_SSL_WANT_WRITE )
    {
       return -1;
    }

    if( ret <= 0 )
    {
        switch( ret )
        {
        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
           return -2;
        case MBEDTLS_ERR_NET_CONN_RESET:
           return -2;
        }

    }
    return ret;
}



#else
#include <polarssl/net.h>
#include <polarssl/debug.h>
#include <polarssl/ssl.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <polarssl/error.h>
#include <polarssl/certs.h>
static ssl_session ssn;

inline int SslRecv( ssl_context *ssl,unsigned char* buffer, int ilen)
{
    int ret = ssl_read(ssl, buffer, ilen );
    if( ret <= 0 )
    {
        switch( ret )
        {
        case POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY:
           return -2;
        case POLARSSL_ERR_NET_CONN_RESET:
           return -2;
        }

    }
    return ret;
}


struct ssl_info
{
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    ssl_context ssl;
    x509_crt cacert;
};
#endif // ISMBEDTLS
int ssl_init_info(int *server_fd,ssl_info *sslinfo);

inline int ssl_free_info(ssl_info *sslinfo){

    #if ISMBEDTLS
    mbedtls_x509_crt_free(&sslinfo->cacert );
    mbedtls_ssl_free(&sslinfo->ssl);
    mbedtls_ssl_config_free( &sslinfo->conf );
    mbedtls_ctr_drbg_free(&sslinfo->ctr_drbg );
    mbedtls_entropy_free(&sslinfo->entropy );
    #else

    x509_crt_free(&sslinfo->cacert);
    ssl_free(&sslinfo->ssl);
    ctr_drbg_free(&sslinfo->ctr_drbg);
    entropy_free(&sslinfo->entropy);
    #endif // ISMBEDTLS
    return 0;
}
#endif

inline int init_ssl_session(){
    #if OPENSSL
    #else
    #if ISMBEDTLS
    static ssl_session ssn;
    memset(&ssn,0,sizeof(mbedtls_ssl_session));
    #else
    memset(&ssn,0,sizeof(ssl_session));
    #endif // ISMBEDTLS
    #endif // OPENSSL
    return 0;
}
#endif // SSLBIO_H_INCLUDED



