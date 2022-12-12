#ifndef CGI_H_
#define CGI_H_

#include "flags.h"
#include <stdbool.h>

int execute_file(const char *string, int socket_fd, bool is_valid_request, RESPONSE *response, char *response_string,struct flags_struct flags);

#endif
