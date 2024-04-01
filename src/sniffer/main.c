#include <pthread.h>
#include <pthread.h>
#include <errno.h>

#include "sniffer.h"
#include "sender.h"
#include "arg_parser.h"

#define ERROR_EXIT(message) do { \
  perror(message); \
  exit(EXIT_FAILURE); \
} while(0)

pthread_mutex_t stat_mtx;

int run_threads( parsed_args_t reqs )
{
    pthread_t sniffing_thread;
    pthread_t sending_thread;

    size_t pkt_len = 0;
    size_t pkt_num = 0;
    pthread_mutex_t pkt_mtx;
    pthread_cond_t pkt_cond;

    sender_args_t args_for_sender = {
        .pkt_len_ptr = &pkt_len,
        .pkt_num_ptr = &pkt_num,
    };
    sniff_args_t args_for_sniffer = {
        .req_ip_dest = reqs.ip_dest,
        .req_ip_source = reqs.ip_source,
        .req_port_dest = reqs.port_dest,
        .req_port_source = reqs.port_source,
        .pkt_len_ptr = &pkt_len,
        .pkt_num_ptr = &pkt_num,
    };

    pthread_mutex_init(&pkt_mtx, NULL);
    pthread_cond_init(&pkt_cond, NULL);

    if ( pthread_create( &sniffing_thread, NULL, &sniff, &args_for_sniffer ) != 0 ) {
        ERROR_EXIT("Sniffing thread creation error!");
    }
    if ( pthread_create( &sending_thread, NULL, &send_data_to_representer, &args_for_sender ) != 0 ) {
        ERROR_EXIT("Thread creation error!");
    }

    if ( pthread_join( sniffing_thread, NULL ) != 0 ) {
        ERROR_EXIT("Thread join error");
    }
    if ( pthread_join( sending_thread, NULL ) != 0 ) {
        ERROR_EXIT("Thread join error");    
    }

    pthread_mutex_destroy(&pkt_mtx);
    pthread_cond_destroy(&pkt_cond);

    return 0;
}


int main( int argc, char *argv[] )
{
    parsed_args_t args = parse_args( argc, argv );

    run_threads( args );

    return 0;
}
