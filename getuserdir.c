/* Length of the string /sws + 1 for safety */
#define SWS_LEN 5

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>

#include "readdirs.h"

char** getuserdir(char* userstr) {
	int userstrlen = strlen(userstr);
	char* username;
	char* requestedContent;
	int i;
	for (i = 0; i < userstrlen; i++) {
		if (userstr[i] == '/') {
			if ((username = malloc((i + 1) * sizeof(char*))) == NULL)
				return(NULL);
			if ((requestedContent = malloc((userstrlen - i - 1) * sizeof(char*))) == NULL)
				return(NULL);
			(void) strncpy(username, userstr, i);
			(void) strncpy(requestedContent, userstr + i + 1, userstrlen - i - 1);
			break;
		}
	}
	struct passwd *p;
	if ((p = getpwnam(username)) == NULL)
		return(NULL);
	int swsdirlen = strlen(p->pw_dir) + SWS_LEN + userstrlen - i - 1;
	char* swsdir;
	if ((swsdir = malloc(swsdirlen * sizeof(char*))) == NULL)
		return(NULL);
	(void) strncpy(swsdir, p->pw_dir, strlen(p->pw_dir));
	(void) strncat(swsdir, "/sws/", SWS_LEN);
	(void) strncat(swsdir, requestedContent, userstrlen - i - 1);
	char** dirs = readdirs(swsdir);	
	return(dirs);
}
