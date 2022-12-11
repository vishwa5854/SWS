#include "handler.h"

#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "cgi.h"
#include "flags.h"
#include "structures.h"
#include "util.h"

int current_fd;
bool close_current_connection = true;
bool debug_mode = false;

void close_connection(int fd) {
    /** Sending a tcp close connection message to the client */
    (void)shutdown(fd, SHUT_RDWR);
    (void)close(fd);

    /** In debug mode there is no child process that is handling client request,
     * if we exit it will cause the entire server to stop */
    if (!debug_mode) {
        /** Child dies peacefully :) */
        _exit(EXIT_SUCCESS);
    }
}

void createResponse(RESPONSE *response) {
    (void)strncpy((*response).server, SERVER, strlen(SERVER));
    (void)strncpy((*response).content_type, CONTENT_TYPE_DEFAULT,
                  strlen(CONTENT_TYPE_DEFAULT));
    get_gmt_date_str((*response).date, DATE_MAX_LEN);
    (void)strncpy((*response).protocol, SUPPORTED_PROTOCOL_ONLY,
                  strlen(SUPPORTED_PROTOCOL_ONLY));
    (void)strncpy((*response).version, SUPPORTED_VERSION_ONLY,
                  strlen(SUPPORTED_VERSION_ONLY));
    (*response).last_modified[0] = '\0';
    (*response).content_length = 0;
    get_status_verb((*response).status_code, (*response).status_verb);
}

void alarm_handler(int sig_num) {
    /** The definition of the handler is supposed to have this but I have no use
     * with it :( */
    (void)sig_num;

    /** If in the meantime there has been something going on between clien
     * and server then we won't close the connection yet. */
    if (close_current_connection) {
        close_connection(current_fd);
    }
}

void handleFirstLine(const char *separator, char *token, char *line_buffer,
                     bool *is_first_line, bool *is_valid_request,
                     REQUEST *request) {
    int iterator = 0;
    token = strtok(line_buffer, separator);

    while (token != NULL) {
        if (iterator <= 2) {
            (*is_valid_request) =
                (*is_valid_request) &&
                create_request_frame(request, token, iterator);
        } else {
            /** This is a bad request brother */
            (*is_valid_request) = false;
        }
        iterator++;
        token = strtok(NULL, separator);
    }
    (*is_first_line) = false;

    /** We need minimum three tokens ("GET", "/", "HTTP/1.0") */
    if (iterator < 2) {
        (*is_valid_request) = false;
    }
}

/** Make sure to set the status_code and content length before calling this
 * function :) */
void send_headers(int fd, bool is_valid_request, RESPONSE *response,
                  char *response_string) {
    if ((*response).status_code == 0) {
        if (!is_valid_request) {
            (*response).status_code = 400;
        } else {
            (*response).status_code = 200;
        }
    }

    createResponse(response);
    /**
     * Even when the timeout is done, we shouldn't close this connection as user
     * is done with his input and we are the ones pending to serve either dir
     * indexing, file serving or CGI execution
     */
    close_current_connection = false;

    /** We stop taking anything else from client now */
    (void)create_response_string(response, response_string);

    /** We couldn't write headers, so it is better to close the connection and
     * call it a day */
    if (write(fd, response_string, strlen(response_string)) < 0) {
        close_connection(fd);
    }
}

void send_error(int status_code, int socket_fd, bool is_valid_request,
                RESPONSE *response, char *response_string) {
    response->status_code = status_code;

    send_headers(socket_fd, is_valid_request, response, response_string);

    /** Sending the error again because clients like browsers will just show
     * empty page on error if we don't. */
    if (write(socket_fd, response->status_verb, strlen(response->status_verb)) <
        0) {
        close_connection(socket_fd);
    }

    if (write(socket_fd, "\r\n", strlen("\r\n")) < 0) {
        close_connection(socket_fd);
    }

    close_connection(socket_fd);
}

