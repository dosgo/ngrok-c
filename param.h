#ifndef __PARAM_H__
#define __PARAM_H__
#include "config.h"
#include "nonblocking.h"
#include <string>
#include <map>
#include "sslbio.h"

#if WIN32
#include <windows.h>
#else
#include <errno.h>
#include <netinet/in.h>
//typedef unsigned long long __int64;
#include <string.h>
#ifndef __UCLIBC__
#include <sys/select.h>
#endif
#endif
#include "bytestool.h"
using namespace std;

int loadargs(int argc, char **argv);

inline int strpos(char *str, char c)
{
    char *sc = strchr(str, c);
    if (sc == NULL)
        return (-1);
    return (sc - str);
}

inline int getvalue(char *str, const char *key, char *value)
{
    int ypos = 0;
    if (strncmp(str, key, strlen(key)) == 0)
    {
        ypos = strpos(str, ':');
        if (ypos != -1)
        {
            memcpy(value, str + ypos + 1, strlen(str + ypos));
            return 0;
        }
    }
    return -1;
}


#endif