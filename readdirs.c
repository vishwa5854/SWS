#define INDEX_SIZE	12

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

char** readdirs(char* dirname, int flag) {
	DIR* dir;
	struct dirent *dirp;
	/* Going with a double pointer for a string array */
	char** dirs;

	/* Straight out max size to avoid any errors due to length*/
	if((dirs = malloc(INT_MAX * sizeof(char*))) == NULL) {
		perror("Could not allocate memory\n");
		return(NULL);
	}
	if ((dir = opendir(dirname)) == NULL) {
		perror("Cound not open directory\n");
		return(NULL);
	}
	int count = 0;
	int dirlen = strlen(dirname);
	char indexfile[dirlen + INDEX_SIZE];
	/* Doing a +1 for termination with \0*/
	(void) strncpy(indexfile, dirname, dirlen + 1);
	(void) strncat(indexfile, "/index.html", INDEX_SIZE - 1);
	if (access(indexfile, R_OK) == 0 && flag) {
		FILE *fp;
		if ((fp = fopen(indexfile, "r")) == NULL) {
			perror("Could not open index file\n");
			return(NULL);
		}
		ssize_t temp;
		char* line = NULL;
		size_t linesize = 0;
		while ((temp = getline(&line, &linesize, fp)) != -1) {
			int templen = strlen(line);
			if ((dirs[count] = malloc(templen * sizeof(char*))) == NULL) {
				perror("Could not allocate memory");
				return(NULL);
			}
			(void) strncpy(dirs[count], line, templen);
			count++;
		}
	}
	else {
		while((dirp = readdir(dir)) != NULL) {
			int templen = strlen(dirp->d_name);
			if ((dirs[count] = malloc(templen * sizeof(char*))) == NULL) {
				perror("Could not allocate memory\n");
				return(NULL);
			}
			(void) strncpy(dirs[count], dirp->d_name, templen);
			count++;
		}
	}
	return(dirs);
}
