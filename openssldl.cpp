#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "openssldl.h"
#ifdef __WIN32__
#include <windows.h>
#define HANDLE_DLL HMODULE
#define GETPROC GetProcAddress
#else
#include <dlfcn.h>
#define HANDLE_DLL void*
#define GETPROC dlsym
#endif

//----------------------------------------------------------------------------
static HANDLE_DLL ssl_handle1 = 0; // Aponta para DLL aberta - SSL
static HANDLE_DLL ssl_handle2 = 0; // Aponta para DLL aberta - util


//----------------------------------------------------------------------------
TSslGetError         SslGetError = 0;
TSslLibraryInit      SslLibraryInit = 0;
TSslLoadErrorStrings SslLoadErrorStrings = 0;
TSslCtxNew           SslCtxNew = 0;
TSslCtxFree          SslCtxFree = 0;
TSslSetFd            SslSetFd = 0;
TSslMethodV2         SslMethodV2 = 0;
TSslMethodV3         SslMethodV3 = 0;
TSslMethodTLSV1      SslMethodTLSV1 = 0;
TSslMethodV23        SslMethodV23 = 0;
TSslNew              SslNew = 0;
TSslFree             SslFree = 0;
TSslAccept           SslAccept = 0;
TSslConnect          SslConnect = 0;
TSslShutdown         SslShutdown = 0;
TSslRead             SslRead = 0;
TSslPeek             SslPeek = 0;
TSslWrite            SslWrite = 0;
TSslPending          SslPending = 0;
TSslPrivateKeyFile   SslPrivateKeyFile = 0;
TSslCertificateFile  SslCertificateFile = 0;
TOPENSSLaddallalgorithms OPENSSLaddallalgorithms = 0;
TSslGetPeerCertificate SslGetPeerCertificate = 0;
TSslX509free         SslX509free = 0;
TSslX509d2i          SslX509d2i = 0;
TSslX509i2d          SslX509i2d = 0;
TSslSetConnectState  SslSetConnectState = 0;//dosgo
TSslDoHandshake  SslDoHandshake = 0;//dosgo
TSsLGetSession   SsLGetSession = 0;//dosgo
TSsLGet1Session   SsLGet1Session = 0;//dosgo
TSsLSetSession   SsLSetSession = 0;//dosgo
TSslCtxCtrl    SslCtxCtrl = 0;//dosgo


//----------------------------------------------------------------------------
void FechaSSL()
{
    if (ssl_handle1 == 0)
        return;


#ifdef __WIN32__
    FreeLibrary(ssl_handle1);
    FreeLibrary(ssl_handle2);
#else
    dlclose(ssl_handle1);
    dlclose(ssl_handle2);
#endif
    ssl_handle1 = ssl_handle2 = 0;
}

