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

char** getuserdir() {
	struct passwd *p;
	if ((p = getpwuid(getuid())) == NULL)
		return(NULL);
	int swsdirlen = strlen(p->pw_dir) + SWS_LEN;
	char* swsdir;
	if ((swsdir = malloc(swsdirlen * sizeof(char*))) == NULL)
		return(NULL);
	(void) strncpy(swsdir, p->pw_dir, strlen(p->pw_dir));
	(void) strncat(swsdir, "/sws", SWS_LEN);
	char** dirs = readdirs(swsdir);	
	return(dirs);
}
