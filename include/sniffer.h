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

#ifndef SNFR
#define SNFR

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>


// This struct will be used to pass requriments from programm arguments and values to increment
typedef struct sniff_args {
    size_t interface;
    char *req_ip_dest;
    char *req_ip_source;
    size_t req_port_dest;
    size_t req_port_source;
    volatile size_t *pkt_len_ptr;
    volatile size_t *pkt_num_ptr;
    pthread_mutex_t *pkt_mtx;
    pthread_cond_t *data_ready_sig;
} sniff_args_t;

void* sniff(void*);

extern int break_signal;

#ifdef DEBUG
int packet_meets_reqs(char*, char*, size_t, size_t,  uint8_t*,  struct sockaddr_ll*);
#endif

#endif 