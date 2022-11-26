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

int execute_file(const char *string, char *outbuf, int outlen, char *errbuf, int errlen,char *fileName)
{
    
    
    char *tempDirectory="/tmp/";

    char *locationoferrtempfile;
    int temperrfd=0;
    char *temperrTempFile="err";//TODO check if this what is required from my side

    char *locationoftempfile;
    int tempfd=0;
    (void)locationoftempfile;
    

    if (string == NULL) /* Is a shell available? */
    {
        return execute_file(":", outbuf, outlen, errbuf, errlen,fileName);
    }
    
    locationoftempfile=(char *)malloc(sizeof(char)*(sizeof(tempDirectory)+sizeof(fileName)));
    locationoftempfile=strcat(locationoftempfile,tempDirectory);
    locationoftempfile=strcat(locationoftempfile,fileName);/* Safe file location to store buffer*/
    printf("\n %s \n",locationoftempfile);

    if ((tempfd=open(locationoftempfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR  ))<0){
        perror("Could Not create the tmp file");
        return -1;
    }
    
    printf("\n%d\n",tempfd);

    locationoferrtempfile=(char *)malloc(sizeof(char)*(sizeof(tempDirectory)+sizeof(temperrTempFile)+sizeof(fileName)));
    locationoferrtempfile=strcat(locationoferrtempfile,tempDirectory);
    locationoferrtempfile=strcat(locationoferrtempfile,temperrTempFile);
    locationoferrtempfile=strcat(locationoferrtempfile,fileName);
    printf("\n %s \n",locationoferrtempfile);

    if ((temperrfd=open(locationoferrtempfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR  ))<0){
        perror("Could Not create the tmp file");
        return -1;
    }

    sigset_t blockMask, origMask;
    struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;
    pid_t childPid;
    int status, savedErrno;

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
        if (dup2(tempfd, STDOUT_FILENO) < 0)
        {
            perror("Could not duplicate STDOUT file descriptor to the pipe.");
            status = -1;
            break;
        }
        if (dup2(temperrfd, STDERR_FILENO) < 0)
        {
            perror("Could not duplicate STDERR file descriptor to the pipe.");
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

        execl("/bin/sh", "sh", "-c", string, (char *)NULL);
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




/* main function to test out the functionality using ls */
int main(int argc,char **argv)// send the filename 
{
    char outbuf[BUFFER_SIZE], errbuf[BUFFER_SIZE];
    char *locationofexe;
    char* baseDirectory="/";//TODO this has to be changed in the integration
    char *tempFileName="siddharth"+'\0';
    (void)argc;//TODO Remove
    // char *A[]={"Data","is","mine",'\0'};

    
    locationofexe=(char *)malloc(sizeof(char)*(sizeof(argv[1])+sizeof(baseDirectory)+sizeof('\0')));
    locationofexe=strcat(locationofexe,baseDirectory);
    argv[1]=argv[1]+'\0';
    locationofexe=strcat(locationofexe,argv[1]);// TODO convert to strlcat
    // printf("%s",locationofexe);//TODO Remove

   if (execute_file(locationofexe, outbuf, BUFFER_SIZE - 1, errbuf, BUFFER_SIZE - 1,tempFileName) < 0)   
    {
        perror("command function failed.");
        return -1;
    }
    

    outbuf[BUFFER_SIZE - 1] = '\0';
    errbuf[BUFFER_SIZE - 1] = '\0';

    // printf("Got stdout: \n%s", outbuf);
    // printf("Got stderr: \n%s", errbuf);

    return 0;
}