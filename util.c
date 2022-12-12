#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include "util.h"
#include "cgi.h"

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

bool is_valid(char* str, char* regex_comparator) {
    regex_t regex;

    if (regcomp(&regex, regex_comparator, REG_EXTENDED) == 0) {
        return (regexec(&regex, str, 0, NULL, 0) == 0);
    }
    return false;
}

/** TODO: Instead of taking a string, just take the filedescriptor and write to the stream instead :) */
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
        case 413:
            (void)sprintf(status_verb, "Payload Too Large");
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

int find_third_slash(char *path) {
    ssize_t n_path = strlen(path);
    ssize_t i = 0;
    int count = 0;

    for (; i < n_path; i++) {
        if (count == 3) {
            return i;
        }

        if (path[i] == '/') {
            count++;
        }
    }

    return 0;
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
            valid = is_valid(token, HTTP_URL_REGEX) || is_valid(token, FILE_PATH_REGEX);
            puts("Is it valid bruh ???");
            printf("%d\n", valid);
            puts("PATH IS");
            puts(token);

            /** This URL contains a prefix of http://localhost:port_num/ */
            if (is_valid(token, HTTP_URL_REGEX)) {
                int location = find_third_slash(token);
                (void)strncpy(request->path, token + location - 1, strlen(token));
                puts(request->path);
            } else {
                (void)strncpy(request->path, token, strlen(token));

            }
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
        /** Not to confuse with token#4 instead used for If-Modified-Since header. */
        case 3:
            if (is_valid(token, HTTP_DATE_REGEX)) {
                valid = true;
                request->if_modified_str_type = 1;
                (void)strncpy(request->if_modified_since, token, strlen(token));
            } else if (is_valid(token, RFC_850_DATE_REGEX)) {
                valid=true;
                request->if_modified_str_type = 2;
                (void)strncpy(request->if_modified_since, token, strlen(token));
            } else if (is_valid(token, ASCTIME_DATE_REGEX)) {
                valid=true;
                request->if_modified_str_type = 3;
                (void)strncpy(request->if_modified_since, token, strlen(token));
            } else {
                valid=false;
            }
            break;
        default:
            break;
    }
    return valid;
}

void reset_response_object(RESPONSE* response) {
    bzero(response->protocol, sizeof(response->protocol));
    bzero(response->version, sizeof(response->version));
    bzero(response->status_verb, sizeof(response->status_verb));
    bzero(response->date, sizeof(response->date));
    bzero(response->server, sizeof(response->server));
    bzero(response->last_modified, sizeof(response->last_modified));
    bzero(response->content_type, sizeof(response->content_type));
}

void reset_request_object(REQUEST* request) {
    bzero(request->if_modified_since, sizeof(request->if_modified_since));
    bzero(request->verb, sizeof(request->verb));
    bzero(request->path, sizeof(request->path));
    bzero(request->protocol, sizeof(request->protocol));
    bzero(request->version, sizeof(request->version));
    request->if_modified_str_type = 0;
    request->if_modified_t = 0;
    request->time_request_made = 0;
}
