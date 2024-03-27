#include <pthread.h>

#include "../../include/sniffer/sniffer.h"


pthread_mutex_t stats_mtx;

int main( int argc, char *argv[] )
{
  pthread_t sniffing_thread;
  pthread_t sending_thread;
  pthread_cond_t info_dspl_signal;

  sniff_args_t args;
  args.req_ip_dest = NULL;
  args.req_ip_source = NULL;
  args.req_port_dest = NULL;
  args.req_port_source = NULL;

  if ( pthread_create( &sniffing_thread, NULL, &sniff, &args ) != 0 )
  {
    fprintf(stderr, "Thread creation error!");
    exit(EXIT_FAILURE);
  } 
  if ( pthread_create( &sending_thread, NULL, &send_data_to_representer, NULL ) != 0 )
    ERROR_EXIT("Thread creation error!");

  if ( pthread_join( sniffing_thread, NULL ) != 0 )
  {
    fprintf(stderr, "Thread join error" );
    exit(EXIT_FAILURE);
  }
  if ( pthread_join( sending_thread, NULL ) != 0 )
  {
    fprintf(stderr, "Thread join error" );
    exit(EXIT_FAILURE);
  }    

  return 0;
}