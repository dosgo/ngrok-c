#include "config.h"
#include <string.h>
#include "mytime.h"
#include "sslbio.h"
#if OPENSSL
int openssl_init_info(int server_fd,openssl_info *sslinfo)
{
    sslinfo->ctx = (SSL_CTX*)SSL_CTX_new (SSLv3_method());
    sslinfo->ssl = SSL_new(sslinfo->ctx);
    SSL_set_fd(sslinfo->ssl,server_fd);
    //·Ç×èÈû
    SSL_set_connect_state (sslinfo->ssl);
    //SSL_connect(sslinfo->ssl);
    int r=0;
    //·Ç×èÈûÎÕÊÖ
    while ((r = SSL_do_handshake(sslinfo->ssl)) != 1) {
        int err = SSL_get_error(sslinfo->ssl, r);
        if (err == SSL_ERROR_WANT_WRITE) {

        } else if (err == SSL_ERROR_WANT_READ) {

        } else {
            return -1;
        }
        //CPU sleep
        sleeps(1);
    }
    return 0;
}



int openssl_free_info(openssl_info *sslinfo)
{
    SSL_shutdown( sslinfo->ssl );
    SSL_free( sslinfo->ssl );
    SSL_CTX_free( sslinfo->ctx  );
   //CRYPTO_cleanup_all_ex_data();
    //ERR_remove_state();
    return 0;
}
#else
#if ISMBEDTLS
int ssl_init_info(int *server_fd,ssl_info *sslinfo)
{
    int ret;
    const char *pers = "ssl";
    mbedtls_x509_crt_init(&sslinfo->cacert );
    mbedtls_ctr_drbg_init(&sslinfo->ctr_drbg);
    mbedtls_entropy_init(&sslinfo->entropy );

    if( mbedtls_ctr_drbg_seed( &sslinfo->ctr_drbg, mbedtls_entropy_func, &sslinfo->entropy,
                       (const unsigned char *) pers, strlen( pers ) ) != 0 )
    {
        return  -1;

    }

    mbedtls_ssl_init( &sslinfo->ssl );
    mbedtls_ssl_config_init( &sslinfo->conf );
    if( ( ret = mbedtls_ssl_config_defaults( &sslinfo->conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
       return -1;
    }



    mbedtls_ssl_conf_endpoint( &sslinfo->conf, MBEDTLS_SSL_IS_CLIENT );
    mbedtls_ssl_conf_authmode( &sslinfo->conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
    mbedtls_ssl_conf_ca_chain( &sslinfo->conf, &sslinfo->cacert,NULL);
    mbedtls_ssl_conf_rng( &sslinfo->conf, mbedtls_ctr_drbg_random, &sslinfo->ctr_drbg );

    if( ( ret = mbedtls_ssl_setup( &sslinfo->ssl, &sslinfo->conf ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
        return -1;
    }

    mbedtls_ssl_set_bio( &sslinfo->ssl, server_fd,mbedtls_net_send, mbedtls_net_recv,NULL );

    while((ret = mbedtls_ssl_handshake(&sslinfo->ssl))!=0)
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            printf( " failed\n  ! ssl_handshake returned -0x%x\n\n", -ret );
            return -1;
        }
        //CPU sleep
        sleeps(1);
    }



    if((ret = mbedtls_ssl_get_verify_result( &sslinfo->ssl ) ) != 0 )
    {
       // printf( "Verifying peer X.509 certificate...failed \r\n" );
    }
    else
    {
        printf( " ok\n" );
    }
    return 0;
}



int ssl_free_info(ssl_info *sslinfo){
    mbedtls_x509_crt_free(&sslinfo->cacert );
    mbedtls_ssl_free(&sslinfo->ssl);
    mbedtls_ssl_config_free( &sslinfo->conf );
    mbedtls_ctr_drbg_free(&sslinfo->ctr_drbg );
    mbedtls_entropy_free(&sslinfo->entropy );
    return 0;
}

#else
int ssl_init_info(int *server_fd,ssl_info *sslinfo)
{
    int ret;
    const char *pers = "ssl";
    x509_crt_init(&sslinfo->cacert );
    entropy_init(&sslinfo->entropy );
    if( ( ret = ctr_drbg_init( &sslinfo->ctr_drbg, entropy_func, &sslinfo->entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {

        return -1;
    }
    if( ( ret = ssl_init( &sslinfo->ssl ) ) != 0 )
    {
        printf( " failed\n  ! ssl_init returned %d\n\n", ret );
        return -1;
    }

    ssl_set_endpoint( &sslinfo->ssl, SSL_IS_CLIENT );
    ssl_set_authmode( &sslinfo->ssl, SSL_VERIFY_OPTIONAL );
    ssl_set_ca_chain( &sslinfo->ssl, &sslinfo->cacert, NULL, "" );
    ssl_set_rng( &sslinfo->ssl, ctr_drbg_random, &sslinfo->ctr_drbg );
    ssl_set_bio( &sslinfo->ssl, net_recv, server_fd,net_send, server_fd );

    while((ret = ssl_handshake(&sslinfo->ssl))!=0)
    {
        if( ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE )
        {
            printf( " failed\n  ! ssl_handshake returned -0x%x\n\n", -ret );
            return -1;
        }
        //CPU sleep
        sleeps(1);
    }



    if((ret = ssl_get_verify_result( &sslinfo->ssl ) ) != 0 )
    {
       // printf( "Verifying peer X.509 certificate...failed \r\n" );
    }
    else
    {
        printf( " ok\n" );
    }
    return 0;
}



int ssl_free_info(ssl_info *sslinfo){
    /*
    if(&sslinfo->cacert!=NULL)
    {
        x509_crt_free(&sslinfo->cacert);
    }
    if(&sslinfo->ssl!=NULL)
    {
        ssl_free(&sslinfo->ssl);
    }
    if(&sslinfo->ctr_drbg!=NULL)
    {
        ctr_drbg_free(&sslinfo->ctr_drbg);
    }
    if(&sslinfo->entropy!=NULL)
    {
        entropy_free(&sslinfo->entropy);
    }
    */
    x509_crt_free(&sslinfo->cacert);
    ssl_free(&sslinfo->ssl);
    ctr_drbg_free(&sslinfo->ctr_drbg);
    entropy_free(&sslinfo->entropy);
    return 0;
}
#endif // ISMBEDTLS
#endif // OPENSSL
