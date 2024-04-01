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


int main(void)
{
    mqd_t note_q = mq_open(SEND_Q_NAME, O_RDWR );
    if ( note_q == (mqd_t) -1 ) {
        ERROR_EXIT("Failed to create queue!");
    }
    mqd_t data_q = mq_open(RECV_Q_NAME, O_RDWR );
    if ( data_q == (mqd_t) -1 ) {
        ERROR_EXIT("Failed to create queue!");
    }

    size_t stats[2];

    if ( mq_send( note_q, (char*)stats, sizeof(char), 0 ) == -1 ) { // Message of zero size to notify sniffer
        ERROR_EXIT("Error in sendind");
    }
    
    if ( mq_receive(data_q, (char*)stats, sizeof(size_t)*2, 0) == -1 ) {
        ERROR_EXIT("Error in recieveing");
    }

    printf("Number of packets:%ld, size of all packets in bytes:%ld\n",
           stats[0], 
           stats[1]);

    mq_unlink(RECV_Q_NAME);
    mq_unlink(SEND_Q_NAME);
}