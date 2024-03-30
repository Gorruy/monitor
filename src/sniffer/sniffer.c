#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <limits.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/ipv6.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <ctype.h>

#include <getopt.h>

#include <fcntl.h>
#include <mqueue.h>

#include "sniffer.h"

#define NOTVALIDSOCKET(s) ((s) < 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int 
#define QUEUE_NAME "/SniffingQueue"
#define UDP_IN_IP_HDR 17
#define IPV4_VERSION 4

static size_t all_pkt_len = 0;
static size_t all_pkt_num = 0;

void* sniff( void* args_struct_ptr ) 
{
    /* Creates a packet socket at l2 and checks all incoming
      packages of udp protocol for matching with requirements 
      if some are given  */

    sniff_args_t* args = (sniff_args_t*)args_struct_ptr;

    // todo: fix seg fault by strcmp args->req_*
    const char* req_ip_source = args->req_ip_source;
    const char* req_ip_dest = args->req_ip_dest;
    const char* req_port_source = args->req_port_source;
    const char* req_port_dest = args->req_port_dest;

    struct sockaddr_ll addr_info;
    memset( &addr_info, 0, sizeof(struct sockaddr_ll) );

    socklen_t info_len = sizeof(struct sockaddr_ll);

    uint8_t* buffer = (uint8_t*)malloc(USHRT_MAX); // 65535 for max size of udp packet
    if (!buffer) {
      ERROR_EXIT("Malloc error!\n");
    }

    SOCKET raw_socket = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );

    if ( NOTVALIDSOCKET(raw_socket) ) {
      ERROR_EXIT("Error in socket creation\n");
    }

    while (1) {
        memset(buffer, 0, USHRT_MAX);
  
        int pkt_len = recvfrom( raw_socket, buffer, USHRT_MAX, MSG_DONTWAIT, (struct sockaddr*)&addr_info, &info_len );
        if ( pkt_len == -1 )
        {
            if ( errno != EAGAIN ) {
                ERROR_EXIT("Error in recvfrom\n");
            }
            else {
                continue;
            }
        }
  
        // Drop all tx packets
        if ( addr_info.sll_pkttype == PACKET_OUTGOING ) {
            continue;
        }
  
        // check l2 header
        struct ethhdr* eth = (struct ethhdr*)buffer;
        size_t protocol = ntohs(eth->h_proto);
  
        if ( protocol != ETH_P_IP && protocol != ETH_P_IPV6 ) {
            continue;
        }
  
        // check ip header
        char ip_dest[MAX_IP_LEN];
        char ip_source[MAX_IP_LEN];
        int ip_hdr_offset;
        struct iphdr* ips = (struct iphdr*)( buffer + sizeof(struct ethhdr) );
  
        if ( ips->protocol != UDP_IN_IP_HDR ) {
            continue;
        }
  
        if ( ips->version == IPV4_VERSION )
        {
            inet_ntop( AF_INET, &(ips->daddr), ip_dest, MAX_IP_LEN );
            inet_ntop( AF_INET, &(ips->saddr), ip_source, MAX_IP_LEN );
  
            ip_hdr_offset = ips->ihl*4;
        }
        else
        {
            struct ipv6hdr* ips6 = (struct ipv6hdr*)( buffer + sizeof(struct ethhdr) );
  
            inet_ntop( AF_INET6, &(ips6->daddr), ip_dest, MAX_IP_LEN );
            inet_ntop( AF_INET6, &(ips6->saddr), ip_source, MAX_IP_LEN );
  
            ip_hdr_offset = sizeof(struct ipv6hdr);
        } 
  
        if ( req_ip_dest && strcmp( req_ip_dest, ip_dest ) != 0 ) {
            continue;
        }
        if ( req_ip_source && strcmp( req_port_source, ip_source ) != 0 ) {
            continue;
        }
  
        // check udp header
        struct udphdr* udp;
        udp = (struct udphdr*)( buffer + sizeof(struct ethhdr) + ip_hdr_offset );
  
        if ( req_port_dest && atoi( req_port_dest) != ntohs(udp->dest) ) {
            continue;
        }
        if ( req_port_source && atoi( req_port_source) != ntohs(udp->source) ) {
            continue;
        }
  
        // Packet satisfies all requirements, can add it to statistic
        all_pkt_len += pkt_len;
        all_pkt_num += 1;

    }

    free(buffer);
    CLOSESOCKET(raw_socket);
}

