#include "https.h"

struct http_response* handle_redirect_get(struct http_response* hresp, char* custom_headers)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = str_replace("Location: ", "", token);
				return http_get(location, custom_headers);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}

/*
	Handles redirect if needed for head requests
*/
struct http_response* handle_redirect_head(struct http_response* hresp, char* custom_headers)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = str_replace("Location: ", "", token);
				return http_head(location, custom_headers);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}

/*
	Handles redirect if needed for post requests
*/
struct http_response* handle_redirect_post(struct http_response* hresp, char* custom_headers, char *post_data)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = str_replace("Location: ", "", token);
				return http_post(location, custom_headers, post_data);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}

/*
	Makes a HTTP request and returns the response
*/
struct http_response* http_req(char *http_headers, struct parsed_url *purl)
{
	/* Parse url */
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}


	/* Declare variable */
	int sock;
	int tmpres;
	char buf[BUFSIZ+1];
	struct sockaddr_in remote;

	/* Allocate memeory for htmlcontent */
	struct http_response *hresp = (struct http_response*)malloc(sizeof(struct http_response));
	if(hresp == NULL)
	{
		printf("Unable to allocate memory for htmlcontent.");
		return NULL;
	}
	hresp->body = NULL;
	hresp->request_headers = NULL;
	hresp->response_headers = NULL;
	hresp->status_code = NULL;
	hresp->status_text = NULL;

	/* Create TCP socket */
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
	    printf("Can't create TCP socket");
	    free(hresp);
		return NULL;
	}

	/* Set remote->sin_addr.s_addr */
	remote.sin_family = AF_INET;
    #if WIN32
    remote.sin_addr.s_addr= inet_addr(purl->ip);
    #else
  	tmpres = inet_pton(AF_INET, purl->ip, (void *)(&(remote.sin_addr.s_addr)));
  	if( tmpres < 0)
  	{
    	printf("Can't set remote->sin_addr.s_addr");
    	free(hresp);
    	return NULL;
  	}
	else if(tmpres == 0)
  	{
		printf("Not a valid IP");
		free(hresp);
    	return NULL;
  	}
    #endif // WIN32
	remote.sin_port = htons(atoi(purl->port));

	/* Connect */
	if(connect(sock, (struct sockaddr *)&remote, sizeof(struct sockaddr)) < 0)
	{
	    printf("Could not connect");
	    free(hresp);
		return NULL;

	}
	ssl_info *sslinfo;
	if ( strcmp(purl->scheme,"https")==0)
	{

           sslinfo=(ssl_info*)malloc(sizeof(ssl_info));
           if(ssl_init_info(&sock,sslinfo)==-1)
           {
               ssl_free_info(sslinfo);
               free(sslinfo);
               free(hresp);
               return NULL;
           }

	}

	/* Send headers to server */
	int sent = 0;
	while(sent < strlen(http_headers))
	{
        if ( strcmp(purl->scheme,"https")==0)
        {
            tmpres = ssl_write(&sslinfo->ssl, (unsigned char*)http_headers+sent, strlen(http_headers)-sent);
        }
        else
        {
            tmpres = send(sock, http_headers+sent, strlen(http_headers)-sent, 0);
        }
		if(tmpres == -1)
		{
			printf("Can't send headers");
			if ( strcmp(purl->scheme,"https")==0)
			{
				ssl_free_info(sslinfo);
				free(sslinfo);
			}
			free(hresp);
			return NULL;
		}
		sent += tmpres;
	 }

	/* Recieve into response*/
	char *response = (char*)malloc(0);
	char BUF[BUFSIZ];
	int recived_len = 0;

	if ( strcmp(purl->scheme,"https")==0)
	{
	   while((recived_len = ssl_read(&sslinfo->ssl, (unsigned char *)BUF, BUFSIZ-1)) > 0)
        {
            BUF[recived_len] = '\0';
            response = (char*)realloc(response, strlen(response) + recived_len + 1);
           // sprintf(response, "%s%s", response, BUF);
            memcpy(response+strlen(response),BUF,recived_len+1);
        }
	}
	else
	{
        while((recived_len = recv(sock, BUF, BUFSIZ-1, 0)) > 0)
        {
            BUF[recived_len] = '\0';
            response = (char*)realloc(response, strlen(response) + recived_len + 1);
           // sprintf(response, "%s%s", response, BUF);
            memcpy(response+strlen(response),BUF,recived_len+1);
        }
	}

    //POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY remote close notify
	if (recived_len < 0&&recived_len!=POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY)
    {
		free(http_headers);
		#ifdef _WIN32
			closesocket(sock);
		#else
			close(sock);
		#endif
        if(strcmp(purl->scheme,"https")==0)
		{
		    ssl_free_info(sslinfo);
		    free(sslinfo);
        }
        free(response);
        free(hresp);
		return NULL;
    }

	/* Reallocate response */
	response = (char*)realloc(response, strlen(response) + 1);


	/* Close socket */
	#ifdef _WIN32
		closesocket(sock);
	#else
		close(sock);
	#endif

	/* Parse status code and text */
	char *status_line = get_until(response, "\r\n");
	status_line = str_replace("HTTP/1.1 ", "", status_line);
	char *status_code = str_ndup(status_line, 4);
	status_code = str_replace(" ", "", status_code);
	char *status_text = str_replace(status_code, "", status_line);
	status_text = str_replace(" ", "", status_text);
	hresp->status_code = status_code;
	hresp->status_code_int = atoi(status_code);
	hresp->status_text = status_text;

	/* Parse response headers */
	char *headers = get_until(response, "\r\n\r\n");
	hresp->response_headers = headers;

	/* Assign request headers */
	hresp->request_headers = http_headers;

	/* Assign request url */
	hresp->request_uri = purl;



    int responselen=strlen(response);
    int bodystart=strpos(response,"\r\n\r\n",0);
	char *body;
    if(bodystart+4>=responselen||bodystart<1)
    {
        body="";
    }
    else
    {
        body=response+bodystart+4;
    }
   //old
    //char *body = strstr(response, "\r\n\r\n");
	//body = str_replace("\r\n\r\n", "", body);
	hresp->body = body;

	/* Return response */
	if (strcmp(purl->scheme,"https")==0)
	{
	    ssl_free_info(sslinfo);
	    free(sslinfo);
    }
	return hresp;
}

