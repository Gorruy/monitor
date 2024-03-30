#include <pthread.h>

#include "sniffer.h"

int main( int argc, char *argv[] )
{
    if ( argc > 5 ) {
        ERROR_EXIT("Too many options!! Try --help to get info on usage");
    }

    sniff_args_t* args;
    parse_args( argc, argv, &args );

    run_threads( args );

    return 0;
}

int run_threads( void* args_for_sniffer )
{
    pthread_t sniffing_thread;
    pthread_t sending_thread;

    if ( pthread_create( &sniffing_thread, NULL, &sniff, &args_for_sniffer ) != 0 ) {
        ERROR_EXIT("Sniffing thread creation error!\n");
    }
    if ( pthread_create( &sending_thread, NULL, &send_data_to_representer, NULL ) != 0 ) {
        ERROR_EXIT("Thread creation error!\n");
    }

    if ( pthread_join( sniffing_thread, NULL ) != 0 ) {
        ERROR_EXIT("Thread join error\n");
    }
    if ( pthread_join( sending_thread, NULL ) != 0 ) {
        ERROR_EXIT("Thread join error\n");    
    }

    return 0;
}
