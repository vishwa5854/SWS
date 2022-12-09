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

void readdirs(char* dirname, int fd, bool is_valid_request, RESPONSE* response,
              char* response_string) {
    char path[PATH_MAX];

    if (realpath(dirname, path) == NULL) {
        send_error(404, fd, is_valid_request, response, response_string);
        return;
    }
    char cwd[PATH_MAX];
    
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
        send_error(500, fd, is_valid_request, response, response_string);
        return;
    }
    
	/* Lock user in the current working directory */
    if (strncmp(dirname, cwd, strlen(cwd)) != 0) {
        send_error(401, fd, is_valid_request, response, response_string);
        return;
    }
    DIR* dir;
    struct dirent* dirp;
    /* Going with a double pointer for a string array */
    char** dirs;

    /* Straight out max size to avoid any errors due to length*/
    if ((dirs = malloc(INT_MAX * sizeof(char*))) == NULL) {
        send_error(500, fd, is_valid_request, response, response_string);
        return;
    }
    int count = 0;
    int dirlen = strlen(dirname);
    char indexfile[dirlen + INDEX_SIZE];
    /* Doing a +1 for termination with \0*/
    (void)strncpy(indexfile, dirname, dirlen + 1);
    (void)strncat(indexfile, "/index.html", INDEX_SIZE - 1);
    struct stat sb;
    
	if (stat(dirname, &sb) != 0) {
        send_error(401, fd, is_valid_request, response, response_string);
        return;
    }
    int isDir = S_ISDIR(sb.st_mode);
    
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
            int templen = strlen(line);
    
	        if ((dirs[count] = malloc(templen * sizeof(char*))) == NULL) {
                send_error(500, fd, is_valid_request, response,
                           response_string);
                return;
            }
            (void)strncpy(dirs[count], line, templen);
            count++;
        }
    } else {
        if ((dir = opendir(dirname)) == NULL) {
            send_error(401, fd, is_valid_request, response, response_string);
            return;
        }
    
	    while ((dirp = readdir(dir)) != NULL) {
            int templen = strlen(dirp->d_name);
    
	        if ((dirs[count] = malloc(templen * sizeof(char*))) == NULL) {
                send_error(500, fd, is_valid_request, response,
                           response_string);
                return;
            }
    
	        if (strncmp(dirp->d_name, ".", 1) != 0) {
                (void)strncpy(dirs[count], dirp->d_name, templen);
                count++;
            }
        }
    }
    FILE* fp;
    
	if ((fp = fdopen(fd, "a")) == NULL) {
        send_error(500, fd, is_valid_request, response, response_string);
    }
    count = 0;
    
	while (dirs[count] != NULL) {
        if (fprintf(fp, dirs[count]) == -1) {
            send_error(500, fd, is_valid_request, response, response_string);
        }
        count++;
    }
}
