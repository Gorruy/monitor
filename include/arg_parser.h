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
    size_t interface;
    char ip_dest[MAX_ADDR_SZ];
    char ip_source[MAX_ADDR_SZ];
    size_t port_dest;
    size_t port_source;
} parsed_args_t;

int parse_args( int, char**, parsed_args_t* );

#endif