#include "handler.h"
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include "util.h"
#include "structures.h"
#include "cgi.h"

int current_fd;
bool close_current_connection = true;

void close_connection(int fd) {
    /** Sending a tcp close connection message to the client */
    (void)shutdown(fd, SHUT_RDWR);
    (void)close(fd);

    /** Lemme die peacefully :) */
    _exit(EXIT_SUCCESS);
}

void createResponse(RESPONSE *response) {
    (void)strncpy((*response).server, SERVER, strlen(SERVER));
    (void)strncpy((*response).content_type, CONTENT_TYPE_DEFAULT, strlen(CONTENT_TYPE_DEFAULT));
    get_gmt_date_str((*response).date, DATE_MAX_LEN);
    (void)strncpy((*response).protocol, SUPPORTED_PROTOCOL_ONLY, strlen(SUPPORTED_PROTOCOL_ONLY));
    (void)strncpy((*response).version, SUPPORTED_VERSION_ONLY, strlen(SUPPORTED_VERSION_ONLY));
    (*response).last_modified[0] = '\0';
    (*response).content_length = 0;
    get_status_verb((*response).status_code, (*response).status_verb);
}

void alarm_handler(int sig_num) {
    if (close_current_connection) {
        /** Obviously this is for testing purpose only */
        (void)printf("Signal passed is %d\n", sig_num);
        close_connection(current_fd);
    }
}

void handleFirstLine(int fd, const char *separator, char *token, char *line_buffer, bool *is_first_line,
                     bool *is_valid_request, REQUEST *request) {
    /**
    * 1. Split the string and set the request type, resource URI, protocol & version
    * 2. Validate all of them and then if it's valid then continue else terminate connection
    */
    int iterator = 0;
    token = strtok(line_buffer, separator);

    while (token != NULL) {
        if (iterator <= 2) {
            (*is_valid_request) = (*is_valid_request) && create_request_frame(request, token, iterator);

            /** This is for testing purpose only */
            if (iterator == 1) {
                (void)fd;
                // execute_file(token, fd);
            }
        } else {
            /** This is a bad request brother */
            (*is_valid_request) = false;
        }
        iterator++;
        token = strtok(NULL, separator);
    }
    (*is_first_line) = false;

    /** We need minimum three tokens */
    if (iterator < 2) {
        (*is_valid_request) = false;
    }
}

/** This code has been referenced from CS631 APUE class notes apue-code/09 */
void handleConnection(int fd, struct sockaddr_in6 client) {
    const char *rip;
    char claddr[INET6_ADDRSTRLEN];

    if ((rip = inet_ntop(PF_INET6, &(client.sin6_addr), claddr, INET6_ADDRSTRLEN)) == NULL) {
        perror("inet_ntop");
    } else {
        (void)printf("Client connection from %s!\n", rip);
    }

    bool is_first_line = true;
    char separator[2] = " ";
    char* token;
    bool is_valid_request = true;
    REQUEST request;
    RESPONSE response;
    response.status_code = 0;
    char response_string[BUFSIZ];
    bzero(response_string, sizeof(response_string));
    
    reset_request_object(&request);
    reset_response_object(&response);

    int number_of_headers = 0;
    char line_buffer_chars[1];
    bool last_char_was_next_line = false;
    bool stream_done = false;
    bool end_of_request = false;
    
    while (!stream_done) {
        int cursor = 0;
        char line_buffer[SUPPORTED_MAX_HEADER_SIZE + 1];
        bzero(line_buffer, sizeof(line_buffer));

        while (true) {
            if (read(fd, line_buffer_chars, 1) <= 0) {
                stream_done = true;
                break;
            }

            if (strncmp(line_buffer_chars, "\n", 1) == 0) {
                if (last_char_was_next_line) {
                    /** This is the end of the request. */
                    end_of_request = true;
                }
                last_char_was_next_line = true;
                break;
            } else {
                if (strncmp(line_buffer_chars, "\r", 1) != 0) {
                    last_char_was_next_line = false;
                    line_buffer[cursor] = line_buffer_chars[0];
                    cursor++;
                }
            }

            /** Client has given a header that is greater than the size we accept. */
            if (cursor >= SUPPORTED_MAX_HEADER_SIZE) {
                end_of_request = true;
                response.status_code = 413;
                break;
            }
        }
        line_buffer[cursor + 1] = '\0';
        number_of_headers++;

        if (number_of_headers >= MAX_NUMBER_OF_HEADERS) {
            end_of_request = true;
            response.status_code = 413;
            break;
        }

        if (end_of_request) {
            break;
        }

        if (number_of_headers == 1) {
           handleFirstLine(fd, separator, token, line_buffer, &is_first_line, &is_valid_request, &request);
        } else {
            /** Reading the headers we don't validate or care about anything else except for If-Modified-Since */
           token = strtok(line_buffer, separator);

           if (
                (strlen(token) == strlen(SUPPORTED_HEADER)) &&
                (strncmp(token, SUPPORTED_HEADER, strlen(token)) == 0)
            ) {
               token = strtok(NULL, "");
               is_valid_request = is_valid_request && create_request_frame(&request, token, 3);
           } else {
               continue;
           }
        }
    }

    /** Assigning status codes will be revisited. */
    if (response.status_code == 0) {
        if (!is_valid_request) {
            response.status_code = 400;
            /** We shouldn't terminate the request we have to wait until the user is done */
        } else {
            response.status_code = 200;
        }
    }

    createResponse(&response);
    /**
     * Even when the timeout is done, we shouldn't close this connection as user is done with his shit 
     * and we are the ones pending to serve either dir indexing, file serving or CGI execution
    */
    close_current_connection = false;
    /** We stop taking anything else from client now */
    (void)create_response_string(&response, response_string);
    write(fd, response_string, strlen(response_string));

    /** Bubyeee */
    close_connection(fd);
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
    } else if (!pid) {
        current_fd = fd;
        signal(SIGALRM, alarm_handler);
        alarm(TIMEOUT);
        handleConnection(fd, client);
    }
}