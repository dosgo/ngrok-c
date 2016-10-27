#include "config.h"
#include <string.h>
#include "mytime.h"
#include "sslbio.h"
#if OPENSSL
static SSL_SESSION *sess = NULL;
int openssl_init_info(int server_fd,openssl_info *sslinfo)
{
    #if OPENSSLDL
    sslinfo->ctx = (SSL_CTX*)SslCtxNew (SslMethodV23());
    //SSL_CTX_set_session_cache_mode(sslinfo->ctx,SSL_SESS_CACHE_CLIENT);
    SslCtxCtrl(sslinfo->ctx,SSL_CTRL_SET_SESS_CACHE_MODE,SSL_SESS_CACHE_CLIENT,NULL);//=SSL_CTX_set_session_cache_mode
    sslinfo->ssl = SslNew(sslinfo->ctx);
    if (sess != NULL)
    {
        SsLSetSession(sslinfo->ssl, sess);
    }

    SslSetFd(sslinfo->ssl,server_fd);
    SslSetConnectState (sslinfo->ssl);
    int r=0;
    while ((r = SslDoHandshake(sslinfo->ssl)) != 1) {
        int err = SslGetError(sslinfo->ssl, r);
        if (err == SSL_ERROR_WANT_WRITE) {

        } else if (err == SSL_ERROR_WANT_READ) {

        } else {
            return -1;
        }
        //CPU sleep
        sleeps(1);
    }
    sess = SsLGet1Session(sslinfo->ssl);
    #else
    sslinfo->ctx = (SSL_CTX*)SSL_CTX_new (SSLv23_method());
    SSL_CTX_set_session_cache_mode(sslinfo->ctx,SSL_SESS_CACHE_CLIENT);
    sslinfo->ssl = SSL_new(sslinfo->ctx);
    if (sess != NULL)
    {
        SSL_set_session(sslinfo->ssl, sess);
    }
    SSL_set_fd(sslinfo->ssl,server_fd);
    SSL_set_connect_state (sslinfo->ssl);
    //SSL_connect(sslinfo->ssl);
    int r=0;
    //·
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
    sess = SSL_get1_session(sslinfo->ssl);
    #endif // OPENSSLDL
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
        echo( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
       return -1;
    }



    mbedtls_ssl_conf_endpoint( &sslinfo->conf, MBEDTLS_SSL_IS_CLIENT );
    mbedtls_ssl_conf_authmode( &sslinfo->conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
    mbedtls_ssl_conf_ca_chain( &sslinfo->conf, &sslinfo->cacert,NULL);
    mbedtls_ssl_conf_rng( &sslinfo->conf, mbedtls_ctr_drbg_random, &sslinfo->ctr_drbg );

    if( ( ret = mbedtls_ssl_setup( &sslinfo->ssl, &sslinfo->conf ) ) != 0 )
    {
        echo( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
        return -1;
    }

    mbedtls_ssl_set_bio( &sslinfo->ssl, server_fd,mbedtls_net_send, mbedtls_net_recv,NULL );
    mbedtls_ssl_set_session(&sslinfo->ssl, &ssn);
    while((ret = mbedtls_ssl_handshake(&sslinfo->ssl))!=0)
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            echo( " failed\n  ! ssl_handshake returned -0x%x\n\n", -ret );
            return -1;
        }
        //CPU sleep
        sleeps(1);
    }



    if((ret = mbedtls_ssl_get_verify_result( &sslinfo->ssl ) ) != 0 )
    {
       // echo( "Verifying peer X.509 certificate...failed \r\n" );
    }
    else
    {
        echo( " ok\n" );
    }
    //保存session
    if( ( ret = mbedtls_ssl_get_session( &sslinfo->ssl, &ssn ) ) != 0 )
    {
        //失败初始化
        memset(&ssn, 0, sizeof(mbedtls_ssl_session));
    }
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
        echo( " failed\n  ! ssl_init returned %d\n\n", ret );
        return -1;
    }

    ssl_set_endpoint( &sslinfo->ssl, SSL_IS_CLIENT );
    ssl_set_authmode( &sslinfo->ssl, SSL_VERIFY_OPTIONAL );
    ssl_set_ca_chain( &sslinfo->ssl, &sslinfo->cacert, NULL, "" );
    ssl_set_rng( &sslinfo->ssl, ctr_drbg_random, &sslinfo->ctr_drbg );
    ssl_set_bio( &sslinfo->ssl, net_recv, server_fd,net_send, server_fd );
    ssl_set_session(&sslinfo->ssl, &ssn);



    while((ret = ssl_handshake(&sslinfo->ssl))!=0)
    {
        if( ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE )
        {
            echo( " failed\n  ! ssl_handshake returned -0x%x\n\n", -ret );
            return -1;
        }
        //CPU sleep
        sleeps(1);
    }

    if((ret = ssl_get_verify_result( &sslinfo->ssl ) ) != 0 )
    {
       // echo( "Verifying peer X.509 certificate...failed \r\n" );
    }
    else
    {
        echo( " ok\n" );
    }
    //保存session加快握手速度
    if( ( ret = ssl_get_session( &sslinfo->ssl, &ssn ) ) != 0 )
    {
        //失败初始化
        memset(&ssn, 0, sizeof(ssl_session));
    }
    return 0;
}



#endif // ISMBEDTLS
#endif // OPENSSL
