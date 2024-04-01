#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>

#include "sender.h"

#define SEND_Q_NAME "/DataQueue"
#define RECV_Q_NAME "/NoteQueue"

#define ERROR_EXIT(message) do { \
  perror(message); \
  exit(EXIT_FAILURE); \
} while(0)


void* send_data_to_representer(void* args_struct_ptr)
{
    /* Creates a posix message queue and starts listening for
       signal from representer proccess, after which it sends 
       collected statistics to representer  */

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
        ERROR_EXIT("Error in queue creation!\n");
    }

    mqd_t data_q = mq_open(SEND_Q_NAME, O_WRONLY | O_CREAT, 0666, &data_attr);
    if ( data_q == (mqd_t) -1 ) {
        ERROR_EXIT("Error in queue creation!\n");
    }
    size_t note[2];

    ssize_t rcv_status;
    while (1) {
        rcv_status = mq_receive(notif_q, (char*)note, sizeof(size_t)*2, NULL);

        if ( rcv_status == -1 && errno != EAGAIN) {
            ERROR_EXIT("Error when receiving message from queue\n");
        }
        else if ( rcv_status != -1 ) {
            break;
        }        

        pthread_mutex_lock(args->stat_mtx_ptr);
        pthread_cond_wait(args->new_data_sig_ptr, args->stat_mtx_ptr);
        all_pkt_num += 1;
        all_pkt_len += *(args->pkt_len_ptr);
        pthread_mutex_unlock(args->stat_mtx_ptr);
    }

    size_t stats_to_send[] = { all_pkt_num, all_pkt_len };

    int send_status = mq_send(data_q, (char*)&stats_to_send, sizeof(size_t)*2, 0);

    if ( send_status == -1 ){
        ERROR_EXIT("Error when sending message to queue\n");
    }

    mq_unlink(RECV_Q_NAME);
    mq_unlink(SEND_Q_NAME);
    exit(EXIT_SUCCESS);
}