/*
	Makes a HTTP GET request to the given url
*/
struct http_response* http_get(char *url, char *custom_headers)
{
	/* Parse url */
	struct parsed_url *purl = parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	char *http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "GET /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "GET /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		char *upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		char *base64 = base64_encode(upwd);

		/* Form header */
		char *auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	/* Add custom headers, and close */
	if(custom_headers != NULL)
	{
		sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
	}
	else
	{
		sprintf(http_headers, "%s\r\n", http_headers);
	}
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	struct http_response *hresp = http_req(http_headers, purl);
    if(hresp!=NULL)
    {
       return handle_redirect_get(hresp, custom_headers);
    }
	/* Handle redirect */
	return NULL;
}

/*
	Makes a HTTP POST request to the given url
*/
struct http_response* http_post(char *url, char *custom_headers, char *post_data)
{
	/* Parse url */
	struct parsed_url *purl = parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	char *http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "POST /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%d\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->query, purl->host, strlen(post_data));
		}
		else
		{
			sprintf(http_headers, "POST /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%d\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->host, strlen(post_data));
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "POST /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%d\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->query, purl->host, strlen(post_data));
		}
		else
		{
			sprintf(http_headers, "POST / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%d\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->host, strlen(post_data));
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		char *upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		char *base64 = base64_encode(upwd);

		/* Form header */
		char *auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	if(custom_headers != NULL)
	{
		sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
		sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
	}
	else
	{
		sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
	}
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	struct http_response *hresp = http_req(http_headers, purl);
  //  printf("post_data:%s\r\n",post_data);
    if(hresp==NULL)
    {
     return NULL;
    }
	/* Handle redirect */
	return handle_redirect_post(hresp, custom_headers, post_data);
}

/*
	Makes a HTTP HEAD request to the given url
*/
struct http_response* http_head(char *url, char *custom_headers)
{
	/* Parse url */
	struct parsed_url *purl = parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	char *http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "HEAD /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "HEAD /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "HEAD/?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "HEAD / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		char *upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		char *base64 = base64_encode(upwd);

		/* Form header */
		char *auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	if(custom_headers != NULL)
	{
		sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
	}
	else
	{
		sprintf(http_headers, "%s\r\n", http_headers);
	}
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	struct http_response *hresp = http_req(http_headers, purl);

	/* Handle redirect */
	return handle_redirect_head(hresp, custom_headers);
}

/*
	Do HTTP OPTIONs requests
*/
struct http_response* http_options(char *url)
{
	/* Parse url */
	struct parsed_url *purl = parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	char *http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "OPTIONS /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "OPTIONS /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "OPTIONS/?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "OPTIONS / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		char *upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		char *base64 = base64_encode(upwd);

		/* Form header */
		char *auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	/* Build headers */
	sprintf(http_headers, "%s\r\n", http_headers);
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	struct http_response *hresp = http_req(http_headers, purl);

	/* Handle redirect */
	return hresp;
}

/*
	Free memory of http_response
*/
void http_response_free(struct http_response *hresp)
{
	if(hresp != NULL)
	{
		if(hresp->request_uri != NULL) parsed_url_free(hresp->request_uri);
		if(hresp->body != NULL) free(hresp->body);
		if(hresp->status_code != NULL) free(hresp->status_code);
		if(hresp->status_text != NULL) free(hresp->status_text);
		if(hresp->request_headers != NULL) free(hresp->request_headers);
		if(hresp->response_headers != NULL) free(hresp->response_headers);
		free(hresp);
	}
}






/*
	Gets the offset of one string in another string
*/
int str_index_of(const char *a, char *b)
{
	char *offset = (char*)strstr(a, b);
	return offset - a;
}

/*
	Checks if one string contains another string
*/
int str_contains(const char *haystack, const char *needle)
{
	char *pos = (char*)strstr(haystack, needle);
	if(pos)
		return 1;
	else
		return 0;
}

/*
	Removes last character from string
*/
char* trim_end(char *string, char to_trim)
{
	char last_char = string[strlen(string) -1];
	if(last_char == to_trim)
	{
		char *new_string = string;
		new_string[strlen(string) - 1] = 0;
		return new_string;
	}
	else
	{
		return string;
	}
}

/*
	Concecates two strings, a wrapper for strcat from string.h, handles the resizing and copying
*/
char* str_cat(char *a, char *b)
{
	char *target = (char*)malloc(strlen(a) + strlen(b) + 1);
	strcpy(target, a);
	strcat(target, b);
	return target;
}

/*
	Converts an integer value to its hex character
*/
char to_hex(char code)
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

/*
	URL encodes a string
*/
char *urlencode(char *str)
{
	char *pstr = str, *buf = (char*)malloc(strlen(str) * 3 + 1), *pbuf = buf;
  	while (*pstr)
	{
    	if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
      		*pbuf++ = *pstr;
    	else if (*pstr == ' ')
      		*pbuf++ = '+';
    	else
      		*pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    	pstr++;
  	}
  	*pbuf = '\0';
  	return buf;
}

/*
	Replacement for the string.h strndup, fixes a bug
*/
char *str_ndup (const char *str, size_t max)
{
    size_t len = strnlen(str, max);
    char *res = (char*)malloc (len + 1);
    if (res)
    {
        memcpy (res, str, len);
        res[len] = '\0';
    }
    return res;
}

/*
	Replacement for the string.h strdup, fixes a bug
*/
char *str_dup(const char *src)
{
   char *tmp = (char*)malloc(strlen(src) + 1);
   if(tmp)
       strcpy(tmp, src);
   return tmp;
}

/*
 	Search and replace a string with another string , in a string
*/
char *str_replace(char *search , char *replace , char *subject)
{
	char  *p = NULL , *old = NULL , *new_subject = NULL ;
	int c = 0 , search_size;
	search_size = strlen(search);
	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search))
	{
		c++;
	}
	c = ( strlen(replace) - search_size )*c + strlen(subject);
	new_subject = (char*)malloc( c );
	strcpy(new_subject , "");
	old = subject;
	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search))
	{
		strncpy(new_subject + strlen(new_subject) , old , p - old);
		strcpy(new_subject + strlen(new_subject) , replace);
		old = p + search_size;
	}
	strcpy(new_subject + strlen(new_subject) , old);
	return new_subject;
}

