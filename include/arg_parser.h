#ifndef APRS
#define APRS

typedef struct parsed_args {
    char* ip_dest;
    char* ip_source;
    char* port_dest;
    char* port_source;
} parsed_args_t;

parsed_args_t parse_args( int, char**);

#endif