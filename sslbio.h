#ifndef SSLBIO_H_INCLUDED
#define SSLBIO_H_INCLUDED
#include "config.h"
#include<malloc.h>
#include "sendmsg.h"

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

struct ssl_info
{
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    ssl_context ssl;
    ssl_session ssn;
    mbedtls_ssl_config conf;
    x509_crt cacert;
};

#else
#include <polarssl/net.h>
#include <polarssl/debug.h>
#include <polarssl/ssl.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <polarssl/error.h>
#include <polarssl/certs.h>
struct ssl_info
{
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    ssl_context ssl;
    ssl_session ssn;
    x509_crt cacert;
};
#endif // ISMBEDTLS
int ssl_init_info(int *server_fd,ssl_info *sslinfo);
int ssl_free_info(ssl_info *sslinfo);
#endif
#endif // SSLBIO_H_INCLUDED



