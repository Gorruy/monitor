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

#ifndef APRS
#define APRS

#include <net/if.h>

#include "helpers.h"

typedef struct parsed_args {
    unsigned interface;
    uint8_t ip_dest[MAX_ADDR_SZ];
    uint8_t ip_source[MAX_ADDR_SZ];
    uint16_t port_dest;
    uint16_t port_source;
} parsed_args_t;

int parse_args( int, char**, parsed_args_t* );

#ifdef DEBUG
int check_valid_ip(void);
int check_valid_port(void);
int check_arg_parse(void);
#endif

#endif