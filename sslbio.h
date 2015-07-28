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
int ssl_init_info(int *server_fd,ssl_info *sslinfo);
int ssl_free_info(ssl_info *sslinfo);
#endif
#endif // SSLBIO_H_INCLUDED



