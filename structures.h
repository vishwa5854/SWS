#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include<sys/syslimits.h>

/** GET or HEAD + '\0' */
#define HTTP_VERB_MAX_LEN 5

/** HTTP + '\0' */
#define PROTOCOL_MAX_LEN 5

/** 1.0 + '\0' */
#define PROTOCOL_VERSION_MAX_LEN 4

#define SERVER "z"

#define CONTENT_TYPE_DEFAULT "text/html"

#define SUPPORTED_PROTOCOL_1 "HTTP/1.0"

#define SUPPORTED_PROTOCOL_2 "HTTP/0.9"

#define SUPPORTED_HTTP_VERB_1 "GET"

#define SUPPORTED_HTTP_VERB_2 "HEAD"

#define SUPPORTED_PROTOCOL_ONLY "HTTP"

#define SUPPORTED_VERSION_ONLY "1.0"

#define SUPPORTED_HEADER "If-Modified-Since:"

/** TODO: Update the regex for weekday as well */
#define HTTP_DATE_REGEX "(Mon|Tue|Wed|Thu|Fri|Sat|Sun), ([0-3][0-9]) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) ([0-9]{4}) ([01][0-9]|2[0-3])(:[0-5][0-9]){2} GMT"

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
    char if_modified_since[DATE_MAX_LEN];
} REQUEST;

/**
 * Sample HTTP Response looks like this
 * HTTP/1.0 200 OK
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