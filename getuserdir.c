/* Length of the string /sws + 1 for safety */
#define SWS_LEN 5

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "handler.h"
#include "readdirs.h"
#include "structures.h"

void getuserdir(char* userstr, int fd, bool is_valid_request,
                RESPONSE* response, char* response_string) {
    int userstrlen = strlen(userstr);
    /* Initializing as NULL because if not, they are not considered NULL but have some garbage value */
    char* username = NULL;
    char* requestedContent = NULL;
    int i;

    for (i = 0; i < userstrlen; i++) {
        if (userstr[i] == '/') {
            if ((username = malloc((i + 1) * sizeof(char*))) == NULL) {
                send_error(500, fd, is_valid_request, response,
                           response_string);
                return;
            }

            if ((requestedContent =
                     malloc((userstrlen - i - 1) * sizeof(char*))) == NULL) {
                send_error(500, fd, is_valid_request, response,
                           response_string);
                return;
            }
            (void)strncpy(username, userstr, i);
            (void)strncpy(requestedContent, userstr + i + 1,
                          userstrlen - i - 1);
            break;
        }
    }
    /* Doing this for the case when the request only contains a username without / */
    if (username == NULL && userstrlen != 0) {
	    if ((username = malloc(userstrlen * sizeof(char*))) == NULL) {
		    send_error(500, fd, is_valid_request, response, response_string);
		    return;
	    }
	    (void)strncpy(username, userstr, userstrlen);
    }

    struct passwd* p;

    if ((p = getpwnam(username)) == NULL) {
        send_error(404, fd, is_valid_request, response, response_string);
        return;
    }
    int swsdirlen = strlen(p->pw_dir) + SWS_LEN + userstrlen - i - 1;
    char* swsdir;
    int userdirlen = strlen(p->pw_dir);
    
	if ((swsdir = malloc(swsdirlen * sizeof(char*))) == NULL) {
        send_error(500, fd, is_valid_request, response, response_string);
        return;
    }
    (void)strncpy(swsdir, p->pw_dir, userdirlen);
    (void)strncat(swsdir, "/sws/", SWS_LEN);
    if (requestedContent != NULL) {
	    (void)strncat(swsdir, requestedContent, userstrlen - i - 1);
    }
    char* resolvedpath;
    
	if ((resolvedpath = malloc(PATH_MAX * sizeof(char*))) == NULL) {
        send_error(500, fd, is_valid_request, response, response_string);
        return;
    }
    
	if ((realpath(swsdir, resolvedpath)) == NULL) {
        send_error(500, fd, is_valid_request, response, response_string);
        return;
    }
    
	if (strncmp(p->pw_dir, resolvedpath, userdirlen) != 0) {
        send_error(401, fd, is_valid_request, response, response_string);
        return;
    }
    readdirs(swsdir, fd, is_valid_request, response, response_string, false);
}
