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
    int readPipeBuf;
    int readPipeErrBuf;


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

    (void)readPipeErrBuf;
    (void) readPipeBuf;
    (void) temperrfd;
    
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

    int stdout_pipe[2];
    int stderr_pipe[2];
    sigset_t blockMask, origMask;
    struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;
    pid_t childPid;
    int status, savedErrno;
    int stdout_index = 0;
    int stderr_index = 0;

    // Creating the pipes
    if (pipe(stdout_pipe))
    {
        perror("Could not create STDOUT pipe");
        return -1;
    }
    if (pipe(stderr_pipe))
    {
        perror("Could not create STDERR pipe");
        return -1;
    }

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
        // Closing one end of the pipes
        if (close(stdout_pipe[0]))
        {
            perror("Could not close the STDOUT pipe.");
            status = -1;
            break;
        }
        if (close(stderr_pipe[0]))
        {
            perror("Could not close the STDERR pipe.");
            status = -1;
            break;
        }

        // Duplicating the file descriptors onto the write-ends of the pipes
        if (dup2(stdout_pipe[1], STDOUT_FILENO) < 0)
        {
            perror("Could not duplicate STDOUT file descriptor to the pipe.");
            status = -1;
            break;
        }
        if (dup2(stderr_pipe[1], STDERR_FILENO) < 0)
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
        int num_elements_read;
        char buffer[BUFFER_SIZE];

        // Reading from the pipes
        if (close(stdout_pipe[1]) < 0)
        {
            perror("Could not close the STDOUT pipe.");
            status = -1;
            break;
        }
        if (close(stderr_pipe[1]) < 0)
        {
            perror("Could not close the STDERR pipe.");
            status = -1;
            break;
        }

        // Ensuring outbuf is not already full
        //TODO remove need to play here 

        while((readPipeBuf=read(stdout_pipe[0],buffer, BUFFER_SIZE))!=-1 && readPipeBuf !=0 ){
                //TODO REMOVE printf("read data value %d",read_data);
                printf("Writing\n");
                write(tempfd,buffer,readPipeBuf);// read data may contain
        }
        
        while((readPipeErrBuf=read(stderr_pipe[0],buffer, BUFFER_SIZE))!=-1 && readPipeErrBuf !=0){
            printf("Writing\n");
            write(temperrfd,buffer,readPipeErrBuf);// read data may contain
        }

        close(tempfd);
        close(temperrfd);

        // if (stdout_index < outlen)
        // {
        //     while ((num_elements_read = (read(stdout_pipe[0], buffer, BUFFER_SIZE))) > 0)
        //     {
        //         if (num_elements_read + stdout_index > outlen)
        //         {
        //             num_elements_read = outlen - stdout_index;
        //         }
        //         for (int i = 0; i < num_elements_read && buffer[i] != '\0'; ++i, ++stdout_index)
        //         {
        //             outbuf[stdout_index] = buffer[i];
        //         }
        //     }
        //     // Null terminating if space permits
        //     if (stdout_index < outlen)
        //     {
        //         outbuf[stdout_index] = '\0';
        //     }
        // }
        // // Ensuring outbuf is not already full
        // if (stderr_index < errlen)
        // {
        //     while ((num_elements_read = (read(stderr_pipe[0], buffer, BUFFER_SIZE))) > 0)
        //     {
        //         if (num_elements_read + stderr_index > errlen)
        //         {
        //             num_elements_read = errlen - stderr_index;
        //         }
        //         for (int i = 0; i < num_elements_read && buffer[i] != '\0'; ++i, ++stderr_index)
        //         {
        //             errbuf[stderr_index] = buffer[i];
        //         }
        //     }
        //     // Null terminating if space permits
        //     if (stderr_index < errlen)
        //     {
        //         errbuf[stderr_index] = '\0';
        //     }
        // }
        break;
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


    locationofexe=(char *)malloc(sizeof(char)*(sizeof(argv[1])+sizeof(baseDirectory)+sizeof('\0')));
    locationofexe=strcat(locationofexe,baseDirectory);
    argv[1]=argv[1]+'\0';
    locationofexe=strcat(locationofexe,argv[1]);// TODO convert to strlcat
    printf("%s",locationofexe);//TODO Remove
  
    

   if (execute_file(locationofexe, outbuf, BUFFER_SIZE - 1, errbuf, BUFFER_SIZE - 1,tempFileName) < 0)   
    {
        perror("command function failed.");
        return -1;
    }
    

    outbuf[BUFFER_SIZE - 1] = '\0';
    errbuf[BUFFER_SIZE - 1] = '\0';

    printf("Got stdout: \n%s", outbuf);
    printf("Got stderr: \n%s", errbuf);

    return 0;
}