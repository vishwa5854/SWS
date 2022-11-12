#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include "flags.h"

#define BACKLOG 5

// creates a socket. returns descriptor referencing the socket, or -1 on error.
int
createSocket(void)
{
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
	server.sin6_port = 0;
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

int main(int argc, char **argv) {

    // Create new flags struct, initializing all flags to 0
    struct flags_struct flags = {0};

    (void) flags;
    
    // default flags
    int opt;
    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (opt) {
            case 'c':
                flags.c_flag = 1;
                break;
            case 'd':
                flags.d_flag = 1;
                break;
            case 'h':
                flags.h_flag = 1;
                break;
            case 'i':
                flags.i_flag = 1;
                break;
            case 'l':
                flags.l_flag = 1;
                break;
            case 'p':
                flags.p_flag = 1;
                break;
            case '?':
                return EXIT_FAILURE;
        }
    }
    // This effectively removes flags from argc and argv.
    argc -= optind;
    argv += optind;

    // running as a daemon should be its own func, due to -d
    // logging should be its own func, due to -l

    createSocket();

    // we want to use fork to create any number of sockets to handle connections
    // using child processes!

    return EXIT_SUCCESS;
}