/*
	Get's all characters until '*until' has been found
*/
char* get_until(char *haystack, char *until)
{
	int offset = str_index_of(haystack, until);
	return str_ndup(haystack, offset);
}


/* decodeblock - decode 4 '6-bit' characters into 3 8-bit binary bytes */
void decodeblock(unsigned char in[], char *clrstr)
{
  	unsigned char out[4];
	out[0] = in[0] << 2 | in[1] >> 4;
	out[1] = in[1] << 4 | in[2] >> 2;
	out[2] = in[2] << 6 | in[3] >> 0;
	out[3] = '\0';
	strncat((char *)clrstr, (char *)out, sizeof(out));
}

/*
	Decodes a Base64 string
*/
char* base64_decode(char *b64src)
{
	char *clrdst = (char*)malloc( ((strlen(b64src) - 1) / 3 ) * 4 + 4 + 50);
	char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int c, phase, i;
	unsigned char in[4];
	char *p;
	clrdst[0] = '\0';
	phase = 0; i=0;
	while(b64src[i])
	{
		c = (int) b64src[i];
		if(c == '=')
		{
			decodeblock(in, clrdst);
			break;
		}
		p = strchr(b64, c);
		if(p)
		{
			in[phase] = p - b64;
			phase = (phase + 1) % 4;
			if(phase == 0)
			{
				decodeblock(in, clrdst);
				in[0]=in[1]=in[2]=in[3]=0;
			}
		}
		i++;
	}
	clrdst = (char*)realloc(clrdst, strlen(clrdst) + 1);
	return clrdst;
}

