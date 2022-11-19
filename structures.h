#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include<sys/syslimits.h>

/** GET or HEAD + '\0' */
#define HTTP_VERB_MAX_LEN 5

/** HTTP + '\0' */
#define PROTOCOL_MAX_LEN 5

/** 1.0 + '\0' */
#define PROTOCOL_VERSION_MAX_LEN 4

/** 
 * Status-Code  = "200"   ; OK
 *              | "201"   ; Created
 *              | "202"   ; Accepted
 *              | "204"   ; No Content
 *              | "301"   ; Moved Permanently
 *              | "302"   ; Moved Temporarily
 *              | "304"   ; Not Modified
 *              | "400"   ; Bad Request
 *              | "401"   ; Unauthorized
 *              | "403"   ; Forbidden
 *              | "404"   ; Not Found
 *              | "500"   ; Internal Server Error
 *              | "501"   ; Not Implemented
 *              | "502"   ; Bad Gateway
 *              | "503"   ; Service Unavailable
 *              | extension-code
*/
#define STATUS_VERB_MAX_LEN 22

/** Tue, 15 Nov 1994 08:12:31 GMT */
#define DATE_MAX_LEN 30

/** Just an arbitary value :) */
#define SERVER_MAX_LEN 100

/** We will only send text/html but we will do the optional type if we have time :) */
#define CONTENT_TYPE_MAX_LEN 10

/** 
 * Sample HTTP Request looks like this
 * GET '/file' HTTP/1.0
 * If-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT
*/
typedef struct HTTP_REQUEST {
    char verb[HTTP_VERB_MAX_LEN];
    char path[PATH_MAX];
    char protocol[PROTOCOL_MAX_LEN];
    char version[PROTOCOL_VERSION_MAX_LEN];
} REQUEST;

/**
 * Sample HTTP Response looks like this
 * HTTP/1.1 200 OK
 * Date: Tue, 15 Nov 1994 08:12:31 GMT
 * Server: gws
 * Last-Modified: Mon, 14 Nov 2022 00:18:42 GMT
 * Content-Type: text/html
 * Content-Length: 123 
*/
typedef struct HTTP_RESPONSE {
    char  protocol[PROTOCOL_MAX_LEN];
    char  version[PROTOCOL_VERSION_MAX_LEN];
    int   status_code;
    char  status_verb[STATUS_VERB_MAX_LEN];
    char  date[DATE_MAX_LEN];
    char  server[SERVER_MAX_LEN];
    char  last_modified[DATE_MAX_LEN];
    char  content_type[CONTENT_TYPE_MAX_LEN];
    off_t content_length;
} RESPONSE;

#endif /* !STRUCTURES_H_ */