#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>

#include "sender.h"

#define QUEUE_NAME "/SniffingQueue"

#define ERROR_EXIT(message) do {\
  fprintf(stderr, message ); \
  perror(strerror(errno)); \
  exit(EXIT_FAILURE); \
} while(0)


void* send_data_to_representer(void* args_struct_ptr)
{
    /* Creates a posix message queue and starts listening for
       signal from representer proccess, after which it sends 
       collected statistics to representer  */

    sender_args_t* args = (sender_args_t*)args_struct_ptr;

    struct stats {
        size_t all_pkt_num;
        size_t all_pkt_len;
    } stats_to_send;

    stats_to_send.all_pkt_num = 0;
    stats_to_send.all_pkt_len = 0;

    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(size_t)*2;

    mqd_t queue_to_representer = mq_open(QUEUE_NAME,
                                         O_RDWR | O_CREAT | O_NONBLOCK,
                                         0777,
                                         &attr);
    if ( queue_to_representer == (mqd_t) -1 ) {
        ERROR_EXIT("Error in queue creation!\n");
    }

    ssize_t rcv_status;
    while (1) {
        rcv_status = mq_receive(queue_to_representer, 
                                (char*)&stats_to_send, 
                                sizeof(size_t)*2, 
                                NULL);

        if ( rcv_status == -1 && errno != EAGAIN ) {
            ERROR_EXIT("Error when receiving message from queue\n");
        }
        else if ( errno != EAGAIN ) {
            break;
        }
                printf("some");
        fflush(stdout);

        pthread_mutex_lock(args->stat_mtx_ptr);
        pthread_cond_wait(args->new_data_sig_ptr, args->stat_mtx_ptr);
        stats_to_send.all_pkt_num += *(args->pkt_len_ptr);
        stats_to_send.all_pkt_len += 1;
        pthread_mutex_unlock(args->stat_mtx_ptr);
    }

    int send_status = mq_send(queue_to_representer, 
                              (char*)&stats_to_send, 
                              sizeof(size_t)*2, 
                              0);

    if ( send_status == -1 ){
        ERROR_EXIT("Error when sending message to queue\n");
    }

    mq_unlink(QUEUE_NAME);
    exit(EXIT_SUCCESS);
}