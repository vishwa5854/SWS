#ifndef READDIRS
#define READDIRS_H

#include <stdbool.h>
#include <time.h>

#include "structures.h"

void readdirs(char *dirname, char* workingdir, int fd, time_t modified_since, bool is_valid_request, RESPONSE *response,
              char *response_string);

#endif /* READDIRS_H */
