#ifndef SNDR
#define SNDR

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

typedef struct sender_args {
    size_t* pkt_len_ptr;
    pthread_mutex_t* stat_mtx_ptr;
    pthread_cond_t* new_data_sig_ptr;
} sender_args_t;

void* send_data_to_representer(void*);

#endif