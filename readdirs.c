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
#include <time.h>
#include <unistd.h>

#include "handler.h"
#include "structures.h"
#include "util.h"

void readdirs(char* dirname, char* workingdir, int fd, time_t modified_since, bool is_valid_request, RESPONSE* response,
              char* response_string) {
    char path[PATH_MAX];
    char realworkingdir[PATH_MAX];

    if ((realpath(workingdir, realworkingdir)) == NULL) {
	    send_error(500, fd, is_valid_request, response, response_string);
	    close_connection(fd);
    }

    int realworkingdirlen = strlen(realworkingdir);

    if (realworkingdir[realworkingdirlen - 1] != '/') {
        realworkingdirlen++;
	    if (strncat(realworkingdir, "/", strlen("/")) == NULL) {
		    send_error(500, fd, is_valid_request, response, response_string);
		    close_connection(fd);
	    }
    }

    if (strncmp(dirname, realworkingdir, realworkingdirlen - 1) == 0) {
	    if (strncpy(path, dirname, strlen(dirname)) == NULL) {
		    send_error(500, fd, is_valid_request, response, response_string);
		    close_connection(fd);
	    }
    }
    else {
        if (strncpy(path, realworkingdir, realworkingdirlen) == NULL) {
            send_error(500, fd, is_valid_request, response, response_string);
            close_connection(fd);
        }
        if (strncat(path, dirname, strlen(dirname)) == NULL) {
            send_error(500, fd, is_valid_request, response, response_string);
            close_connection(fd);
        }
    }

    char finalpath[PATH_MAX];

    if (realpath(path, finalpath) == NULL) {
        send_error(404, fd, is_valid_request, response, response_string);
        close_connection(fd);
    }


    if (strncmp(finalpath, realworkingdir, realworkingdirlen - 1) != 0) {
        send_error(401, fd, is_valid_request, response, response_string);
        close_connection(fd);
    }

    struct stat sb;

    if (stat(finalpath, &sb) < 0) {
        send_error(401, fd, is_valid_request, response, response_string);
        close_connection(fd);
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
            if (sb.st_mtim < modified_since) {
                send_error(304, fd, is_valid_request, response, response_string);
                close_connection(fd);
            }
            if ((fp = fopen(finalpath, "r")) == NULL) {
                send_error(401, fd, is_valid_request, response,
                           response_string);
                close_connection(fd);
            }
        } else {
            if (stat(indexfile, &sb) == NULL) {
                send_error(401, fd ,is_valid_request, response, response_string);
                close_connection(fd);
            }
            if (sb.st_mtim < modified_since) {
                send_error(304, fd, is_valid_request, response, response_string);
                close_connection(fd);
            }
            if ((fp = fopen(indexfile, "r")) == NULL) {
                send_error(401, fd, is_valid_request, response,
                           response_string);
                close_connection(fd);
            }
        }
        ssize_t temp;
        char* line = NULL;
        size_t linesize = 0;

        char* last_modified_time[DATE_MAX_LEN];
        get_gmt_date_str(last_modified_time, DATE_MAX_LEN);
        if (strncpy(response->last_modified, last_modified_time, DATE_MAX_LEN) == NULL) {
            send_error(500, fd, is_valid_request, response, response_string);
            close_connection(fd);
        }
        response->content_length = sb.st_size;
        response->status_code = 200;
        send_headers(fd, is_valid_request, response, response_string);
	    while ((temp = getline(&line, &linesize, fp)) != -1) {
		    write(fd, line, strlen(line));
            }
	    write(fd, "\n", strlen("\n"));
    } else {
        if ((dir = opendir(finalpath)) == NULL) {
            send_error(401, fd, is_valid_request, response, response_string);
            close_connection(fd);
        }
        response->status_code = 200;
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

int main(int argc, char** argv) {
	RESPONSE response;
	readdirs(argv[1],argv[2], STDOUT_FILENO, 0, true, &response, "OK");
	return(0);
}
