#include "handler.h"
#include <stdbool.h>
#include "util.h"
#include "structures.h"

/** This code has been referenced from CS631 APUE class notes apue-code/09 */
void handleConnection(int fd, struct sockaddr_in6 client) {
    const char *rip;
    int client_request;
    char claddr[INET6_ADDRSTRLEN];

    if ((rip = inet_ntop(PF_INET6, &(client.sin6_addr), claddr, INET6_ADDRSTRLEN)) == NULL) {
        perror("inet_ntop");
        rip = "unknown";
    } else {
        (void)printf("Client connection from %s!\n", rip);
    }

    bool is_first_line = true;
    char separator[2] = " ";
    char* token;
    bool is_valid_request = true;
    REQUEST request;
    RESPONSE response;
    char response_string[BUFSIZ];
    bzero(response_string, sizeof(response_string));

    do {
        char line_buffer[BUFSIZ];
        bzero(line_buffer, sizeof(line_buffer));

        if ((client_request = read(fd, line_buffer, BUFSIZ)) < 0) {
            perror("reading stream message");
        } else if (client_request == 0) {
            printf("Ending connection from %s.\n", rip);
        } else {
            if (is_first_line) {
                /**
                 * 1. Split the string and set the request type, resource URI, protocol & version
                 * 2. Validate all of them and then if it's valid then continue else terminate connection
                */
               int iterator = 0;
               token = strtok(line_buffer, separator);

                while (token != NULL) {
                    if (iterator <= 2) {
                        is_valid_request = is_valid_request && create_request_frame(&request, token, iterator);
                    } else if (iterator > 2) {
                        /** This is a bad request brother */
                        is_valid_request = false;
                    }
                    iterator++;
                    token = strtok(NULL, separator);
                }
                is_first_line = false;

                /** We need minimum three tokens */
                if (iterator < 2) {
                    is_valid_request = false;
                }

                /** Assigning status codes will be revisited. */
                if (!is_valid_request) {
                    response.status_code = 400;
                    /** We shouldn't terminate the request we have to wait until the user is done */
                } else {
                    response.status_code = 200;
                }

                (void)strncpy(response.server, SERVER, strlen(SERVER));
                (void)strncpy(response.content_type, CONTENT_TYPE_DEFAULT, strlen(CONTENT_TYPE_DEFAULT));
                get_gmt_date_str(response.date, DATE_MAX_LEN);
                (void)strncpy(response.protocol, SUPPORTED_PROTOCOL_ONLY, strlen(SUPPORTED_PROTOCOL_ONLY));
                (void)strncpy(response.version, SUPPORTED_VERSION_ONLY, strlen(SUPPORTED_VERSION_ONLY));
                response.last_modified[0] = '\0';
                response.content_length = 0;
                get_status_verb(response.status_code, response.status_verb);
            }
            /** We stop taking anything else from client now */
            if (strncmp(line_buffer, "\r\n", strlen("\r\n")) == 0) {
                (void)create_response_string(&response, response_string);
                write(fd, response_string, strlen(response_string));
                break;
            } else {
                /** Reading the headers we don't validate or care about anything else except for If-Modified-Since */

            }
            printf("Client (%s) sent: %s", rip, line_buffer);
        }
    } while (client_request != 0);

    /** Sending a tcp close connection message to the client */
    (void)shutdown(fd, SHUT_RDWR);
    (void)close(fd);
    _exit(EXIT_SUCCESS);
    /* NOTREACHED */
}

/** This code has been referenced from CS631 APUE class notes apue-code/09 */
void handleSocket(int socket) {
    int fd;
    pid_t pid;
    struct sockaddr_in6 client;
    socklen_t length;
    
    memset(&client, 0, sizeof(client));
    length = sizeof(client);

    if ((fd = accept(socket, (struct sockaddr *)&client, &length)) < 0) {
        perror("accept");
        return;
    }

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
        /* NOTREACHED */
    } else if (!pid) {
        handleConnection(fd, client);
        /** Child dies peacefully :) */
        // exit(EXIT_SUCCESS);
        /* NOTREACHED */
    }
    /* parent silently returns */
}