/*
* Sniffer is prrogramm that can collect data about incoming udp packages
* Copyright (C) 2024  Vladimir Mimikin
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "arg_parser.h"
#include "helpers.h"


static int is_valid_ip( char* ip, char* binary_res )
{
    char buf[100];
    if ( inet_pton( AF_INET6, ip, buf ) > 0 ||
         inet_pton( AF_INET, ip, buf ) > 0 ) {
        strcpy( binary_res, buf );
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

static int int_exist( const char* interface )
{
    struct ifreq rq;
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (NOTVALIDSOCKET(sock)) {
        CLOSESOCKET(sock);
        perror("Can't create socket!");
        return 0;
    }

    memset(&rq, 0, sizeof(struct ifreq));
    strcpy(rq.ifr_name, interface);
    if ( ioctl(sock, SIOCGIFFLAGS, &rq ) < 0) {
        CLOSESOCKET(sock);
        return 0;
    }

    if ( rq.ifr_flags && IFF_UP ) {
        return 1;
    }
    else {
        return 0;
    }
}

int parse_args( int argc, char *argv[], parsed_args_t *args ) 
{

    if ( argc > 9 ) {
        WRONG_OPT_RETURN("Too many options!! Try --help to get info on usage\n");
    }

    args->ip_dest = NULL;
    args->ip_source = NULL;
    args->port_dest = 0;
    args->port_source = 0;

    enum opt_names {
        IPSRC,
        IPDEST,
        PORTSRC,
        PORTDEST
    };

    const struct option options[] = {
        { .name = "interface", .has_arg = required_argument, .flag = 0, .val = 'i' },
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
          case 'i':
              if ( int_exist(optarg) ) {
                  args->interface = optarg;
              }
              else {
                  WRONG_OPT_RETURN("Interface does not exist!\n");
              }
              break;
          case IPSRC:
              if ( !is_valid_ip(optarg, args->ip_source) ) {
                  WRONG_OPT_RETURN("Wrong value of required ip source address!\n");
              }
              break;
          case IPDEST:
              if ( !is_valid_ip(optarg, args->ip_dest) ) {
                  WRONG_OPT_RETURN("Wrong value of reqired ip dest address!\n");
              }
              break;
          case PORTSRC:
              if ( is_valid_port(optarg) ) {
                  args->port_source = atol(optarg);
              }
              else {
                  WRONG_OPT_RETURN("Wrong value of reqired source port!\n");
              }
              break;
          case PORTDEST:
              if ( is_valid_port(optarg) ) {
                  args->port_dest = atol(optarg);
              }
              else {
                  WRONG_OPT_RETURN("Wrong value of reqired dest port!\n");
              }
              break;
          case 'h':
              printf("Usage: sniffer [OPTION]... [ADDRESS]...\n"
                     "Collect statistic on incoming udp packages\n"
                     "pass it to representer\n\n"
                     "--interface [NAME]  defines interface name\n"
                     "                    NAME can be a string that represents existing interface name\n"
                     "--ipsrc [ADDRESS]   defines source ip address for filtering\n"
                     "                    ADDRESS can be ipv4 or ipv6 address in standard format\n"
                     "--ipdest [ADDRESS]  defines destination ip address for filtering\n"
                     "                    ADDRESS can be ipv4 or ipv6 address in standard format\n"
                     "--ipsrc [PORT]      defines source port number for filtering\n"
                     "                    PORT can be any positive number\n"
                     "--ipsrc [PORT]      defines source ip address for filtering\n"
                     "                    PORT can be any positive number\n"
                    );
              return 0;
          case '?':
              WRONG_OPT_RETURN("Try --help\n");
        }
    }

    if ( !args->interface ) {
        WRONG_OPT_RETURN("You should specify interface!\n");
    }

    return 1;
}
