#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <sys/stat.h>
#include <sys/syslimits.h>
#include <sys/types.h>
#include <time.h>

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

/** 
 * After looking at several HTTP requests I saw a lot of the have cookies in their headers, auth tokens in their headers
 * and most of the times the cookies are really large in size and 8K should be good enough size for one header
 * These are the sample request headers that user might send and we have space for accomodating all of them
 * * GET /home.html HTTP/1.1
 * Host: developer.mozilla.org
 * User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:50.0) Gecko/20100101 Firefox/50.0
 * Accept: text/html,application/xhtml+xml,application/xml;q=0.9,**;q=0.8
 * Accept-Language: en-US,en;q=0.5
 * Accept-Encoding: gzip, deflate, br
 * Referer: https://developer.mozilla.org/testpage.html
 * Connection: keep-alive
 * Upgrade-Insecure-Requests: 1
 * If-Modified-Since: Mon, 18 Jul 2016 02:36:04 GMT
 * If-None-Match: "c561c68d0ba92bbeb8b0fff2a9199f722e3a621a"
 * Cache-Control: max-age=0
*/
#define SUPPORTED_MAX_HEADER_SIZE 8192

/** For a given request we will only support 100 headers as per request, since IRL there are no HTTP requests with more than 100 headers */
#define MAX_NUMBER_OF_HEADERS 100

/** An average human with average typing speed will be able to give atleast basic headers within two minutes */
#define TIMEOUT 120

/** Maximum length of a string representing a valid IPV6 IP or valid IPV4 IP as represented in IPV6 */
#define IPV6_MAXSTRLEN 46

/** Maximum length of a string representing a valid port number.*/
#define PORT_MAXSTRLEN 6

/** Default port is port 8080 if no port is passed via -p*/
#define DEFAULT_PORT "8080"

#define HTTP_DATE_REGEX "(Mon|Tue|Wed|Thu|Fri|Sat|Sun), ([0-3][0-9]) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) ([0-9]{4}) ([01][0-9]|2[0-3])(:[0-5][0-9]){2} GMT"

#define RFC_850_DATE_REGEX "(Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday), ([0-3][0-9])-(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)-([0-9][0-9]) ([01][0-9]|2[0-3])(:[0-5][0-9]){2} GMT"

#define ASCTIME_DATE_REGEX "(Mon|Tue|Wed|Thu|Fri|Sat|Sun) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) (([0-3][0-9])| [1-9]) ([01][0-9]|2[0-3])(:[0-5][0-9]){2} ([0-9][0-9][0-9][0-9])"

#define HTTP_URL_REGEX "^https?:\\/\\/[a-zA-Z0-9.-:]+\\/[a-zA-Z0-9\\/.?&~=_-]+$"

#define FILE_PATH_REGEX "^([a-zA-Z0-9_~?&=.\\()-\\/])+$"

#define CGI 1

#define FILE_SERVING 2

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
    int if_modified_str_type; // type of given if_modified_since string.
    // 1 maps to RFC822, aka RFC 1123 val. 
    // 2 maps to RFC 850, aka RFC 1036 val.
    // 3 maps to asctime val
    // all other values of this int are not recognized!
    time_t if_modified_t; // if_modified_since seconds since epoch
    int time_request_made;
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