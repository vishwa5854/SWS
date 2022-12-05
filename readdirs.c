#define INDEX_SIZE	12

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>

void readdirs(char* dirname) {
	char path[PATH_MAX];
	if (realpath(dirname, path) == NULL) {
		return(NULL);
	}
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		return(NULL);
	}
	/* Lock user in the current working directory */
	if (strncmp(dirname, cwd, strlen(cwd)) != 0) {
		return(NULL);
	}
	DIR* dir;
	struct dirent *dirp;
	/* Going with a double pointer for a string array */
	char** dirs;

	/* Straight out max size to avoid any errors due to length*/
	if((dirs = malloc(INT_MAX * sizeof(char*))) == NULL) {
		perror("Could not allocate memory\n");
		return(NULL);
	}
	int count = 0;
	int dirlen = strlen(dirname);
	char indexfile[dirlen + INDEX_SIZE];
	/* Doing a +1 for termination with \0*/
	(void) strncpy(indexfile, dirname, dirlen + 1);
	(void) strncat(indexfile, "/index.html", INDEX_SIZE - 1);
	struct stat sb;
	if (stat(dirname, &sb) != 0) {
		perror("Could not open file\n");
		return(NULL);
	}
	int isDir = S_ISDIR(sb.st_mode);
	if (access(indexfile, R_OK) == 0 || !isDir) {
		FILE *fp;
		if (!isDir) {
			if ((fp = fopen(dirname, "r")) == NULL) {
				perror("Could not open file\n");
				return(NULL);
			}
		}
		else {
			if ((fp = fopen(indexfile, "r")) == NULL) {
				perror("Could not open index file\n");
				return(NULL);
			}
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
		if ((dir = opendir(dirname)) == NULL) {
                  	perror("Cound not open directory\n");
                  	return(NULL);
          	}
		while((dirp = readdir(dir)) != NULL) {
			int templen = strlen(dirp->d_name);
			if ((dirs[count] = malloc(templen * sizeof(char*))) == NULL) {
				perror("Could not allocate memory\n");
				return(NULL);
			}
			if (strncmp(dirp->d_name, ".", 1) != 0) {
				(void) strncpy(dirs[count], dirp->d_name, templen);
				count++;
			}
		}
	}
	return(dirs);
}