/* encodeblock - encode 3 8-bit binary bytes as 4 '6-bit' characters */
void encodeblock( unsigned char in[], char b64str[], int len )
{
	char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char out[5];
    out[0] = b64[ in[0] >> 2 ];
    out[1] = b64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? b64[ ((in[1] & 0x0f) << 2) |
             ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? b64[ in[2] & 0x3f ] : '=');
    out[4] = '\0';
    strncat((char *)b64str, (char *)out, sizeof(out));
}

/*
	Encodes a string with Base64
*/
char* base64_encode(char *clrstr)
{
	char *b64dst = (char*)malloc(strlen(clrstr) + 50);
	char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned char in[3];
	int i, len = 0;
	int j = 0;

	b64dst[0] = '\0';
	while(clrstr[j])
	{
		len = 0;
		for(i=0; i<3; i++)
		{
			in[i] = (unsigned char) clrstr[j];
			if(clrstr[j])
			{
				len++; j++;
			}
			else in[i] = 0;
		}
		if( len )
		{
			encodeblock( in, b64dst, len );
		}
	}
	b64dst = (char*)realloc(b64dst, strlen(b64dst) + 1);
	return b64dst;
}



/*
	http-client-c
	Copyright (C) 2012-2013  Swen Kooij

	This file is part of http-client-c.

    http-client-c is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    http-client-c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with http-client-c. If not, see <http://www.gnu.org/licenses/>.

	Warning:
	This library does not tend to work that stable nor does it fully implent the
	standards described by IETF. For more information on the precise implentation of the
	Hyper Text Transfer Protocol:

	http://www.ietf.org/rfc/rfc2616.txt
*/

