#ifndef SNDR
#define SNDR

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

typedef struct sender_args {
    size_t* pkt_len_ptr;
    size_t* pkt_num_ptr;
} sender_args_t;

void* send_data_to_representer(void*);

#endif