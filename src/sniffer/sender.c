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
#include <unistd.h>

#include "sender.h"
#include "helpers.h"

#define SEND_Q_NAME "/DataQueue" // Queue that will send data to representer
#define RECV_Q_NAME "/NoteQueue" // Queue that will recieve notification from representer
#define CWAIT_TIMEOUT_NSEC 1000000


void* send_data_to_representer(void* args_struct_ptr)
{
    /* Creates a posix message queue and starts listening for
       signal from representer process, after which it sends 
       collected statistics to representer  */
       
    struct timespec timeout = {
        .tv_sec = 0,
        .tv_nsec = CWAIT_TIMEOUT_NSEC
    };
    sender_args_t* args = (sender_args_t*)args_struct_ptr;

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

    mqd_t notif_q = mq_open(RECV_Q_NAME, O_RDONLY | O_CREAT | O_NONBLOCK, 0666, &notif_attr);
    if ( notif_q == (mqd_t) -1 ) {
        THREAD_ERROR_RETURN("Error in queue creation!");
    }

    mqd_t data_q = mq_open(SEND_Q_NAME, O_WRONLY | O_CREAT, 0666, &data_attr);
    if ( data_q == (mqd_t) -1 ) {
        THREAD_ERROR_RETURN("Error in queue creation!");
    }

    size_t stats_to_send[2];
    size_t note[2];
    ssize_t rcv_status;
    while (!break_signal) {
        rcv_status = mq_receive(notif_q, (char*)note, sizeof(size_t)*2, NULL);

        if ( rcv_status == -1 && errno != EAGAIN) {
            THREAD_ERROR_RETURN("Error when receiving message from queue");
        }
        else if ( rcv_status != -1 ) {
            pthread_mutex_lock(args->pkt_mtx);
            stats_to_send[0] = all_pkt_num;
            stats_to_send[1] = all_pkt_len;

            int send_status = mq_send(data_q, (char*)&stats_to_send, sizeof(size_t)*2, 0);

            all_pkt_num = 0;
            all_pkt_len = 0;
            pthread_mutex_unlock(args->pkt_mtx);

            if ( send_status == -1 ){
                THREAD_ERROR_RETURN("Error when sending message to queue");
            }
        }        

        pthread_mutex_lock(args->pkt_mtx);
        if ( pthread_cond_timedwait(args->data_ready_sig, 
                                    args->pkt_mtx, 
                                    &timeout) != ETIMEDOUT ) {
            all_pkt_num += 1;
            all_pkt_len += *(args->pkt_len_ptr);
        }
        pthread_mutex_unlock(args->pkt_mtx);
    }

    mq_unlink(RECV_Q_NAME);
    mq_unlink(SEND_Q_NAME);

    return (void*) 1;
}