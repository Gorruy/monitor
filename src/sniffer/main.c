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

#include <pthread.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include "sniffer.h"
#include "sender.h"
#include "arg_parser.h"
#include "helpers.h"

// Global var that signals to whole app that representer send message
volatile int break_signal;

static void sigint_handler(int sig)
{
    if ( sig == SIGINT ) {
        signal( SIGINT, SIG_DFL );
        break_signal = 1; 
    }
}

int run_threads( parsed_args_t *reqs )
{
    int sniff_status;
    int send_status;

    pthread_t sniffing_thread;
    pthread_t sending_thread;

    pthread_mutex_t pkt_mtx;

    volatile size_t pkt_len = 0;
    volatile size_t pkt_num = 0;

    sender_args_t args_for_sender = {
        .pkt_len_ptr = &pkt_len,
        .pkt_num_ptr = &pkt_num,
        .pkt_mtx = &pkt_mtx
    };
    sniff_args_t args_for_sniffer = {
        .interface = reqs->interface,
        .req_ip_dest = reqs->ip_dest,
        .req_ip_source = reqs->ip_source,
        .req_port_dest = reqs->port_dest,
        .req_port_source = reqs->port_source,
        .pkt_len_ptr = &pkt_len,
        .pkt_num_ptr = &pkt_num,
        .pkt_mtx = &pkt_mtx
    };

    pthread_mutex_init( &pkt_mtx, NULL );

    if ( pthread_create(&sniffing_thread, 
                        NULL, 
                        &sniff, 
                        &args_for_sniffer) != 0 ) {
        ERROR_RETURN("Sniffing thread creation error!");
    }
    if ( pthread_create(&sending_thread, 
                        NULL, 
                        &send_data_to_representer, 
                        &args_for_sender) != 0) {
        ERROR_RETURN("Representer thread creation error!");
    }

    if ( pthread_join( sniffing_thread, (void**)&sniff_status ) != 0 ) {
        ERROR_RETURN("Thread join error");
    }
    if ( pthread_join( sending_thread, (void**)&send_status ) != 0 ) {
        ERROR_RETURN("Thread join error");    
    }

    pthread_mutex_destroy(&pkt_mtx);

    if ( sniff_status < 0 || send_status < 0 ) {
        return 0;
    }

    return 1;
}


int main( int argc, char *argv[] )
{
    signal(SIGINT, sigint_handler);
    parsed_args_t args;
    
    int parse_args_res = parse_args( argc, argv, &args );
    if ( parse_args_res == -1 ) {
        exit(EXIT_SUCCESS);
    }
    else if ( !parse_args_res ) {
        ERROR_EXIT("Error while parsing options\n");
    }

    printf("Sniffing started!\n");
    if ( argc == 3 ) {
        printf("Module will sniff all packets at given interface\n");
    }
    if ( !run_threads( &args ) ) {
        ERROR_EXIT("Error while running threads\n");
    }

    printf("Sniffing is over!\n");
    exit(EXIT_SUCCESS);
}
