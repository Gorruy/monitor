#ifndef HLPR
#define HLPR

#define ERROR_EXIT(message) do { \
    perror(message); \
    exit(EXIT_FAILURE); \
} while(0)

#define ERROR_RETURN(message) do { \
    perror(message); \
    return 0; \
} while(0)

#define THREAD_ERROR_RETURN(message) do { \
    perror(message); \
    return (void*) 0; \
} while(0)

#define WRONG_OPT_RETURN(message) do { \
    printf(message); \
    return 0; \
} while(0)

#define NOTVALIDSOCKET(s) ((s) < 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int 
#define MAX_ADDR_SZ 16

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

#endif