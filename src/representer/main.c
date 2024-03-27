#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(void)
{
  mqd_t queue_to_sniffer = mq_open("/SniffingQueue", O_RDWR );
  if ( queue_to_sniffer == (mqd_t) -1 )
  {
    perror(strerror(errno));
    exit(EXIT_FAILURE);
  }

  size_t stats[2];

  mq_send( queue_to_sniffer, (char*)stats, 1, 0 );
  mq_receive( queue_to_sniffer, (char*)stats, sizeof(size_t)*2, 0 );
  printf( "%ld, %ld", stats[0], stats[1] );
}