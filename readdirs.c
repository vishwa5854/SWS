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

    int workingdirlen = strlen(workingdir);

    char* realworkingdir = NULL;

    struct stat sb;

    if (stat(dirname, &sb) != 0) {
          send_error(401, fd, is_valid_request, response, response_string);
          return;
    }

    int isDir = S_ISDIR(sb.st_mode); 

    if (workingdir[workingdirlen - 1] != '/' && isDir) {
	    if ((realworkingdir = malloc((workingdirlen + 1) * sizeof(char*))) == NULL) {
		    send_error(500, fd, is_valid_request, response, response_string);
		    return;
	    }
	    if (strncat(realworkingdir, "/", strlen("/")) == NULL) {
		    send_error(500, fd, is_valid_request, response, response_string);
		    return;
	    }
    }

    if (realworkingdir == NULL) {
	    if (strncpy(realworkingdir, workingdir, workingdirlen) == NULL) {
		    send_error(500, fd, is_valid_request, response, response_string);
		    return;
	    }
    }

    if (realpath(dirname, path) == NULL) {
        send_error(404, fd, is_valid_request, response, response_string);
        return;
    }
    
          /* Lock user in the current working directory */
    if (strncmp(path, realworkingdir, strlen(realworkingdir)) != 0) {
            bzero(path, strlen(path));
            if (strncpy(path, realworkingdir, strlen(realworkingdir)) == NULL) {
                    send_error(500, fd, is_valid_request, response, response_string);
                    return;
            }
            if (strncat(path, dirname, strlen(dirname)) == NULL) {
                    send_error(500, fd, is_valid_request, response, response_string);
                    return;
            }


            if (realpath(path, path) == NULL) {
                    send_error(404, fd, is_valid_request, response, response_string);
                    return;
            }

            if (strncmp(path, realworkingdir, strlen(realworkingdir)) != 0) {
                    send_error(401, fd, is_valid_request, response, response_string);
                    return;
            }
    }
    DIR* dir;
    struct dirent* dirp;
    int dirlen = strlen(dirname);
    char indexfile[dirlen + INDEX_SIZE];
    /* Doing a +1 for termination with \0*/
    (void)strncpy(indexfile, dirname, dirlen + 1);
    (void)strncat(indexfile, "/index.html", INDEX_SIZE - 1);
	if (access(indexfile, R_OK) == 0 || !isDir) {
        FILE* fp;
        if (!isDir) {
            if ((fp = fopen(dirname, "r")) == NULL) {
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
        if ((dir = opendir(dirname)) == NULL) {
            send_error(401, fd, is_valid_request, response, response_string);
            return;
        }
    
	    while ((dirp = readdir(dir)) != NULL) {
	    if (strncmp(dirp->d_name, ".", 1) != 0) {
                (void)write(fd, dirp->d_name, strlen(dirp->d_name));
		write(fd, "\n", strlen("\n"));
            }
        }
    }
}