void* send_data_to_representer(void* args)
{
    /* Creates a posix message queue and starts listening for
      signal from representer proccess, after which it sends 
      collected statistics to representer  */

    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(size_t)*2;

    mqd_t queue_to_representer = mq_open(QUEUE_NAME, O_RDWR | O_CREAT, 0777, &attr );
    if ( queue_to_representer == (mqd_t) -1 ) {
        ERROR_EXIT("Error in queue creation!\n");
    }

    size_t stats_ptr[2];

    if ( mq_receive(queue_to_representer, (char*)stats_ptr, sizeof(size_t)*2, 0 ) == -1 ) {
        ERROR_EXIT("Error when receiving message from queue\n");
    }

    stats_ptr[0] = all_pkt_num;
    stats_ptr[1] = all_pkt_len;

    if ( mq_send(queue_to_representer, (char*)stats_ptr, sizeof(size_t)*2, 0 ) == -1 ){
        ERROR_EXIT("Error when sending message to queue\n");
    }

    mq_unlink(QUEUE_NAME);
    exit(EXIT_SUCCESS);
}

int is_valid_ip( char* ip )
{
    char buf[100];
    if ( inet_pton( AF_INET6, ip, buf ) > 0 ||
         inet_pton( AF_INET, ip, buf ) > 0 ) {
        return 1;
    }
    else {
        return 0;
    }
}

int is_valid_port( char* port )
{
    for ( size_t i = 0; i < strlen(port); i++ )
    {
        if ( !isdigit(port[i]) ) {
          return 0;
        }
    }

    return 1;
}

int parse_args( int argc, char* argv[], sniff_args_t* args ) 
{
    enum opt_names {
        IPSRC,
        IPDEST,
        PORTSRC,
        PORTDEST
    };

    args->req_ip_dest = NULL;
    args->req_ip_source = NULL;
    args->req_port_dest = NULL;
    args->req_port_source = NULL; 

    const struct option options[] = {
        { .name = "ipsrc", .has_arg = required_argument, .flag = 0, .val = IPSRC },
        { .name = "ipdest", .has_arg = required_argument, .flag = 0, .val = IPDEST },
        { .name = "portsrc", .has_arg = required_argument, .flag = 0, .val = PORTSRC },
        { .name = "portdest", .has_arg = required_argument, .flag = 0, .val = PORTDEST },
        { .name = "help", .has_arg = no_argument, .flag = 0, .val = 'h' },
        { .name = NULL, .has_arg = 0, .flag = NULL, .val = 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "1:2:3:4:", options, NULL)) != -1 ) {
        switch (opt) {
          case IPSRC:
              if ( is_valid_ip(optarg) ) {
                  args->req_ip_source = optarg;
                  break;
              }
              else {
                  ERROR_EXIT("Wrong value of reqired ip source address!\n");
              }
          case IPDEST:
              if ( is_valid_ip(optarg) ) {
                  args->req_ip_dest = optarg;
                  break;
              }
              else {
                  ERROR_EXIT("Wrong value of reqired ip dest address!\n");
              }
          case PORTSRC:
              if ( is_valid_port(optarg) ) {
                  args->req_port_source = optarg;
                  break;
              }
              else {
                  ERROR_EXIT("Wrong value of reqired source port value!\n");
              }
          case PORTDEST:
              if ( is_valid_port(optarg) ) {
                  args->req_port_dest = optarg;
                  break;
              }
              else {
                  ERROR_EXIT("Wrong value of reqired dest port value!\n");
              }
          case 'h':
              printf("Usage: sniffer [OPTION]... [ADDRESS]...\n"
                     "Collect statistic on incoming udp packages\n"
                     "pass it to representer\n\n"
                     "--ipsrc [ADDRESS]   defines source ip address for filtering\n"
                     "                    ADDRESS can be ipv4 or ipv6 address in standard format\n"
                     "--ipdest [ADDRESS]  defines destination ip address for filtering\n"
                     "                    ADDRESS can be ipv4 or ipv6 address in standard format\n"
                     "--ipsrc [PORT]      defines source port number for filtering\n"
                     "                    PORT can be any positive number\n"
                     "--ipsrc [PORT]      defines source ip address for filtering\n"
                     "                    PORT can be any positive number\n"
                    );
              exit(EXIT_SUCCESS);
        }
    }
}

