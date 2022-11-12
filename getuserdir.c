#define HOME_DIR_LEN	10

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "readdirs.h"

char** getuserdir(char* username) {
	int length = strlen(username);
	char* user;
	int count = 0;
	while (count < length) {
		if (username[count] == '/') {
			/* Weirdly, when tried with char user[len] it always had an @ in the end so malloc it is */
			if ((user = malloc((count + 1) * sizeof(char*))) == NULL) {
				perror("Count not allocate memory");
				return(NULL);
			}
			(void) strncpy(user, username, count + 1);
			break;
		}
		count++;
	}
	if (count == length) {
		return(NULL);
	}
	char swsdir[HOME_DIR_LEN + strlen(user)];
	(void) strncpy(swsdir, "/home/", strlen("/home/"));
	(void) strncat(swsdir, user, strlen(user));
	(void) strncat(swsdir, "sws", strlen("sws"));
	char** dirs = readdirs(swsdir);	
	return(dirs);
}

