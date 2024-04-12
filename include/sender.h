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

#ifndef SNDR
#define SNDR

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

typedef struct sender_args {
    volatile size_t *pkt_len_ptr;
    volatile size_t *pkt_num_ptr;
    pthread_mutex_t *pkt_mtx;
    pthread_cond_t *data_ready_sig;
} sender_args_t;

void* send_data_to_representer(void*);

extern volatile int break_signal;

#endif