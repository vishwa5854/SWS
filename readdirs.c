#define INDEX_SIZE	12

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

char** readdirs(char* dirname) {
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
	while((dirp = readdir(dir)) != NULL) {
		int templen = strlen(dirp->d_name);
		if ((dirs[count] = malloc(templen * sizeof(char*))) == NULL) {
			perror("Could not allocate memory\n");
			exit(EXIT_FAILURE);
		}
	}
	return(dirs);
}
