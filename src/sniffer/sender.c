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

    mqd_t notif_q = mq_open(RECV_Q_NAME, O_RDONLY | O_CREAT | O_NONBLOCK, 0666, &notif_attr);
    if ( notif_q == (mqd_t) -1 ) {
        THREAD_ERROR_RETURN("Error in queue creation!");
    }

    mqd_t data_q = mq_open(SEND_Q_NAME, O_WRONLY | O_CREAT, 0666, &data_attr);
    if ( data_q == (mqd_t) -1 ) {
        THREAD_ERROR_RETURN("Error in queue creation!");
    }

    size_t note[2];
    ssize_t rcv_status;
    while (1) {
        rcv_status = mq_receive(notif_q, (char*)note, sizeof(size_t)*2, NULL);

        if ( rcv_status == -1 && errno != EAGAIN) {
            THREAD_ERROR_RETURN("Error when receiving message from queue");
        }
        else if ( rcv_status != -1 ) {
            break;
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

    size_t stats_to_send[] = { all_pkt_num, all_pkt_len };

    int send_status = mq_send(data_q, (char*)&stats_to_send, sizeof(size_t)*2, 0);

    if ( send_status == -1 ){
        THREAD_ERROR_RETURN("Error when sending message to queue");
    }

    mq_unlink(RECV_Q_NAME);
    mq_unlink(SEND_Q_NAME);

    break_signal = 1; 

    return (void*) 1;
}