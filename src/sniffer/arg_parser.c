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
#include <limits.h>

#ifdef DEBUG
#include <ifaddrs.h>
#endif

#include "arg_parser.h"
#include "helpers.h"


static int valid_ip( char* ip , uint8_t* res )
{
    struct in6_addr addr6;
    struct in_addr addr4;

    if ( inet_pton( AF_INET6, ip, &addr6 ) > 0 ) {
        memcpy( res, &addr6, MAX_ADDR_SZ );
        return 1;
    }
    else if ( inet_pton( AF_INET, ip, &addr4 ) > 0 ) {
        memcpy( res, &addr4, sizeof(uint32_t) );
        return 1;
    }
    else {
        return 0;
    }
}

static int valid_port( char* port )
{
    for ( size_t i = 0; i < strlen(port); i++ )
    {
        if ( !isdigit(port[i]) ) {
          return 0;
        }
    }
    if ( strtol(port, NULL, 10) > USHRT_MAX ) {
        return 0;
    }

    return 1;
}

static int valid_int( char* interface )
{
    int idx;
    if ( !(idx = if_nametoindex(interface)) ) {
        return 0;
    }
    else {
        return idx;
    }
}

int parse_args( int argc, char *argv[], parsed_args_t *args ) 
{

    if ( argc > 11 ) {
        WRONG_OPT_RETURN("Too many options!! Try --help to get info on usage\n");
    }

    memset( args, 0, sizeof(parsed_args_t) );

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
              if ( !(args->interface = valid_int(optarg)) ) {
                  WRONG_OPT_RETURN("Interface does not exist!\n");
              }
              break;
          case IPSRC:
              if ( !valid_ip(optarg, args->ip_source) ) {
                  WRONG_OPT_RETURN("Wrong value of required ip source address!\n");
              }
              break;
          case IPDEST:
              if ( !valid_ip(optarg, args->ip_dest) ) {
                  WRONG_OPT_RETURN("Wrong value of reqired ip dest address!\n");
              }
              break;
          case PORTSRC:
              if ( valid_port(optarg) ) {
                  args->port_source = atoi(optarg);
              }
              else {
                  WRONG_OPT_RETURN("Wrong value of reqired source port!\n");
              }
              break;
          case PORTDEST:
              if ( valid_port(optarg) ) {
                  args->port_dest = atoi(optarg);
              }
              else {
                  WRONG_OPT_RETURN("Wrong value of reqired dest port!\n");
              }
              break;
          case 'h':
              printf("Usage: sniffer [OPTION]... [ADDRESS/PORT]...\n"
                     "Collect statistic on incoming udp packages\n"
                     "pass it to representer\n\n"
                     "--interface [NAME]  defines interface name\n"
                     "                    NAME can be a string that represents existing interface name\n"
                     "--ipsrc [ADDRESS]   defines source ip address for filtering\n"
                     "                    ADDRESS can be ipv4 or ipv6 address in standard format\n"
                     "--ipdest [ADDRESS]  defines destination ip address for filtering\n"
                     "                    ADDRESS can be ipv4 or ipv6 address in standard format\n"
                     "--portsrc [PORT]    defines source port number for filtering\n"
                     "                    PORT can be any positive number below 65536\n"
                     "--portdest [PORT]   defines destination port number for filtering\n"
                     "                    PORT can be any positive number below 65536\n"
                    );
              return -1;
          case '?':
              WRONG_OPT_RETURN("Try --help\n");
        }
    }
    
    // todo:fix check_arg_parse 0 from if_nametoindex
    if ( !args->interface ) {
        WRONG_OPT_RETURN("You should specify interface!\n");
    }

    return 1;
}


#ifdef DEBUG
int check_valid_ip(void)
{
    char* ip = "1.1.1.1";
    uint8_t buf[100];
    if ( valid_ip(ip, buf) < 0 ) {
        return 0;
    }
    return 1;
}

int check_valid_port(void)
{
    char* port = "1000";
    if ( !valid_port(port) ) {
        return 0;
    }
    port = "abcd";
    if ( valid_port(port) ) {
        return 0;
    }
    port = "31231231231231232131232";
    if ( valid_port(port) ) {
        return 0;
    }
    return 1;
}

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

#endif
