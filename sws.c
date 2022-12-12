#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "flags.h"
#include "handler.h"
#include "structures.h"

/** listen(2) states backlog silently limited to 128 */
#define BACKLOG 128
#define SLEEP_FOR 5

int isPresentOrNot(const char *path_name) {
    struct stat st;

    if (stat((const char *)path_name, &st) != 0) {
        (void)printf("\n Error getting information, of directory \n");
        return (-1);
    }

    if (st.st_mode & S_IFDIR) {
        (void)printf("\n Is a directory \n");
        return (1);
    } else {
        (void)printf(
            "\n Is regular File, please give a directory as an option to -c  "
            "\n");
        return (-1);
    }
}

int createSocket(int port, struct flags_struct flags) {
    int sock;
    socklen_t length;
    struct sockaddr_in6 server;

    memset(&server, 0, sizeof(server));

    if ((sock = socket(PF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server.sin6_family = PF_INET6;
    /** if i flag, support single IP only instead of multiple IPs */
    if (flags.i_flag) {
        char *given_addr = flags.addr_arg;
        struct in6_addr ip_result;
        int inet_pton_result;
        int inet6_pton_result;

        /** if string passed is IPV4, append an IPV6 prefix to it */
        if ((inet_pton_result = inet_pton(PF_INET, given_addr, &ip_result)) ==
            1) {
            char v4tov6_prefix[IPV6_MAXSTRLEN] = "::FFFF:";
            (void)strncat(v4tov6_prefix, given_addr, IPV6_MAXSTRLEN);
            (void)strncpy(given_addr, v4tov6_prefix, IPV6_MAXSTRLEN);
        } else if (inet_pton_result < 0) {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }

        if ((inet6_pton_result = inet_pton(PF_INET6, given_addr, &ip_result)) ==
            1) {
            server.sin6_addr = ip_result;
        } else if (inet6_pton_result == 0) {
            (void)printf("inet_pton: string not parsable!\n");
            exit(EXIT_FAILURE);
        } else {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }
    } else {
        server.sin6_addr = in6addr_any;
    }

    server.sin6_port = port;

    int off = 0;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&off, sizeof(off)) <
        0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) != 0) {
        perror("binding stream socket");
        exit(EXIT_FAILURE);
    }
    length = sizeof(server);

    if (getsockname(sock, (struct sockaddr *)&server, &length) != 0) {
        perror("getting socket name");
        exit(EXIT_FAILURE);
    }
    (void)printf("Socket has port #%d\n", ntohs(server.sin6_port));

    if (listen(sock, BACKLOG) < 0) {
        perror("listening");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void printUsage(char *progName) {
    (void)fprintf(
        stderr,
        "usage: %s [-dh] [-c dir] [-i address] [-l file] [-p port] dir\n",
        progName);
}

void reap() { (void)wait(NULL); }

/** This code has been referenced from CS631 APUE class notes apue-code/09 */
void selectSocket(int socket, struct flags_struct flags) {
    fd_set ready;
    struct timeval to;
    int select_return;
    FD_ZERO(&ready);
    FD_SET(socket, &ready);
    to.tv_sec = SLEEP_FOR;
    to.tv_usec = 0;

    select_return = select(socket + 1, &ready, 0, 0, &to);

    if (select_return < 0) {
        if (errno != EINTR) {
            perror("select");
        }
        return;
    }

    if (FD_ISSET(socket, &ready)) {
        handleSocket(socket, flags);
    }
}

int main(int argc, char **argv) {
    if (signal(SIGCHLD, reap) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    /** Create new flags struct, initializing all flags to 0 */
    struct flags_struct flags = {0};

    /** for each optarg case, we copy the exact data needed, then explicitly add
     * null byte */
    int opt;

    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (opt) {
            case 'c':
                (void)strncpy(flags.cdi_dir_arg, optarg,
                              strnlen(optarg, PATH_MAX));
                flags.cdi_dir_arg[strnlen(optarg, PATH_MAX)] = '\0';
                flags.c_flag = 1;
                int temp;

                if ((temp = isPresentOrNot(flags.cdi_dir_arg)) != 1) {
                    perror("-c option");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
                flags.d_flag = 1;
                break;
            case 'h':
                printUsage(argv[0]);
                return EXIT_SUCCESS;
            case 'i':
                (void)strncpy(flags.addr_arg, optarg,
                              strnlen(optarg, IPV6_MAXSTRLEN));
                flags.addr_arg[strnlen(optarg, IPV6_MAXSTRLEN)] = '\0';
                flags.i_flag = 1;
                break;
            case 'l':
                (void)strncpy(flags.log_file_arg, optarg,
                              strnlen(optarg, PATH_MAX));
                flags.log_file_arg[strnlen(optarg, PATH_MAX)] = '\0';
                flags.l_flag = 1;
                break;
            case 'p':
                (void)strncpy(flags.port_arg, optarg,
                              strnlen(optarg, PORT_MAXSTRLEN));
                flags.port_arg[strnlen(optarg, PORT_MAXSTRLEN)] = '\0';
                flags.p_flag = 1;
                break;
            case '?':
                return EXIT_FAILURE;
        }
    }

    /** This effectively removes flags from argc and argv. */
    char *executable_name = argv[0];
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        printUsage(executable_name);
        return EXIT_FAILURE;
    }

    (void)strncpy(flags.argument_path, argv[0], strlen(argv[0]));
    (void)strncpy(flags.working_dir, argv[0], strlen(argv[0]));
    flags.argument_path[strlen(argv[0])] = '\0';

    if (!flags.p_flag) {
        /** If no -p flag provided, set port to 8080 by default */
        (void)strncpy(flags.port_arg, DEFAULT_PORT,
                      strnlen(DEFAULT_PORT, PORT_MAXSTRLEN));
        flags.port_arg[PORT_MAXSTRLEN + 1] = '\0';
        flags.p_flag = 1;
    }

    int socket;

    if (flags.p_flag) {
        int input_int = atoi(flags.port_arg);

        if ((input_int < 0) || (input_int > 65535)) {
            (void)fprintf(
                stderr,
                "%s: Port number must be an int between 0 and 65,535.\n",
                executable_name);
            exit(EXIT_FAILURE);
        }
        socket = createSocket(htons(input_int), flags);
    } else {
        socket = createSocket(0, flags);
    }

    if (!flags.d_flag) {
        int daemon_ret;
        if ((daemon_ret = daemon(0, 0)) == -1) {
            perror("daemon");
            return EXIT_FAILURE;
        }
        /** Our process is now a daemon and continues. */
    }

    while (1) {
        selectSocket(socket, flags);
    }

    /** Until and unless the server dies, this code is never reached. */
    return errno;
}