#include<stdio.h>
#include<time.h>
#include"util.h"

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
    return sprintf(
        response_str, 
        "%s/%s %d %s\nDate: %s\nServer: %s\nLast-Modified: %s\nContent-Type: %s\nContent-Length: %ld\n", 
        response->protocol, response->version, response->status_code, 
        response->status_verb, response->date, response-> server, 
        response->last_modified, response->content_type, 
        response->content_length
    );
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

// bool create_request_frame(REQUEST* request, char* request_str, int request_str_length) {
//     bool valid = true;

    /** 
     * No matter how many lines or headers you give me 
    */
    // realpath

//     return valid;
// }