#ifndef READDIRS
#define READDIRS_H

#include <stdbool.h>

#include "structures.h"

void readdirs(char *dirname, char* workingdir, int fd, bool is_valid_request, RESPONSE *response,
              char *response_string);

#endif /* READDIRS_H */
