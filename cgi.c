#include "cgi.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "flags.h"
#include "handler.h"

int get_real_path(const char *orginal_path, char *resolved_path) {
    if ((resolved_path = realpath(orginal_path, resolved_path)) == NULL) {
        return 0;
    } else {
        return 1;
    }
}

int execute_file(const char *executable_path, int socket_fd,
                 bool is_valid_request, RESPONSE *response,
                 char *response_string, struct flags_struct flags) {
    char resolved_path_of_starting[PATH_MAX];
    bzero(resolved_path_of_starting,sizeof(resolved_path_of_starting));
    char resolved_path_of_executable_path[PATH_MAX];
    bzero(resolved_path_of_executable_path, sizeof(resolved_path_of_executable_path));

    if ((get_real_path(flags.cdi_dir_arg, resolved_path_of_starting)) == 0) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    (void)socket_fd;
    char temp_buff[BUFSIZ];
    bzero(temp_buff,sizeof(temp_buff));
    int fd_out[2];
    int n_chars = 0;
    int flag = 0;
    char query_string[14];
    bzero(query_string,sizeof(query_string));
    (void)strncpy(query_string, "QUERY_STRING=", strlen("QUERY_STRING="));
    query_string[13] = '\0';
    
    // (void)printf("Hello World: %d\n", query_string[13]);
    const char *executable_path_after_strtok;
    const char *parameters;

    if (strstr(executable_path, "?") == NULL) {
        flag = 0;
    } else {
        flag = 1;
    }

    executable_path_after_strtok = strtok((char *)executable_path, "?");


    if (flag == 0) {
        parameters = "";
    } else {
        parameters = strtok(NULL, "?");
    }
    char parameters_array[strlen(parameters) + 1];
    bzero(parameters_array,sizeof(parameters_array));
    if (parameters != NULL) {
        (void)strncpy(parameters_array, parameters, strlen(parameters));
    } else {
        (void)strncpy(parameters_array, "", strlen("") + 1);
    }
    char find_string[] = "/cgi-bin";
    char variable_executable_path[strlen(executable_path_after_strtok) + 1];
    bzero(variable_executable_path, sizeof(variable_executable_path));

    // variable_executable_path = executable_path_after_strtok;
    (void)strncpy(variable_executable_path, executable_path_after_strtok, strlen(executable_path_after_strtok));
    char *where_is_cgi = strstr(variable_executable_path, find_string);

    char resolved_path_starting_bak[PATH_MAX];
    bzero(resolved_path_starting_bak, sizeof(resolved_path_starting_bak));
   
    (void)strncpy(resolved_path_starting_bak, resolved_path_of_starting,
                  strlen(resolved_path_of_starting));
    

    if (where_is_cgi) {
        (void)strncat(resolved_path_of_starting,
                      where_is_cgi + strlen(find_string),
                      strlen(where_is_cgi + strlen(find_string)));
        (void)strncpy(where_is_cgi, resolved_path_of_starting,
                      strlen(resolved_path_of_starting));
        where_is_cgi[strlen(resolved_path_of_starting)] = '\0';
    }

    if ((get_real_path(variable_executable_path,
                       resolved_path_of_executable_path)) == 0) {
        send_error(404, socket_fd, is_valid_request, response, response_string);
    }
    

    char combined_query_string[strlen(query_string) + strlen(parameters_array)+1];
    bzero(combined_query_string, sizeof(combined_query_string));
    
    if (parameters_array == NULL) {
        
        (void)strncpy(parameters_array, "", strlen(""));
        (void)strncpy(combined_query_string, query_string, strlen(query_string));
        
        combined_query_string[strlen(query_string)] = '\0';
    } else {
        (void)strncpy(combined_query_string, query_string, strlen(query_string));
        (void)strlcat(combined_query_string, parameters_array, strlen(parameters_array) + strlen(combined_query_string) + 1);
        combined_query_string[strlen(parameters_array) + strlen(combined_query_string)] = '\0';
    }
  
    char *envp[] = {combined_query_string, 0};

    /* Check if file exsists and has executable permission with the respective
     * error codes*/
    if (access(resolved_path_of_executable_path, F_OK) == -1) {
        send_error(404, socket_fd, is_valid_request, response, response_string);
    }

    if (access(resolved_path_of_executable_path, X_OK)) {
        send_error(401, socket_fd, is_valid_request, response, response_string);
    }
     
    /* Starting on comparing */
    strncat(resolved_path_starting_bak, "/", strlen("/"));

    if (strstr(resolved_path_of_executable_path, resolved_path_starting_bak) ==
        NULL) {
        send_error(401, socket_fd, is_valid_request, response, response_string);
    }

    
    /* End of comparing code*/

    /*code for checking the paths given by the user with realpath*/

    /* Code for converting the given request path to realpath */

    sigset_t blockMask, origMask;
    struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;
    pid_t childPid;
    int status, savedErrno;
    /* Code for enviornment variable assigning*/
    int errno;

    /* Block SIGCHLD */
    if (sigemptyset(&blockMask) == -1) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    if (sigaddset(&blockMask, SIGCHLD) == -1) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    /* Ignore SIGINT and SIGQUIT */
    saIgnore.sa_handler = SIG_IGN;
    saIgnore.sa_flags = 0;

    if (sigemptyset(&saIgnore.sa_mask) == -1) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    if (sigaction(SIGINT, &saIgnore, &saOrigInt) == -1) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    if (sigaction(SIGQUIT, &saIgnore, &saOrigQuit) == -1) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    /** Creation of pipe failed, so Internal Server Error 500 */
    if (pipe(fd_out) < 0) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }
     
    /** Forking a new process */
    switch (childPid = fork()) {
        /* fork() failed */
        case -1:
            status = -1;
            perror("Could not create child process (fork() failed).");

            send_error(500, socket_fd, is_valid_request, response,
                       response_string);
            break; /* Carry on to reset signal attributes */

        case 0: /* Child: exec CGI */
            /* We ignore possible error returns because the only specified error
            is for a failed exec(), and because errors in these calls can't
            affect the caller of command() (which is a separate process) */

            /** Child, closing the read end */
            (void)close(fd_out[0]);

            if (dup2(fd_out[1], STDOUT_FILENO) != STDOUT_FILENO) {
                send_error(500, socket_fd, is_valid_request, response,
                           response_string);
                break;
            }

            saDefault.sa_handler = SIG_DFL;
            saDefault.sa_flags = 0;

            if (sigemptyset(&saDefault.sa_mask) == -1) {
                send_error(500, socket_fd, is_valid_request, response,
                           response_string);
            }

            if (saOrigInt.sa_handler != SIG_IGN) {
                if (sigaction(SIGINT, &saDefault, NULL) == -1) {
                    send_error(500, socket_fd, is_valid_request, response,
                               response_string);
                }
            }

            if (saOrigQuit.sa_handler != SIG_IGN) {
                if (sigaction(SIGQUIT, &saDefault, NULL) == -1) {
                    send_error(500, socket_fd, is_valid_request, response,
                               response_string);
                }
            }

            if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1) {
                send_error(500, socket_fd, is_valid_request, response,
                           response_string);
            }
            char *args[] = {
                "0",
                NULL}; /* each element represents a command line argument */
            char *env[] = {NULL}; /* leave the environment list null */

            (void)args;
            (void)env;

            (void)close(fd_out[1]);

         
            if (execve(resolved_path_of_executable_path, args, envp) == -1) {
                (void)printf("Execve() Not able to run this\n");
                send_error(500, socket_fd, is_valid_request, response,
                           response_string);
            }

            (void)printf("Execve() Not able to run this");

            if (!flags.d_flag) {
                _exit(127); /* We could not exec the shell */
            }
            break;
        default: /* Parent: wait for our child to terminate */
            /** Parent, closing the write end */
            (void)close(fd_out[1]);

            /*
            We must use waitpid() for this task; using wait() could
            inadvertently collect the status of one of the caller's other
            children
            */
            if (waitpid(childPid, &status, WNOHANG) < 0) {
                perror(
                    "Could not wait for child process; waitpid resulted in "
                    "error.");
                status = -1;
                break;
            }

            /** Check if the CGI program has terminated */
            if (WIFEXITED(status)) {
                /** CGI program exited, get its exit code */
                int exit_code = WEXITSTATUS(status);
                if (exit_code == 0) {
                    /** CGI program succeeded, send output as response */
                    response->status_code = 200;
                    send_headers(socket_fd, is_valid_request, response,
                                 response_string);

                    /** Sending the output of the child execve to the client. */
                    n_chars = read(fd_out[0], temp_buff, BUFSIZ);

                    if (n_chars < 0) {
                        close_connection(socket_fd);
                        break;
                    }

                    while (n_chars > 0) {
                        if (write(socket_fd, temp_buff, n_chars) < 0) {
                            close_connection(socket_fd);
                            break;
                        }
                        n_chars = read(fd_out[0], temp_buff, BUFSIZ);

                        if (n_chars < 0) {
                            close_connection(socket_fd);
                            break;
                        }
                    }
                    close_connection(socket_fd);
                } else {
                    /** CGI program failed, send error response */
                    send_error(500, socket_fd, is_valid_request, response,
                               response_string);
                }
            } else {
                /** CGI program has not terminated, read remaining data from the
                 * pipe */
                response->status_code = 200;
                send_headers(socket_fd, is_valid_request, response,
                             response_string);
                /** Sending the output of the child execve to the client. */
                n_chars = read(fd_out[0], temp_buff, BUFSIZ);

                if (n_chars < 0) {
                    close_connection(socket_fd);
                    break;
                }

                while (n_chars > 0) {
                    if (write(socket_fd, temp_buff, n_chars) < 0) {
                        close_connection(socket_fd);
                        break;
                    }
                    n_chars = read(fd_out[0], temp_buff, BUFSIZ);

                    if (n_chars < 0) {
                        close_connection(socket_fd);
                        break;
                    }
                }
                close_connection(socket_fd);
            }

            (void)close(fd_out[0]);
    }

    /* Unblock SIGCHLD, restore dispositions of SIGINT and SIGQUIT */

    savedErrno = errno;
    /* The following may change 'errno' */
    if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    if (sigaction(SIGINT, &saOrigInt, NULL) == -1) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    if (sigaction(SIGQUIT, &saOrigQuit, NULL) == -1) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
    }

    errno = savedErrno;

    return status;
}
