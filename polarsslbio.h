#ifndef POLARSSLBIO_H_INCLUDED
#define POLARSSLBIO_H_INCLUDED
#include "config.h"
#include<malloc.h>

#if OPENSSL ==0

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
#else

#include <polarssl/net.h>
#include <polarssl/debug.h>
#include <polarssl/ssl.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <polarssl/error.h>
#include <polarssl/certs.h>
#endif

static ssl_session ssn;
struct ssl_info
{
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    ssl_context ssl;
    #if ISMBEDTLS
    mbedtls_ssl_config conf;
    #endif
    x509_crt cacert;
};



#if ISMBEDTLS

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

inline int init_ssl_session(){
    memset(&ssn,0,sizeof(ssl_session));
    return 0;
}
#endif //
#endif // POLARSSLBIO_H_INCLUDED



