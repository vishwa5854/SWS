#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

char** readdirs(char* dirname) {
	DIR* dir;
	struct dirent *dirp;
	char** dirs;

	if ((dirs = malloc(INT_MAX * sizeof(char*))) == NULL) {
		perror("Could not allocate memory\n");
		exit(EXIT_FAILURE);
	}
	if ((dir = opendir(dirname)) == NULL) {
		perror("Cound not open directory\n");
		exit(EXIT_FAILURE);
	}
	int count = 0;
	while((dirp = readdir(dir)) != NULL) {
		int templen = strlen(dirp->d_name);
		if ((dirs[count] = malloc(templen * sizeof(char*))) == NULL) {
			perror("Could not allocate memory\n");
			exit(EXIT_FAILURE);
		}
		(void) strncpy(dirs[count], dirp->d_name, templen);
		count++;
	}
	return(dirs);
}
