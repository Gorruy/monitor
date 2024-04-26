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

#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

#define SEND_Q_NAME "/NoteQueue"
#define RECV_Q_NAME "/DataQueue"


struct stats {
    size_t packet_num;
    size_t packets_len;
};

static int run_queues(struct stats *result)
{
    mqd_t note_q = mq_open( SEND_Q_NAME, O_WRONLY );
    if ( note_q == (mqd_t) -1 ) {
        ERROR_RETURN("Failed to create queue!\n");
    }
    mqd_t data_q = mq_open( RECV_Q_NAME, O_RDONLY );

    if ( data_q == (mqd_t) -1 ) {
        ERROR_RETURN("Failed to create queue!\n");
    }

    if ( mq_send( note_q, (char*)result, sizeof(char), 0 ) == -1 ) { // Message of zero size to notify sniffer
        ERROR_RETURN("Error in sendind\n");
    }
    
    if ( mq_receive(data_q, (char*)result, sizeof(size_t)*2, 0) == -1 ) {
        ERROR_RETURN("Error in recieveing\n");
    }

    return 1;
}

int main(void)
{
    struct stats stats_to_print = {0};
    if ( !run_queues(&stats_to_print) ) {
        ERROR_EXIT("Error while running queues!\n");
    }

    printf("Number of packets:%ld, size of all packets in bytes:%ld\n",
           stats_to_print.packet_num, 
           stats_to_print.packets_len);
}