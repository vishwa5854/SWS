#include <errno.h>
#include "handler.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "flags.h"

// backlog used for listen, maybe change from 5? see listen(2)
#define BACKLOG 5
#define SLEEP_FOR 5

int createSocket(int port) {
	int sock;
	socklen_t length;
	struct sockaddr_in6 server;

	memset(&server, 0, sizeof(server));

	if ((sock = socket(PF_INET6, SOCK_STREAM, 0)) < 0) {
		perror("opening stream socket");
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
	server.sin6_family = PF_INET6;
	server.sin6_addr = in6addr_any;
	server.sin6_port = port;

	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) != 0) {
		perror("binding stream socket");
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
	length = sizeof(server);

	if (getsockname(sock, (struct sockaddr *)&server, &length) != 0) {
		perror("getting socket name");
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
	(void)printf("Socket has port #%d\n", ntohs(server.sin6_port));

	if (listen(sock, BACKLOG) < 0) {
		perror("listening");
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
	return sock;
}

void printUsage(char* progName) {
    fprintf(stderr, "usage: %s [-dh] [-c dir] [-i address] [-l file] [-p port] dir\n", progName);
}

int parse_args(int argc, char **argv) {
    // Create new flags struct, initializing all flags to 0
    struct flags_struct flags = {0};

    // just so we use the flags struct for now, and dont get compilation error
    (void) flags;

    // defaults are to listen on all ipv4 and ipv6 addresses, and port 8080
    // use INADDR_ANY to listen for all available ips
    strncpy(flags.port_arg, "8080", 5);

    
    // for each optarg case, we copy the exact data needed, then explicitly add null byte
    int opt;

    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (opt) {
            case 'c':
                strncpy(flags.cdi_dir_arg, optarg, PATH_MAX);
                flags.cdi_dir_arg[PATH_MAX] = '\0';
                flags.c_flag = 1;
                break;
            case 'd':
                flags.d_flag = 1;
                break;
            case 'h':
                printUsage(argv[0]);
                return EXIT_SUCCESS;
            case 'i':
                strncpy(flags.addr_arg, optarg, 45);
                flags.addr_arg[45] = '\0';
                flags.i_flag = 1;
                break;
            case 'l':
                strncpy(flags.log_file_arg, optarg, PATH_MAX);
                flags.log_file_arg[PATH_MAX] = '\0';
                flags.l_flag = 1;
                break;
            case 'p':
                strncpy(flags.port_arg, optarg, 6);
                flags.port_arg[5] = '\0';
                flags.p_flag = 1;
                break;
            case '?':
                return EXIT_FAILURE;
        }
    }
    // This effectively removes flags from argc and argv.
    argc -= optind;
    argv += optind;

    
    /*
    if (!flags.d_flag) {
        int daemon_ret;
        if ((daemon_ret = daemon(0, 0)) == -1){
            perror("creating daemon");
		    return EXIT_FAILURE;
        }
        // our process is now a daemon.

        // here we fork and handle any number of connections
        
    } else {
        // debug mode here
        // accept one connection at a time, log to stdout
    }
    */

    // make sure to properly clean up before exiting!

    return EXIT_SUCCESS;
}

void reap() {
    wait(NULL);
}

int main(int argc, char **argv) {
    if (signal(SIGCHLD, reap) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
        /* NOTREACHED */
    }

    parse_args(argc, argv);

    /** TODO: replace the zero here with the user specified port from the parsed args @lucas */
    int socket = createSocket(0);

    /** This code has been referenced from CS631 APUE class notes apue-code/09 */
    while (1) {
        fd_set ready;
        struct timeval to;
        FD_ZERO(&ready);
        FD_SET(socket, &ready);
        to.tv_sec = SLEEP_FOR;
        to.tv_usec = 0;

        if (select(socket + 1, &ready, 0, 0, &to) < 0) {
            if (errno != EINTR) {
                perror("select");
            }
            continue;
        }

        if (FD_ISSET(socket, &ready)) {
            handleSocket(socket);
        }
    }
    
    return errno;
}