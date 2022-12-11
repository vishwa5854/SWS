#ifndef HANDLER_H_
#define HANDLER_H_

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "flags.h"
#include "structures.h"

void handleConnection(int fd, struct sockaddr_in6 client);

void handleSocket(int socket, struct flags_struct);

void send_headers(int fd, bool is_valid_request, RESPONSE *response,
                  char *response_string);

void send_error(int status_code, int socket_fd, bool is_valid_request,
                RESPONSE *response, char *response_string);

void close_connection(int fd);

#endif /* !HANDLER_H_ */