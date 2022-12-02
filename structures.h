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

/** 
 * After looking at several HTTP requests I saw a lot of the have cookies in their headers, auth tokens in their headers
 * and most of the times the cookies are really large in size and 16K should be good enough size for the entire payload
 * if you consider 1K for each header, we can take 16 different headers or atleast 15 headers which are more than enoguh
 * for us.
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

/** For a given request we will only support 100 headers as per request this could be the maximum number of headers passed */
#define MAX_NUMBER_OF_HEADERS 100

/** An average human with average typing speed will be able to give atleast basic headers within two minutes */
#define TIMEOUT 120

/** TODO: Update the regex for two other formats mentioned in the RFC */
#define HTTP_DATE_REGEX "(Mon|Tue|Wed|Thu|Fri|Sat|Sun), ([0-3][0-9]) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) ([0-9]{4}) ([01][0-9]|2[0-3])(:[0-5][0-9]){2} GMT"

#define HTTP_URL_REGEX "[http|https]://([a-zA-Z]+(\\.[a-zA-Z]+)+)/[A-Za-z0-9]+"

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