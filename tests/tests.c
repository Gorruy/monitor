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
#include <ifaddrs.h>

#include "arg_parser.h"
#include "sniffer.h"
#include "helpers.h"

#define NUMBER_OF_TEST_RUNS 1000000

int check_arg_parse(void)
{
    parsed_args_t args;
    struct ifaddrs *addrs,*tmp;

    int argc = 5;

    getifaddrs(&addrs);
    tmp = addrs;
    
    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET) {
            break;
        }
        tmp = tmp->ifa_next;
    }

    char *argv[5] = { "prname", "--interface", tmp->ifa_name, "--ipdest", "1.1.1.1" };
    
    freeifaddrs(addrs);

    int res = parse_args( argc, argv, &args );
    if ( res < 0 ) {
        return 0;
    }

    return 1;
}

int check_valid_ip(void)
{
    char* ip = "1.1.1.1";
    char buf[100];
    if ( valid_ip(ip, buf) < 0 ) {
        return 0;
    }
    return 1;
}

int check_valid_port(void)
{
    char* port = "1000";
    if ( valid_port(port) < 0 ) {
        return 0;
    }
    return 1;
}

int loadtest(int argc, char* argv[] )
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
    addr.sin_port = htons(args.port_dest);
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

    return 1;
}

int main(int argc, char* argv[] )
{
    if ( !check_arg_parse() ) {
        ERROR_EXIT("Error in arg_parse function!");
    }
    if ( !check_valid_ip() ) {
        ERROR_EXIT("Error in valid_ip function!");
    }
    if ( !check_valid_port() ) {
        ERROR_EXIT("Error in valid_port function!");
    }

#ifdef LOAD
    loadtest(argc, argv);
#endif

    return 0;
}