/*
	Free memory of parsed url
*/
void parsed_url_free(struct parsed_url *purl)
{
    if ( NULL != purl )
	{
        if ( NULL != purl->scheme ) free(purl->scheme);
        if ( NULL != purl->host ) free(purl->host);
        if ( NULL != purl->port ) free(purl->port);
        if ( NULL != purl->path )  free(purl->path);
        if ( NULL != purl->query ) free(purl->query);
        if ( NULL != purl->fragment ) free(purl->fragment);
        if ( NULL != purl->username ) free(purl->username);
        if ( NULL != purl->password ) free(purl->password);
        free(purl);
    }
}

/*
	Retrieves the IP adress of a hostname
*/
char* hostname_to_ip(char *hostname)
{
	char *ip = "0.0.0.0";
	struct hostent *h;
	if ((h=gethostbyname(hostname)) == NULL)
	{
		printf("gethostbyname");
		return NULL;
	}
	return inet_ntoa(*((struct in_addr *)h->h_addr));
}

/*
	Check whether the character is permitted in scheme string
*/
int is_scheme_char(int c)
{
    return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
}

/*
	Parses a specified URL and returns the structure named 'parsed_url'
	Implented according to:
	RFC 1738 - http://www.ietf.org/rfc/rfc1738.txt
	RFC 3986 -  http://www.ietf.org/rfc/rfc3986.txt
*/
struct parsed_url *parse_url(const char *url)
{

	/* Define variable */
    struct parsed_url *purl;
    const char *tmpstr;
    const char *curstr;
    int len;
    int i;
    int userpass_flag;
    int bracket_flag;

    /* Allocate the parsed url storage */
    purl = (struct parsed_url*)malloc(sizeof(struct parsed_url));
    if ( NULL == purl )
	{
        return NULL;
    }
    purl->scheme = NULL;
    purl->host = NULL;
    purl->port = NULL;
    purl->path = NULL;
    purl->query = NULL;
    purl->fragment = NULL;
    purl->username = NULL;
    purl->password = NULL;
    curstr = url;

    /*
     * <scheme>:<scheme-specific-part>
     * <scheme> := [a-z\+\-\.]+
     *             upper case = lower case for resiliency
     */
    /* Read scheme */
    tmpstr = strchr(curstr, ':');
    if ( NULL == tmpstr )
	{
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);

