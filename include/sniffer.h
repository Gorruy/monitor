#ifndef SNFR
#define SNFR

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>


// This struct will be used to pass requriments from programm arguments and values to increment
typedef struct sniff_args {
    char* req_ip_dest;
    char* req_ip_source;
    char* req_port_dest;
    char* req_port_source;
    size_t* pkt_len_ptr;
    pthread_mutex_t* stat_mtx_ptr;
    pthread_cond_t* new_data_sig_ptr;
} sniff_args_t;

void* sniff(void*);

#endif 