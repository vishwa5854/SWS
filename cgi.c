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

#include "cgi.h"

#define BUFFER_SIZE 64
// TODO add all the code inside the while and send return to socket
// TODO check whether the file has permission to execute the file 
// TODO check whether the files are in the same parent directory 

int execute_file( char *executable_path, int socket_fd)
{
    // char tempArgs;
    // strncpy(ex)
    // char *args[]={"/bin/sh", "sh", "-c",executable_path,NULL};
    // for(int i=0; i<2;i++){
    //     printf("%s",args[i]);
    // }
    sigset_t blockMask, origMask;
    struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;
    pid_t childPid;
    int status, savedErrno;
    char *envp={}

    sigemptyset(&blockMask); /* Block SIGCHLD */
    sigaddset(&blockMask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &blockMask, &origMask);

    saIgnore.sa_handler = SIG_IGN; /* Ignore SIGINT and SIGQUIT */
    saIgnore.sa_flags = 0;
    sigemptyset(&saIgnore.sa_mask);
    sigaction(SIGINT, &saIgnore, &saOrigInt);
    sigaction(SIGQUIT, &saIgnore, &saOrigQuit);

    // Forking a new process
    switch (childPid = fork())
    {
    case -1: /* fork() failed */
        status = -1;
        perror("Could not create child process (fork() failed).");
        break; /* Carry on to reset signal attributes */

    case 0: /* Child: exec CGI */
        // Duplicating the file descriptors onto the write-ends of the pipes
        if (dup2(socket_fd, STDOUT_FILENO) < 0)
        {
            perror("Could not duplicate STDOUT file descriptor to the pipe.");
            status = -1;
            break;
        }
        
        /* We ignore possible error returns because the only specified error
           is for a failed exec(), and because errors in these calls can't
           affect the caller of command() (which is a separate process) */

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
        (void)executable_path;
        execve(executable_path,args,env);
        printf("Execve() Not able to run this");
        _exit(127); /* We could not exec the shell */

    default: /* Parent: wait for our child to terminate */

        /* We must use waitpid() for this task; using wait() could inadvertently
           collect the status of one of the caller's other children */
        if (waitpid(childPid, &status, 0) < 0)
        {
            perror("Could not wait for child process; waitpid resulted in error.");
            status = -1;
            break;
        }
    }

    /* Unblock SIGCHLD, restore dispositions of SIGINT and SIGQUIT */

    savedErrno = errno; /* The following may change 'errno' */

    sigprocmask(SIG_SETMASK, &origMask, NULL);
    sigaction(SIGINT, &saOrigInt, NULL);
    sigaction(SIGQUIT, &saOrigQuit, NULL);

    errno = savedErrno;

    return status;
}
