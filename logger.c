#include "logger.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "structures.h"

int writelog(int fd, struct HTTP_REQUEST request, struct HTTP_RESPONSE response,
             char* requestip) {
    FILE* fp;

    if ((fp = fdopen(fd, "a")) == NULL) {
        return (-1);
    }
    char* writedata;
    
	if ((writedata = malloc(PATH_MAX * sizeof(char*))) == NULL) {
        return (-1);
    }
    (void)strncpy(writedata, requestip, strlen(requestip));
    (void)strncat(writedata, " ", 1);
    struct tm* gmttime;
    const time_t requesttime = request.time_request_made;
    
	if ((gmttime = gmtime(&requesttime)) == NULL) {
        return (-1);
    }
    char* finaltime;
    
	if ((finaltime = malloc(DATE_MAX_LEN * sizeof(char*))) == NULL) {
        return (-1);
    }
    
	if ((strftime(finaltime, DATE_MAX_LEN, "%FT%TZ", gmttime)) == 0) {
        return (-1);
    }
    (void)strncat(writedata, finaltime, strlen(finaltime));
    (void)strncat(writedata, " \"", 2);
    (void)strncat(writedata, request.verb, strlen(request.verb));
    (void)strncat(writedata, "\" ", 2);
    char* status_code;
    
	if ((status_code = malloc(INT_MAX * sizeof(char*))) == 0) {
        return (-1);
    }
    
	if ((sprintf(status_code, "%d", response.status_code)) == -1) {
        return (-1);
    }
    (void)strncat(writedata, status_code, strlen(status_code));
    char* content_length;
    
	if ((content_length = malloc(INT_MAX * sizeof(char*))) == 0) {
        return (-1);
    }
    
	if ((sprintf(content_length, "%ld", response.content_length)) == -1) {
        return (-1);
    }
    (void)strncat(writedata, " ", 1);
    (void)strncat(writedata, content_length, strlen(content_length));
    (void)strncat(writedata, "\n", 1);
    
	if ((fprintf(fp, writedata)) == -1) {
        return (-1);
    }
    (void)fclose(fp);
    return (0);
}