//----------------------------------------------------------------------------
const char * AbreSSL()
{
    if (ssl_handle1)
        return 0;
#ifdef __WIN32__
    ssl_handle1 = LoadLibrary("ssleay32.dll");
    if (ssl_handle1 == 0)
        ssl_handle1 = LoadLibrary("libssl32.dll");
    if (ssl_handle1 == 0)
        return "Erro ao carregar ssleay32.dll ou libssl32.dll";
    ssl_handle2 = LoadLibrary("libeay32.dll");
    if (ssl_handle2 == 0)
    {
        FreeLibrary(ssl_handle1);
        ssl_handle1 = 0;
        return "Erro ao carregar libeay32.dll";
    }
#else
    ssl_handle1 = dlopen("libssl.so", RTLD_LAZY);
    if (ssl_handle1 == 0)
        ssl_handle1 = dlopen("libssl.so.1.0.0", RTLD_LAZY);
    if (ssl_handle1 == 0)
        return "Erro ao carregar libssl.so";
    ssl_handle2 = dlopen("libcrypto.so", RTLD_LAZY);
    if (ssl_handle2 == 0)
    ssl_handle2 = dlopen("libcrypto.so.1.0.0", RTLD_LAZY);
    if (ssl_handle2 == 0)
    {
        dlclose(ssl_handle1);
        ssl_handle1 = 0;
        return "Erro ao carregar libcrypto.so";
    }
#endif

    SslGetError         = (TSslGetError)   GETPROC(ssl_handle1, "SSL_get_error");
    SslLibraryInit      = (TSslLibraryInit)GETPROC(ssl_handle1, "SSL_library_init");
    SslLoadErrorStrings = (TSslLoadErrorStrings)GETPROC(ssl_handle1, "SSL_load_error_strings");
    SslCtxNew           = (TSslCtxNew)     GETPROC(ssl_handle1, "SSL_CTX_new");
    SslCtxFree          = (TSslCtxFree)    GETPROC(ssl_handle1, "SSL_CTX_free");
    SslSetFd            = (TSslSetFd)      GETPROC(ssl_handle1, "SSL_set_fd");
    SslMethodV2         = (TSslMethodV2)   GETPROC(ssl_handle1, "SSLv2_method");
    SslMethodV3         = (TSslMethodV3)   GETPROC(ssl_handle1, "SSLv3_method");
    SslMethodTLSV1      = (TSslMethodTLSV1)GETPROC(ssl_handle1, "TLSv1_method");
    SslMethodV23        = (TSslMethodV23)  GETPROC(ssl_handle1, "SSLv23_method");
    SslNew              = (TSslNew)        GETPROC(ssl_handle1, "SSL_new");
    SslFree             = (TSslFree)       GETPROC(ssl_handle1, "SSL_free");
    SslAccept           = (TSslAccept)     GETPROC(ssl_handle1, "SSL_accept");
    SslConnect          = (TSslConnect)    GETPROC(ssl_handle1, "SSL_connect");
    SslShutdown         = (TSslShutdown)   GETPROC(ssl_handle1, "SSL_shutdown");
    SslRead             = (TSslRead)       GETPROC(ssl_handle1, "SSL_read");
    SslPeek             = (TSslPeek)       GETPROC(ssl_handle1, "SSL_peek");
    SslWrite            = (TSslWrite)      GETPROC(ssl_handle1, "SSL_write");
    SslPending          = (TSslPending)    GETPROC(ssl_handle1, "SSL_pending");
    SslPrivateKeyFile   = (TSslPrivateKeyFile)GETPROC(ssl_handle1, "SSL_CTX_use_PrivateKey_file");
    SslCertificateFile  = (TSslCertificateFile)GETPROC(ssl_handle1, "SSL_CTX_use_certificate_file");
    OPENSSLaddallalgorithms = (TOPENSSLaddallalgorithms)GETPROC(ssl_handle2, "OPENSSL_add_all_algorithms_noconf");
    SslGetPeerCertificate = (TSslGetPeerCertificate)GETPROC(ssl_handle1, "SSL_get_peer_certificate");
    SslX509free         = (TSslX509free)   GETPROC(ssl_handle2, "X509_free");
    SslX509d2i          = (TSslX509d2i)    GETPROC(ssl_handle2, "d2i_X509");
    SslX509i2d          = (TSslX509i2d)    GETPROC(ssl_handle2, "i2d_X509");
    SslSetConnectState  = (TSslSetConnectState)   GETPROC(ssl_handle1, "SSL_set_connect_state");//dosgo
    SslDoHandshake      = (TSslDoHandshake)   GETPROC(ssl_handle1, "SSL_do_handshake");//dosgo
    SsLGetSession      = (TSsLGetSession)   GETPROC(ssl_handle1, "SSL_get_session");//dosgo
    SsLGet1Session      = (TSsLGet1Session)   GETPROC(ssl_handle1, "SSL_get1_session");//dosgo
    SsLSetSession      = (TSsLSetSession)   GETPROC(ssl_handle1, "SSL_set_session");//dosgo
    SslCtxCtrl      = (TSslCtxCtrl)   GETPROC(ssl_handle1, "SSL_CTX_ctrl");//dosgo

    const char * erro = 0;
    if (!SslGetError)         erro = "SSL_get_error";
    if (!SslLibraryInit)      erro = "SSL_library_init";
    if (!SslLoadErrorStrings) erro = "SSL_load_error_strings";
    if (!SslCtxNew)           erro = "SSL_CTX_new";
    if (!SslCtxFree)          erro = "SSL_CTX_free";
    if (!SslSetFd)            erro = "SSL_set_fd";
    if (!SslMethodV2 && !SslMethodV23 && !SslMethodV3)
        erro = "SSLv2_method/SSLv3_method";
    if (!SslMethodTLSV1)      erro = "TLSv1_method";
    if (!SslNew)              erro = "SSL_new";
    if (!SslFree)             erro = "SSL_free";
    if (!SslAccept)           erro = "SSL_accept";
    if (!SslConnect)          erro = "SSL_connect";
    if (!SslShutdown)         erro = "SSL_shutdown";
    if (!SslRead)             erro = "SSL_read";
    if (!SslPeek)             erro = "SSL_peek";
    if (!SslWrite)            erro = "SSL_write";
    if (!SslPending)          erro = "SSL_pending";
    if (!SslPrivateKeyFile)   erro = "SSL_CTX_use_PrivateKey_file";
    if (!SslCertificateFile)  erro = "SSL_CTX_use_certificate_file";
    if (!OPENSSLaddallalgorithms) erro = "OPENSSL_add_all_algorithms";
    if (!SslGetPeerCertificate) erro = "SSL_get_peer_certificate";
    if (!SslX509free)         erro = "X509_free";
    if (!SslX509d2i)          erro = "d2i_X509";
    if (!SslX509i2d)          erro = "i2d_X509";
    if (!SslSetConnectState)  erro = "SSL_set_connect_state";//dosgo
    if (!SslDoHandshake)     erro = "SSL_do_handshake";//dosgo
    if (!SsLGetSession)     erro = "SSL_get_session";//dosgo
    if (!SsLSetSession)     erro = "SSL_set_session";//dosgo
    if (!SslCtxCtrl)     erro = "SSL_CTX_ctrl";//dosgo
    if (!SsLGet1Session)     erro = "SSL_get1_session";//dosgo



    if (erro)
    {
        FechaSSL();
        return erro;
    }

    //if (SslLibraryInit())
    //{
    //    FechaSSL();
    //    return "Erro ao inicializar OpenSSL (SSL_library_init)";
    //}
    SslLibraryInit();
    SslLoadErrorStrings();
    OPENSSLaddallalgorithms();
    //RAND_screen();
    return 0;
}



//----------------------------------------------------------------------------


