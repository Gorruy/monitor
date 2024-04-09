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

#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_EXIT(message) do { \
    perror(message); \
    exit(EXIT_FAILURE); \
} while(0)

#define SEND_Q_NAME "/NoteQueue"
#define RECV_Q_NAME "/DataQueue"


struct stats {
    size_t packet_num;
    size_t packets_len;
};

static struct stats run_queues(void)
{
    struct stats result;

    mqd_t note_q = mq_open( SEND_Q_NAME, O_WRONLY );
    if ( note_q == (mqd_t) -1 ) {
        ERROR_EXIT("Failed to create queue!");
    }
    mqd_t data_q = mq_open( RECV_Q_NAME, O_RDONLY );

    if ( data_q == (mqd_t) -1 ) {
        ERROR_EXIT("Failed to create queue!");
    }

    if ( mq_send( note_q, (char*)&result, sizeof(char), 0 ) == -1 ) { // Message of zero size to notify sniffer
        ERROR_EXIT("Error in sendind");
    }
    
    if ( mq_receive(data_q, (char*)&result, sizeof(size_t)*2, 0) == -1 ) {
        ERROR_EXIT("Error in recieveing");
    }

    mq_unlink(RECV_Q_NAME);
    mq_unlink(SEND_Q_NAME);

    return result;
}

int main(void)
{
    struct stats stats_to_print = run_queues();

    printf("Number of packets:%ld, size of all packets in bytes:%ld\n",
           stats_to_print.packet_num, 
           stats_to_print.packets_len);
}