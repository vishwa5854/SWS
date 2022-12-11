#include <stdbool.h>
#include "urlparser.h"

int parse(REQUEST* request) {
    (void) request;
    return CGI;
}

/**
 * This function validates the given resource URL
 * A URL could be of two types
 * 1. https://localhost:8080/file_path
 * 2. /file_path
 */
bool is_valid_url(char* url) {
    (void)url;
    return true;
}