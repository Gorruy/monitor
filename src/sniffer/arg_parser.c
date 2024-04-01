#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "arg_parser.h"

#define ERROR_EXIT(message) do { \
  perror(message); \
  exit(EXIT_FAILURE); \
} while(0)


static int is_valid_ip( char* ip )
{
    char buf[100];
    if ( inet_pton( AF_INET6, ip, buf ) > 0 ||
         inet_pton( AF_INET, ip, buf ) > 0 ) {
        return 1;
    }
    else {
        return 0;
    }
}

static int is_valid_port( char* port )
{
    for ( size_t i = 0; i < strlen(port); i++ )
    {
        if ( !isdigit(port[i]) ) {
          return 0;
        }
    }

    return 1;
}

parsed_args_t parse_args( int argc, char* argv[] ) 
{
    if ( argc > 5 ) {
        ERROR_EXIT("Too many options!! Try --help to get info on usage");
    }

    parsed_args_t args = {
        .ip_dest = NULL,
        .ip_source = NULL,
        .port_dest = NULL,
        .port_source = NULL 
    };

    enum opt_names {
        IPSRC,
        IPDEST,
        PORTSRC,
        PORTDEST
    };

    const struct option options[] = {
        { .name = "ipsrc", .has_arg = required_argument, .flag = 0, .val = IPSRC },
        { .name = "ipdest", .has_arg = required_argument, .flag = 0, .val = IPDEST },
        { .name = "portsrc", .has_arg = required_argument, .flag = 0, .val = PORTSRC },
        { .name = "portdest", .has_arg = required_argument, .flag = 0, .val = PORTDEST },
        { .name = "help", .has_arg = no_argument, .flag = 0, .val = 'h' },
        { .name = NULL, .has_arg = 0, .flag = NULL, .val = 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "1:2:3:4:", options, NULL)) != -1 ) {
        switch (opt) {
          case IPSRC:
              if ( is_valid_ip(optarg) ) {
                  args.ip_source = optarg;
                  break;
              }
              else {
                  ERROR_EXIT("Wrong value of reqired ip source address!\n");
              }
          case IPDEST:
              if ( is_valid_ip(optarg) ) {
                  args.ip_dest = optarg;
                  break;
              }
              else {
                  ERROR_EXIT("Wrong value of reqired ip dest address!\n");
              }
          case PORTSRC:
              if ( is_valid_port(optarg) ) {
                  args.port_source = optarg;
                  break;
              }
              else {
                  ERROR_EXIT("Wrong value of reqired source port value!\n");
              }
          case PORTDEST:
              if ( is_valid_port(optarg) ) {
                  args.port_dest = optarg;
                  break;
              }
              else {
                  ERROR_EXIT("Wrong value of reqired dest port value!\n");
              }
          case 'h':
              printf("Usage: sniffer [OPTION]... [ADDRESS]...\n"
                     "Collect statistic on incoming udp packages\n"
                     "pass it to representer\n\n"
                     "--ipsrc [ADDRESS]   defines source ip address for filtering\n"
                     "                    ADDRESS can be ipv4 or ipv6 address in standard format\n"
                     "--ipdest [ADDRESS]  defines destination ip address for filtering\n"
                     "                    ADDRESS can be ipv4 or ipv6 address in standard format\n"
                     "--ipsrc [PORT]      defines source port number for filtering\n"
                     "                    PORT can be any positive number\n"
                     "--ipsrc [PORT]      defines source ip address for filtering\n"
                     "                    PORT can be any positive number\n"
                    );
              exit(EXIT_SUCCESS);
        }
    }

    return args;
}