        return NULL;
    }

    /* Get the scheme length */
    len = tmpstr - curstr;

    /* Check restrictions */
    for ( i = 0; i < len; i++ )
	{
        if (is_scheme_char(curstr[i]) == 0)
		{
            /* Invalid format */
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
    }
    /* Copy the scheme to the storage */
    purl->scheme = (char*)malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->scheme )
	{
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);

        return NULL;
    }

    (void)strncpy(purl->scheme, curstr, len);
    purl->scheme[len] = '\0';

    /* Make the character to lower if it is upper case. */
    for ( i = 0; i < len; i++ )
	{
        purl->scheme[i] = tolower(purl->scheme[i]);
    }

    /* Skip ':' */
    tmpstr++;
    curstr = tmpstr;

    /*
     * //<user>:<password>@<host>:<port>/<url-path>
     * Any ":", "@" and "/" must be encoded.
     */
    /* Eat "//" */
    for ( i = 0; i < 2; i++ )
	{
        if ( '/' != *curstr )
		{
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        curstr++;
    }

    /* Check if the user (and password) are specified. */
    userpass_flag = 0;
    tmpstr = curstr;
    while ( '\0' != *tmpstr )
	{
        if ( '@' == *tmpstr )
		{
            /* Username and password are specified */
            userpass_flag = 1;
            break;
        }
		else if ( '/' == *tmpstr )
		{
            /* End of <host>:<port> specification */
            userpass_flag = 0;
            break;
        }
        tmpstr++;
    }

    /* User and password specification */
    tmpstr = curstr;
    if ( userpass_flag )
	{
        /* Read username */
        while ( '\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr )
		{
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->username = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->username )
		{
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->username, curstr, len);
        purl->username[len] = '\0';

        /* Proceed current pointer */
        curstr = tmpstr;
        if ( ':' == *curstr )
		{
            /* Skip ':' */
            curstr++;

            /* Read password */
            tmpstr = curstr;
            while ( '\0' != *tmpstr && '@' != *tmpstr )
			{
                tmpstr++;
            }
            len = tmpstr - curstr;
            purl->password = (char*)malloc(sizeof(char) * (len + 1));
            if ( NULL == purl->password )
			{
                parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
                return NULL;
            }
            (void)strncpy(purl->password, curstr, len);
            purl->password[len] = '\0';
            curstr = tmpstr;
        }
        /* Skip '@' */
        if ( '@' != *curstr )
		{
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        curstr++;
    }

    if ( '[' == *curstr )
	{
        bracket_flag = 1;
    }
	else
	{
        bracket_flag = 0;
    }
    /* Proceed on by delimiters with reading host */
    tmpstr = curstr;
    while ( '\0' != *tmpstr ) {
        if ( bracket_flag && ']' == *tmpstr )
 		{
            /* End of IPv6 address. */
            tmpstr++;
            break;
        }
		else if ( !bracket_flag && (':' == *tmpstr || '/' == *tmpstr) )
		{
            /* Port number is specified. */
            break;
        }
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->host = (char*)malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->host || len <= 0 )
	{
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    (void)strncpy(purl->host, curstr, len);
    purl->host[len] = '\0';
    curstr = tmpstr;

    /* Is port number specified? */
    if ( ':' == *curstr )
	{
        curstr++;
        /* Read port number */
        tmpstr = curstr;
        while ( '\0' != *tmpstr && '/' != *tmpstr )
		{
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->port = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->port )
		{
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->port, curstr, len);
        purl->port[len] = '\0';
        curstr = tmpstr;
    }
	else
	{
	    purl->port =  (char*)malloc(4);
	    memset(purl->port,0,4);
	    if(strcmp(purl->scheme,"https")==0)
	    {
	        (void)strncpy(purl->port, "443", 3);
	    }
	    else
	    {
	        (void)strncpy(purl->port, "80", 3);
	    }
	}

	/* Get ip */
	char *ip = hostname_to_ip(purl->host);
	purl->ip = ip;

	/* Set uri */
	purl->uri = (char*)url;

    /* End of the string */
    if ( '\0' == *curstr )
	{
        return purl;
    }

    /* Skip '/' */
    if ( '/' != *curstr )
	{
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    curstr++;

    /* Parse path */
    tmpstr = curstr;
    while ( '\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr )
	{
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->path = (char*)malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->path )
	{
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    (void)strncpy(purl->path, curstr, len);
    purl->path[len] = '\0';
    curstr = tmpstr;

    /* Is query specified? */
    if ( '?' == *curstr )
	{
        /* Skip '?' */
        curstr++;
        /* Read query */
        tmpstr = curstr;
        while ( '\0' != *tmpstr && '#' != *tmpstr )
		{
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->query = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->query )
		{
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->query, curstr, len);
        purl->query[len] = '\0';
        curstr = tmpstr;
    }

    /* Is fragment specified? */
    if ( '#' == *curstr )
	{
        /* Skip '#' */
        curstr++;
        /* Read fragment */
        tmpstr = curstr;
        while ( '\0' != *tmpstr )
		{
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->fragment = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->fragment )
 		{
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->fragment, curstr, len);
        purl->fragment[len] = '\0';
        curstr = tmpstr;
    }
	return purl;
}







#if WIN32
size_t  strnlen (const char *str, size_t maxlen)
{
  const char *char_ptr, *end_ptr = str + maxlen;
  const unsigned long int *longword_ptr;
  unsigned long int longword, magic_bits, himagic, lomagic;

  if (maxlen == 0)
    return 0;

  if (__builtin_expect (end_ptr < str, 0))
    end_ptr = (const char *) ~0UL;

  /* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = str; ((unsigned long int) char_ptr
            & (sizeof (longword) - 1)) != 0;
       ++char_ptr)
    if (*char_ptr == '/0')
      {
    if (char_ptr > end_ptr)
      char_ptr = end_ptr;
    return char_ptr - str;
      }

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to 8-byte longwords.  */

  longword_ptr = (unsigned long int *) char_ptr;

  /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:

     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
  magic_bits = 0x7efefeffL;
  himagic = 0x80808080L;
  lomagic = 0x01010101L;
  if (sizeof (longword) > 4)
    {
      /* 64-bit version of the magic.  */
      /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
      magic_bits = ((0x7efefefeL << 16) << 16) | 0xfefefeffL;
      himagic = ((himagic << 16) << 16) | himagic;
      lomagic = ((lomagic << 16) << 16) | lomagic;
    }
  if (sizeof (longword) > 8)
    abort ();

  /* Instead of the traditional loop which tests each character,
     we will test a longword at a time.  The tricky part is testing
     if *any of the four* bytes in the longword in question are zero.  */
  while (longword_ptr < (unsigned long int *) end_ptr)
    {
      /* We tentatively exit the loop if adding MAGIC_BITS to
     LONGWORD fails to change any of the hole bits of LONGWORD.

     1) Is this safe?  Will it catch all the zero bytes?
     Suppose there is a byte with all zeros.  Any carry bits
     propagating from its left will fall into the hole at its
     least significant bit and stop.  Since there will be no
     carry from its most significant bit, the LSB of the
     byte to the left will be unchanged, and the zero will be
     detected.

     2) Is this worthwhile?  Will it ignore everything except
     zero bytes?  Suppose every byte of LONGWORD has a bit set
     somewhere.  There will be a carry into bit 8.  If bit 8
     is set, this will carry into bit 16.  If bit 8 is clear,
     one of bits 9-15 must be set, so there will be a carry
     into bit 16.  Similarly, there will be a carry into bit
     24.  If one of bits 24-30 is set, there will be a carry
     into bit 31, so all of the hole bits will be changed.

     The one misfire occurs when bits 24-30 are clear and bit
     31 is set; in this case, the hole at bit 31 is not
     changed.  If we had access to the processor carry flag,
     we could close this loophole by putting the fourth hole
     at bit 32!

     So it ignores everything except 128's, when they're aligned
     properly.  */

      longword = *longword_ptr++;

      if ((longword - lomagic) & himagic)
    {
      /* Which of the bytes was the zero?  If none of them were, it was
         a misfire; continue the search.  */

      const char *cp = (const char *) (longword_ptr - 1);

      char_ptr = cp;
      if (cp[0] == 0)
        break;
      char_ptr = cp + 1;
      if (cp[1] == 0)
        break;
      char_ptr = cp + 2;
      if (cp[2] == 0)
        break;
      char_ptr = cp + 3;
      if (cp[3] == 0)
        break;
      if (sizeof (longword) > 4)
        {
          char_ptr = cp + 4;
          if (cp[4] == 0)
        break;
          char_ptr = cp + 5;
          if (cp[5] == 0)
        break;
          char_ptr = cp + 6;
          if (cp[6] == 0)
        break;
          char_ptr = cp + 7;
          if (cp[7] == 0)
        break;
        }
    }
      char_ptr = end_ptr;
    }

  if (char_ptr > end_ptr)
    char_ptr = end_ptr;
  return char_ptr - str;
}
#endif
int strpos(const char *haystack,const char *needle, int ignorecase = 0)
{
    register unsigned char c, needc;
    unsigned char const *from, *end;
    int len = strlen(haystack);
    int needlen = strlen(needle);
    from = (unsigned char *)haystack;
    end = (unsigned char *)haystack + len;
    const char *findreset = needle;
    for (int i = 0; from < end; ++i) {
        c = *from++;
        needc = *needle;
        if (ignorecase) {
            if (c >= 65 && c < 97)
                c += 32;
            if (needc >= 65 && needc < 97)
                needc += 32;
        }
        if(c == needc) {
            ++needle;
            if(*needle == '\0') {
                if (len == needlen)
                    return 0;
                else
                    return i - needlen+1;
            }
        }
        else {
            if(*needle == '\0' && needlen > 0)
                return i - needlen +1;
            needle = findreset;
        }
    }
    return  -1;
}

