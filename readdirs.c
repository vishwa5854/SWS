#define INDEX_SIZE 12

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#include "handler.h"
#include "structures.h"

void readdirs(char* dirname, char* workingdir, int fd, bool is_valid_request, RESPONSE* response,
              char* response_string) {
    char path[PATH_MAX];
    char realworkingdir[PATH_MAX];

    if ((realpath(workingdir, realworkingdir)) == NULL) {
	    send_error(500, fd, is_valid_request, response, response_string);
	    return;
    }

    int realworkingdirlen = strlen(realworkingdir);

    if (realworkingdir[realworkingdirlen - 1] != '/') {
        realworkingdirlen++;
	    if (strncat(realworkingdir, "/", strlen("/")) == NULL) {
		    send_error(500, fd, is_valid_request, response, response_string);
		    return;
	    }
    }

    if (strncmp(dirname, realworkingdir, realworkingdirlen - 1) == 0) {
	    if (strncpy(path, dirname, strlen(dirname)) == NULL) {
		    send_error(500, fd, is_valid_request, response, response_string);
		    return;
	    }
    }
    else {
        if (strncpy(path, realworkingdir, realworkingdirlen) == NULL) {
            send_error(500, fd, is_valid_request, response, response_string);
            return;
        }
        if (strncat(path, dirname, strlen(dirname)) == NULL) {
            send_error(500, fd, is_valid_request, response, response_string);
            return;
        }
    }

    char finalpath[PATH_MAX];

    if (realpath(path, finalpath) == NULL) {
        send_error(404, fd, is_valid_request, response, response_string);
        return;
    }


    if (strncmp(finalpath, realworkingdir, realworkingdirlen - 1) != 0) {
        send_error(401, fd, is_valid_request, response, response_string);
        return;
    }

    struct stat sb;

    if (stat(finalpath, &sb) < 0) {
        send_error(401, fd, is_valid_request, response, response_string);
        return;
    }

    int isDir = S_ISDIR(sb.st_mode);
 
    DIR* dir;
    struct dirent* dirp;
    int pathlen = strlen(finalpath);
    char indexfile[pathlen + INDEX_SIZE];
    /* Doing a +1 for termination with \0*/
    (void)strncpy(indexfile, finalpath, pathlen + 1);
    (void)strncat(indexfile, "/index.html", INDEX_SIZE - 1);
	if (access(indexfile, R_OK) == 0 || !isDir) {
        FILE* fp;
        if (!isDir) {
            if ((fp = fopen(finalpath, "r")) == NULL) {
                send_error(401, fd, is_valid_request, response,
                           response_string);
                return;
            }
        } else {
            if ((fp = fopen(indexfile, "r")) == NULL) {
                send_error(401, fd, is_valid_request, response,
                           response_string);
                return;
            }
        }
        ssize_t temp;
        char* line = NULL;
        size_t linesize = 0;
    
	    while ((temp = getline(&line, &linesize, fp)) != -1) {
		    write(fd, line, strlen(line));
            }
	    write(fd, "\n", strlen("\n"));
    } else {
        if ((dir = opendir(finalpath)) == NULL) {
            send_error(401, fd, is_valid_request, response, response_string);
            return;
        }
	send_headers(fd, is_valid_request, response, response_string);
    
	    while ((dirp = readdir(dir)) != NULL) {
	    if (strncmp(dirp->d_name, ".", 1) != 0) {
                (void)write(fd, dirp->d_name, strlen(dirp->d_name));
		write(fd, "\n", strlen("\n"));
            }
        }
    }
	close_connection(fd);
}
