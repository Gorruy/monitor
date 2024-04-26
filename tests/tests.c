/*
* Sniffer is prrogramm that can collect data about incoming udp packages
* Copyright (C) 2024  Vladimir Mimikin <vladimirmimikin@gmail.com>
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
#include "sniffer.h"
#include "helpers.h"

#define NUMBER_OF_TEST_RUNS 10000000

volatile int break_signal;

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

