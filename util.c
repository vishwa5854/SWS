#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "util.h"

/** 
 * This is util function which will generate current time like
 * Tue, 15 Nov 1994 08:12:31 GMT
 * This function is inspired by https://s.vishw.site/KeTK7
 */
void get_gmt_date_str(char* time_str, size_t time_length) {
    time_t now = time(0);
    struct tm tm = *gmtime(&now);

    if (strftime(time_str, time_length, "%a, %d %b %Y %H:%M:%S %Z", &tm) <= 0) {
        perror("Error while getting GMT timestamp");
    }
}

int create_response_string(RESPONSE* response, char* response_str) {
    if (response->protocol != NULL) {
        (void)sprintf(response_str, "%s", response->protocol);
    }

    if (response->version != NULL) {
        (void)sprintf(response_str, "%s/%s", response_str, response->version);
    }

    if (response->status_code > 0) {
        (void)sprintf(response_str, "%s %d", response_str, response->status_code);
    }

    if (response->status_verb != NULL) {
        (void)sprintf(response_str, "%s %s\n", response_str, response->status_verb);
    }

    if (response->date != NULL) {
        (void)sprintf(response_str, "%sDate: %s\n", response_str, response->date);
    }

    if (response->server != NULL) {
        (void)sprintf(response_str, "%sServer: %s\n", response_str, response->server);
    }

    if (strlen(response->last_modified) > 0) {
        (void)sprintf(response_str, "%sLast-Modified: %s\n", response_str, response->last_modified);
    }

    if (response->content_type != NULL) {
        (void)sprintf(response_str, "%sContent-Type: %s\n", response_str, response->content_type);
    }

    if (response->content_length > 0) {
        (void)sprintf(response_str, "%sContent-Length: %ld\n", response_str, response->content_length);
    }

    return strlen(response_str);
}

void get_status_verb(int status_code, char* status_verb) {
    switch (status_code) {
        case 200:
            (void)sprintf(status_verb, "OK");
            break;
        case 201:
            (void)sprintf(status_verb, "Created");
            break;
        case 202:
            (void)sprintf(status_verb, "Accepted");
            break;
        case 204:
            (void)sprintf(status_verb, "No Content");
            break;
        case 301:
            (void)sprintf(status_verb, "Moved Permanently");
            break;
        case 302:
            (void)sprintf(status_verb, "Moved Temporarily");
            break;
        case 304:
            (void)sprintf(status_verb, "Not Modified");
            break;
        case 400:
            (void)sprintf(status_verb, "Bad Request");
            break;
        case 401:
            (void)sprintf(status_verb, "Unauthorized");
            break;
        case 403:
            (void)sprintf(status_verb, "Forbidden");
            break;
        case 404:
            (void)sprintf(status_verb, "Not Found");
            break;
        case 500:
            (void)sprintf(status_verb, "Internal Server Error");
            break;
        case 501:
            (void)sprintf(status_verb, "Not Implemented");
            break;
        case 502:
            (void)sprintf(status_verb, "Bad Gateway");
            break;
        case 503:
            (void)sprintf(status_verb, "Service Unavailable");
            break;
        default:
            break;
    }
}

bool create_request_frame(REQUEST* request, char* token, int token_number) {
    bool valid = true;

    switch (token_number) {
        /** GET or HEAD */
        case 0:
            if (
                (strlen(token) == strlen(SUPPORTED_HTTP_VERB_1)) &&
                (strncmp(token, SUPPORTED_HTTP_VERB_1, strlen(SUPPORTED_HTTP_VERB_1)) == 0)
            ) {
                (void)strncpy(request->verb, token, strlen(token));
            } else if (
                (strlen(token) == strlen(SUPPORTED_HTTP_VERB_2)) &&
                (strncmp(token, SUPPORTED_HTTP_VERB_2, strlen(SUPPORTED_HTTP_VERB_2)) == 0)
            ) {
                (void)strncpy(request->verb, token, strlen(token));
            } else {
                valid = false;
            }
            break;
        case 1:
            puts("Inside 1");
            return valid;
            // URI validation
            break;
        case 2:
            /** Protocol and version check bruh. */
            token[strcspn(token, "\r\n")] = '\0';

            if (
                (strlen(token) != strlen(SUPPORTED_PROTOCOL_1)) && 
                (strlen(token) != strlen(SUPPORTED_PROTOCOL_2))
            ) {
                valid = false;
            } else if (
                (strncmp(token, SUPPORTED_PROTOCOL_1, strlen(SUPPORTED_PROTOCOL_1)) != 0) &&
                (strncmp(token, SUPPORTED_PROTOCOL_2, strlen(SUPPORTED_PROTOCOL_2)) != 0)
            ) {
                valid = false;
            } else {
                (void)strncpy(request->protocol, "HTTP", 4);
                (void)strncpy(request->version, "1.0", 3);
            }
            break;
        default:
            break;
    }
    return valid;
}
