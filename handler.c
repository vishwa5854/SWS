#include "handler.h"

void handleConnection(int fd, struct sockaddr_in6 client) {
    const char *rip;
    int rval;
    char claddr[INET6_ADDRSTRLEN];

    if ((rip = inet_ntop(PF_INET6, &(client.sin6_addr), claddr, INET6_ADDRSTRLEN)) == NULL) {
        perror("inet_ntop");
        rip = "unknown";
    } else {
        (void)printf("Client connection from %s!\n", rip);
    }

    do {
        char buf[BUFSIZ];
        bzero(buf, sizeof(buf));

        if ((rval = read(fd, buf, BUFSIZ)) < 0) {
            perror("reading stream message");
        } else if (rval == 0) {
            printf("Ending connection from %s.\n", rip);
        } else {
            printf("Client (%s) sent: %s", rip, buf);
        }
    } while (rval != 0);
    (void)close(fd);
    /* NOTREACHED */
}

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
        exit(EXIT_SUCCESS);
        /* NOTREACHED */
    }
    /* parent silently returns */
}