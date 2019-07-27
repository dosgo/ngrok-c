#include "config.h"
#include <string.h>
#include "mytime.h"
#include "opensslbio.h"

#if OPENSSL
static SSL_SESSION *sess = NULL;
int ssl_init_info(int *server_fd,ssl_info *sslinfo)
{
    #if OPENSSLDL
    if(SslMethodV23){
        sslinfo->ctx = (SSL_CTX*)SslCtxNew (SslMethodV23());
    }else if(TlsMethod){
        sslinfo->ctx = (SSL_CTX*)SslCtxNew (TlsMethod());
    }else{
        printf("openssl error\r\n");
    }

    SslCtxCtrl(sslinfo->ctx,SSL_CTRL_SET_SESS_CACHE_MODE,SSL_SESS_CACHE_CLIENT,NULL);
    sslinfo->ssl = SslNew(sslinfo->ctx);
    if (sess != NULL)
    {
        SsLSetSession(sslinfo->ssl, sess);
    }

    SslSetFd(sslinfo->ssl,*server_fd);
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
    SSL_set_fd(sslinfo->ssl,*server_fd);
    SSL_set_connect_state (sslinfo->ssl);
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
#endif

