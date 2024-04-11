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

#include "sniffer.h"
#include "sender.h"
#include "arg_parser.h"
#include "helpers.h"


int run_threads( parsed_args_t *reqs )
{
    int sniff_status;
    int send_status;

    pthread_t sniffing_thread;
    pthread_t sending_thread;

    volatile size_t pkt_len = 0;
    volatile size_t pkt_num = 0;

    sender_args_t args_for_sender = {
        .pkt_len_ptr = &pkt_len,
        .pkt_num_ptr = &pkt_num,
    };
    sniff_args_t args_for_sniffer = {
        .interface = reqs->interface,
        .req_ip_dest = reqs->ip_dest,
        .req_ip_source = reqs->ip_source,
        .req_port_dest = reqs->port_dest,
        .req_port_source = reqs->port_source,
        .pkt_len_ptr = &pkt_len,
        .pkt_num_ptr = &pkt_num,
    };

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

    if ( sniff_status < 0 || send_status < 0 ) {
        return 0;
    }

    return 1;
}


int main( int argc, char *argv[] )
{
    parsed_args_t args;
    
    if ( !parse_args( argc, argv, &args ) ) {
        ERROR_EXIT("Error while parsing options");
    }

    printf("Sniffing started!\n");
    if ( !run_threads( &args ) ) {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
