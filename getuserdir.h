#ifndef GETUSERDIR
#define GETUSERDIR_H

#include <stdbool.h>
#include <time.h>

#include "structures.h"

void getuserdir(char *username, int fd, time_t modified_since, bool is_valid_request,
                RESPONSE *response, char *response_string);

#endif /* GETUSERDIR_H */
