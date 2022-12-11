#ifndef GETUSERDIR
#define GETUSERDIR_H

#include <stdbool.h>

#include "structures.h"

void getuserdir(char *username, int fd, bool is_valid_request,
                RESPONSE *response, char *response_string);

#endif /* GETUSERDIR_H */
