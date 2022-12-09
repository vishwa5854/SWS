#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include "handler.h"
#include "cgi.h"

#define BUFFER_SIZE 64
// TODO add all the code inside the while and send return to socket
// TODO check whether the file has permission to execute the file 
// TODO check whether the files are in the same parent directory 

int execute_file(const char *executable_path, int socket_fd, bool is_valid_request, RESPONSE *response, char *response_string)
{
    (void)socket_fd;
    char temp_buff[BUFSIZ];
    int fd_out[2];
    int n_chars = 0;
    
    const char* Query_String="QUERY_STRING=";
    const char* combined_query_string;
    const char* executable_path_after_strtok;
    const char* parameters;

    executable_path_after_strtok=strtok((char *)executable_path,"?");
    
    parameters=strtok(NULL,"?");
    
    if (parameters == NULL)
    {
        parameters="";
        if((combined_query_string = malloc(strlen(Query_String) * sizeof(char)))==NULL){
            send_error(500, socket_fd, is_valid_request, response, response_string);
        }
        combined_query_string = strncpy((char * restrict)combined_query_string, (char * restrict)Query_String,strlen(Query_String));
    }
    else{
         if ((combined_query_string=malloc(sizeof(char)*(strlen(Query_String)+strlen(parameters))))==NULL){
            send_error(500, socket_fd, is_valid_request, response, response_string);
        }
        strncpy((char * restrict)combined_query_string,(char * restrict)Query_String,strlen(Query_String));//TODO check for return values on the same
        strlcat((char * restrict)combined_query_string,(char * restrict)parameters,strlen(parameters)+strlen(combined_query_string)+1);//Rough POC//TODO check for return values on the same
    }
    char *envp[]={(char *)combined_query_string,0};

    free((void *)combined_query_string);

    /* Check if file exsists and has executable permission with the respective error codes*/
    if(access(executable_path_after_strtok,F_OK)==-1){
        send_error(404, socket_fd, is_valid_request, response, response_string);
    }
    if(access(executable_path_after_strtok,X_OK)){
        send_error(401, socket_fd, is_valid_request, response, response_string);
    }
    
    //end testing code



    sigset_t blockMask, origMask;
    // int flag=0;

    struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;
    pid_t childPid;
    int status, savedErrno;
    /* Code for enviornment variable assigning*/

    int errno;

    sigemptyset(&blockMask); /* Block SIGCHLD */
    sigaddset(&blockMask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &blockMask, &origMask);

    saIgnore.sa_handler = SIG_IGN; /* Ignore SIGINT and SIGQUIT */
    saIgnore.sa_flags = 0;
    sigemptyset(&saIgnore.sa_mask);
    sigaction(SIGINT, &saIgnore, &saOrigInt);
    sigaction(SIGQUIT, &saIgnore, &saOrigQuit);

    /** Creation of pipe failed, so Internal Server Error 500 */
    if (pipe(fd_out) < 0) {
        send_error(500, socket_fd, is_valid_request, response, response_string);
        return EXIT_FAILURE;
    }

    // Forking a new process
    switch (childPid = fork()) {
        case -1: /* fork() failed */
            status = -1;
            perror("Could not create child process (fork() failed).");

            send_error(500, socket_fd, is_valid_request, response, response_string);
            break; /* Carry on to reset signal attributes */

        case 0: /* Child: exec CGI */
            
            
            /* We ignore possible error returns because the only specified error
            is for a failed exec(), and because errors in these calls can't
            affect the caller of command() (which is a separate process) */

            /** Child, closing the read end */
            (void)close(fd_out[0]);

            if (dup2(fd_out[1], STDOUT_FILENO) != STDOUT_FILENO) {
                send_error(500, socket_fd, is_valid_request, response, response_string);
                break;
            }

            saDefault.sa_handler = SIG_DFL;
            saDefault.sa_flags = 0;
            sigemptyset(&saDefault.sa_mask);

            if (saOrigInt.sa_handler != SIG_IGN)
                sigaction(SIGINT, &saDefault, NULL);
            if (saOrigQuit.sa_handler != SIG_IGN)
                sigaction(SIGQUIT, &saDefault, NULL);
            sigprocmask(SIG_SETMASK, &origMask, NULL);
            char *args[] = {"0", NULL};	/* each element represents a command line argument */
            char *env[] = { NULL };	/* leave the environment list null */

            (void)args;
            (void)env;
            // execve("/bin/ls", args, env);
            // execl("/bin/sh", "sh", "-c", executable_path, (char *)NULL);
            // printf("\n%d\n",errno);
            // execve(executable_path,args,envp);//Working

            (void)close(fd_out[1]);

            if (execve(executable_path_after_strtok, args, envp) == -1) {
                printf("Execve() Not able to run this");
                send_error(500, socket_fd, is_valid_request, response, response_string);
            }

            puts("Below the execve call");

            printf("\n%d\n",errno);
            fprintf(stderr, "Value of errno: %d\n", errno);

            // if (execve(executable_path,(char *const*)NULL,envp)==-1){
            //     printf("Execve() Not able to run this");
            // }
            // printf("\n%d\n",errno);
            // fprintf(stderr, "Value of errno: %d\n", errno);

            printf("Execve() Not able to run this");

            _exit(127); /* We could not exec the shell */

        default: /* Parent: wait for our child to terminate */
            /** Parent, closing the write end */
            (void)close(fd_out[1]);
            

            /* 
            We must use waitpid() for this task; using wait() could inadvertently
            collect the status of one of the caller's other children 
            */
            if (waitpid(childPid, &status, WNOHANG) < 0) {
                perror("Could not wait for child process; waitpid resulted in error.");
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
                    send_headers(socket_fd, is_valid_request, response, response_string);

                    /** Sending the output of the child execve to the client. */
                    while ((n_chars = read(fd_out[0], temp_buff, BUFSIZ)) > 0) {
                        (void)write(socket_fd, temp_buff, n_chars);
                    }
                    close_connection(socket_fd);
                } else {
                    /** CGI program failed, send error response */
                    send_error(500, socket_fd, is_valid_request, response, response_string);
                }
            } else {
                /** CGI program has not terminated, read remaining data from the pipe */
                response->status_code = 200;
                send_headers(socket_fd, is_valid_request, response, response_string);
                /** Sending the output of the child execve to the client. */
                while ((n_chars = read(fd_out[0], temp_buff, BUFSIZ)) > 0) {
                    (void)write(socket_fd, temp_buff, n_chars);
                }
                close_connection(socket_fd);
            }

            (void)close(fd_out[0]);
    }

    /* Unblock SIGCHLD, restore dispositions of SIGINT and SIGQUIT */

    savedErrno = errno; /* The following may change 'errno' */
    sigprocmask(SIG_SETMASK, &origMask, NULL);
    sigaction(SIGINT, &saOrigInt, NULL);
    sigaction(SIGQUIT, &saOrigQuit, NULL);

    errno = savedErrno;

    return status;
}

// int data(){}