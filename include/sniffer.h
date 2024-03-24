#ifndef SNFR
#define SNFR

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <linux/if_ether.h>
#include <limits.h>
#include <netinet/ip.h>
#include <netinet/udp.h>


#define NOTVALIDSOCKET(s) ((s) < 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int 
#define GETSOCKETERRNO() (errno)

int sniff(void);

#endif 