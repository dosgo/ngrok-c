#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef OPENSSL
#define OPENSSL 0
#endif

#ifndef ISMBEDTLS
#define ISMBEDTLS 1
#endif


#include <stdarg.h>
#include <stdio.h>
inline int echo(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int n = vprintf(format,ap);
    fflush(stdout);
    va_end(ap);
    return n;
}

#endif
