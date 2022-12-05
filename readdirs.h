#ifndef READDIRS
#define READDIRS_H

#include <stdbool.h>

#include "structures.h"

void readdirs(char *dirname, int fd, bool is_valid_request, RESPONSE *response,
              char *response_string, bool flag);

#endif /* READDIRS_H */
