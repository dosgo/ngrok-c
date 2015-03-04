#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32

#include <windows.h>
#elif _LINUX
	#include <sys/socket.h>
#elif __FreeBSD__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#else
	#error Platform not suppoted.
#endif

#include <errno.h>




 #include<cstring>

#ifdef __cplusplus
	#include <locale>
#endif
#include "sslbio.h"

#if WIN32
size_t  strnlen (const char *str, size_t maxlen);
#endif
/*str*/
int str_contains(const char *haystack, const char *needle);
char *str_replace(char *search , char *replace , char *subject);
char* get_until(char *haystack, char *until);
char *str_ndup (const char *str, size_t max);
char* base64_encode(char *clrstr);
int str_index_of(const char *a, char *b);
int strpos(const char *haystack,const char *needle, int ignorecase);

/*url*/
struct parsed_url *parse_url(const char *url);
void parsed_url_free(struct parsed_url *purl);

/*
	Prototype functions
*/
struct http_response* http_req(char *http_headers, struct parsed_url *purl);
struct http_response* http_get(char *url, char *custom_headers);
struct http_response* http_head(char *url, char *custom_headers);
struct http_response* http_post(char *url, char *custom_headers, char *post_data);


/*
	Represents an HTTP html response
*/
struct http_response
{
	struct parsed_url *request_uri;
	char *body;
	char *status_code;
	int status_code_int;
	char *status_text;
	char *request_headers;
	char *response_headers;
};


struct parsed_url
{
	char *uri;					/* mandatory */
    char *scheme;               /* mandatory */
    char *host;                 /* mandatory */
	char *ip; 					/* mandatory */
    char *port;                 /* optional */
    char *path;                 /* optional */
    char *query;                /* optional */
    char *fragment;             /* optional */
    char *username;             /* optional */
    char *password;             /* optional */
};
