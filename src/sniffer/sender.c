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
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "sender.h"
#include "helpers.h"

#define SEND_Q_NAME "/DataQueue"
#define RECV_Q_NAME "/NoteQueue"

// Global var that signals to whole app that representer send message
int break_signal;

static void sigint_handler(int sig)
{
    if ( sig == SIGINT ) {
        signal( SIGINT, SIG_DFL );
        mq_unlink(RECV_Q_NAME);
        mq_unlink(SEND_Q_NAME);
        kill( getpid(), SIGINT );
    }
}

void* send_data_to_representer(void* args_struct_ptr)
{
    /* Creates a posix message queue and starts listening for
       signal from representer proccess, after which it sends 
       collected statistics to representer  */

    sender_args_t* args = (sender_args_t*)args_struct_ptr;

    signal(SIGINT, sigint_handler);

    size_t all_pkt_num = 0;
    size_t all_pkt_len = 0;

    struct mq_attr notif_attr = {
        .mq_maxmsg = 1,
        .mq_msgsize = sizeof(size_t)
    };

    struct mq_attr data_attr = {
        .mq_maxmsg = 1,
        .mq_msgsize = sizeof(size_t)*2
    };

    mqd_t notif_q = mq_open(RECV_Q_NAME, O_RDONLY | O_CREAT, 0666, &notif_attr);
    if ( notif_q == (mqd_t) -1 ) {
        THREAD_ERROR_RETURN("Error in queue creation!");
    }

    mqd_t data_q = mq_open(SEND_Q_NAME, O_WRONLY | O_CREAT, 0666, &data_attr);
    if ( data_q == (mqd_t) -1 ) {
        THREAD_ERROR_RETURN("Error in queue creation!");
    }

    size_t note[2];

    if ( mq_receive(notif_q, (char*)note, sizeof(size_t)*2, NULL) == -1 ) {
        THREAD_ERROR_RETURN("Error when receiving message from queue");
    }

    all_pkt_num = *(args->pkt_num_ptr);
    all_pkt_len = *(args->pkt_len_ptr);

    size_t stats_to_send[] = { all_pkt_num, all_pkt_len };

    fflush(stdout);

    int send_status = mq_send(data_q, (char*)&stats_to_send, sizeof(size_t)*2, 0);

    if ( send_status == -1 ){
        THREAD_ERROR_RETURN("Error when sending message to queue");
    }

    mq_unlink(RECV_Q_NAME);
    mq_unlink(SEND_Q_NAME);

    break_signal = 1; 

    return (void*) 1;
}
