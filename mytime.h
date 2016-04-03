#ifndef __MYTIME_H__
#define __MYTIME_H__

#if WIN32
#include <windows.h>
#else
#include <errno.h>
#include <sys/select.h>
#include <netinet/in.h>
inline void milliseconds_sleep( unsigned long mSec )
{
    struct timeval tv;
    tv.tv_sec   = mSec / 1000;
    tv.tv_usec  = (mSec % 1000) * 1000;
    int err;
    do
    {
        err = select( 0, NULL, NULL, NULL, &tv );
    }
    while ( err < 0 && errno == EINTR );
}
#endif

inline void sleeps(int ti)
{
    #if WIN32
        Sleep( ti);
    #else
        milliseconds_sleep( ti);
    #endif
}

#endif
