//
//  base64.h
//  base64
//
//  Created by guofu on 2017/5/25.
//  Copyright © 2017年 guofu. All rights reserved.
//

#ifndef base64_h
#define base64_h

#include <stdio.h>

#if __cplusplus
extern "C"{
#endif

    int base64_encode(const char *indata, int inlen, char *outdata, int *outlen);
    int base64_decode(const char *indata, int inlen, char *outdata, int *outlen);

#if __cplusplus
}
#endif

#endif /* base64_h */
