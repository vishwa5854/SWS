#include <stdlib.h>
#include <unistd.h>
#include "flags.h"

int main(int argc, char **argv) {

    // Create new flags struct, initializing all flags to 0
    struct flags_struct flags = {0};
    
    // default flags

    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (getopt_ret) {
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

    // running as a daemon should be its own func, due to -d
    // logging should be its own func, due to -l

    return EXIT_SUCCESS;
}