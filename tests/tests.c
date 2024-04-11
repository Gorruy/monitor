#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "arg_parser.h"
#include "helpers.h"

#define NUMBER_OF_TEST_RUNS 100000


int main(int argc, char* argv[] )
{
    struct sockaddr_in addr;
    parsed_args_t args;
    parse_args(argc, argv, &args);

    char *buf = (char*)malloc(USHRT_MAX + 1);

    SOCKET br_sock = socket( AF_INET, SOCK_DGRAM, 0);

    if ( br_sock < 0 ) {
        perror("Error in socket creation!");
        exit(EXIT_FAILURE);
    }

    int broadcast_enable = 1;
    int set_res = setsockopt(br_sock, 
                             SOL_SOCKET, 
                             SO_BROADCAST,
                             &broadcast_enable, 
                             sizeof(broadcast_enable));
    if (set_res) {
        perror("Error in broadcast socket creation\n");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = args.port_dest;
    addr.sin_addr.s_addr = *(uint32_t*)args.ip_dest;

    for ( int i = 0; i < NUMBER_OF_TEST_RUNS; i++ ) {
        if ( sendto(br_sock, 
                    buf, 
                    USHRT_MAX - 66, 
                    0, 
                    (struct sockaddr*)&addr, 
                    sizeof(addr)) < 0) {
            perror("Cant send one packet!\n");
        }
    }

    return 0;
}