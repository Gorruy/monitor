#ifndef SNFR
#define SNFR

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MAX_IP_LEN 40
#define ERROR_EXIT(message) do {\
  fprintf(stderr, message ); \
  perror(strerror(errno)); \
  exit(EXIT_FAILURE); \
} while(0)

// This struct will be used to pass requriments from programm arguments and values to increment
typedef struct sniff_args {
    char* req_ip_dest;
    char* req_ip_source;
    char* req_port_dest;
    char* req_port_source;
} sniff_args_t;

void* sniff(void*);
void* send_data_to_representer(void*);

int check_ip(char*);
int check_port(char*);

#endif 