/** This code has been referenced from CS631 APUE class notes apue-code/09 */
void handleConnection(int fd, struct sockaddr_in6 client) {
    /** TODO: Integration of logger will use the rip */
    const char *rip;
    char claddr[INET6_ADDRSTRLEN];
    bool is_first_line = true;
    char separator[2] = " ";
    char *token;
    bool is_valid_request = true;
    ssize_t n_chars = 0;
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

    if ((rip = inet_ntop(PF_INET6, &(client.sin6_addr), claddr,
                         INET6_ADDRSTRLEN)) == NULL) {
        perror("inet_ntop");

        /** Something wrong with rip */
        send_error(500, fd, is_valid_request, &response, response_string);
    }

    while (!stream_done) {
        int cursor = 0;
        char line_buffer[SUPPORTED_MAX_HEADER_SIZE + 1];
        bzero(line_buffer, sizeof(line_buffer));

        while (true) {
            /** We are reading one character at a time to parse the entire
             * request */
            n_chars = read(fd, line_buffer_chars, 1);

            /** An error has occured while reading :( */
            if (n_chars < 0) {
                send_error(500, fd, is_valid_request, &response,
                           response_string);
                return;
            }

            if (n_chars == 0) {
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

            /** Client has given a header that is greater than the size we
             * accept. */
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
            handleFirstLine(separator, token, line_buffer, &is_first_line,
                            &is_valid_request, &request);
        } else {
            /** Reading the headers we don't validate or care about anything
             * else except for If-Modified-Since */
            token = strtok(line_buffer, separator);

            if ((strlen(token) == strlen(SUPPORTED_HEADER)) &&
                (strncmp(token, SUPPORTED_HEADER, strlen(token)) == 0)) {
                token = strtok(NULL, "");
                is_valid_request = is_valid_request &&
                                   create_request_frame(&request, token, 3);
            } else {
                continue;
            }
        }
    }

    if (is_valid_request && (request.if_modified_str_type != 0)) {
        struct tm tm;
        time_t t;

        if (request.if_modified_str_type == 1) {
            if (strptime(request.if_modified_since, "%a, %d %b %Y %T GMT",
                         &tm) == NULL) {
                perror("strptime failed");

                /** This is a bad request because the regex matched but the
                 * values doesn't make sense, which is a mistake from client */
                send_error(400, fd, is_valid_request, &response,
                           response_string);
            }
        } else if (request.if_modified_str_type == 2) {
            if (strptime(request.if_modified_since, "%a, %d-%b-%y %T GMT",
                         &tm) == NULL) {
                perror("strptime failed");

                /** This is a bad request because the regex matched but the
                 * values doesn't make sense, which is a mistake from client */
                send_error(400, fd, is_valid_request, &response,
                           response_string);
            }
        } else if (request.if_modified_str_type == 3) {
            if (strptime(request.if_modified_since, "%a %b %d %T %Y GMT",
                         &tm) == NULL) {
                perror("strptime failed");

                /** This is a bad request because the regex matched but the
                 * values doesn't make sense, which is a mistake from client */
                send_error(400, fd, is_valid_request, &response,
                           response_string);
            }
        }

        if ((t = mktime(&tm)) < 0) {
            perror("mktime failed");
            send_error(500, fd, is_valid_request, &response, response_string);
        }

        request.if_modified_t = t;
        send_headers(fd, is_valid_request, &response, response_string);
        /** Bubyeee */
        close_connection(fd);
    } else {
        send_headers(fd, is_valid_request, &response, response_string);
        /** Bubyeee */
        close_connection(fd);
    }

    /** TODO: URL PARSING */

    send_headers(fd, is_valid_request, &response, response_string);
    /** Bubyeee */
    close_connection(fd);
}

/** This code has been referenced from CS631 APUE class notes apue-code/09 */
void handleSocket(int socket, struct flags_struct flags) {
    int fd;
    pid_t pid;
    struct sockaddr_in6 client;
    socklen_t length;

    memset(&client, 0, sizeof(client));
    length = sizeof(client);

    if ((fd = accept(socket, (struct sockaddr *)&client, &length)) < 0) {
        /** If accept is failing there is not much we can do apart from ignoring
         * and moving on to next request because we don't even have fd to say
         * something to client */
        perror("accept");
        return;
    }

    if (flags.d_flag) {
        pid = 0;
        debug_mode = true;
    } else {
        pid = fork();
    }

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (!pid) {
        /** Child */
        current_fd = fd;

        if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
            perror("signal for SIGALRM ");

            /** We need all of these to send an error. :( */
            RESPONSE response;
            response.status_code = 0;
            char response_string[BUFSIZ];
            bzero(response_string, sizeof(response_string));
            reset_response_object(&response);

            send_error(500, fd, true, &response, response_string);
        }

        if (alarm(TIMEOUT) == ((unsigned int)-1)) {
            /** We need all of these to send an error. :( */
            RESPONSE response;
            response.status_code = 0;
            char response_string[BUFSIZ];
            bzero(response_string, sizeof(response_string));
            reset_response_object(&response);

            send_error(500, fd, true, &response, response_string);
        }

        handleConnection(fd, client);
    }
}