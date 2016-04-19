#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef OPENSSL
#define OPENSSL 1
#endif

#ifndef OPENSSLDL
#define OPENSSLDL 1
#endif

#ifndef ISMBEDTLS
#define ISMBEDTLS 1
#endif

#define MAXBUF 9216
#define DEBUG 0

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
inline int echo(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int n = vprintf(format,ap);
    fflush(stdout);
    va_end(ap);
    return n;
}

inline int hex(char *xx,int len){
    printf("buf:");
    for(int i=0;i<len;i++)
    {
        unsigned char c = xx[i]; // must use unsigned char to print >128 value
        if( c< 16)
        {
            printf("0%x", c);
        }
        else
        {
            printf("%x", c);
        }
    }
    printf("\r\n");
    return 0;
}

#endif
