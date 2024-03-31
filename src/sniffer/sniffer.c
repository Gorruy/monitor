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

#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>

#include <getopt.h>

#include "sniffer.h"

#define NOTVALIDSOCKET(s) ((s) < 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int 
#define UDP_IN_IP_HDR 17
#define IPV4_VERSION 4
#define MAX_IP_LEN 40

#define ERROR_EXIT(message) do {\
  fprintf(stderr, message ); \
  perror(strerror(errno)); \
  exit(EXIT_FAILURE); \
} while(0)

static int packet_meets_reqs(const char* req_ip_source,
                             const char* req_ip_dest,
                             const char* req_port_source,
                             const char* req_port_dest, 
                             uint8_t* packet, 
                             struct sockaddr_ll addr_info)
{
    // Drop all tx packets
    if ( addr_info.sll_pkttype == PACKET_OUTGOING ) {
        return 0;
    }

    // check l2 header
    struct ethhdr* eth = (struct ethhdr*)packet;
    size_t protocol = ntohs(eth->h_proto);
  
    if ( protocol != ETH_P_IP && protocol != ETH_P_IPV6 ) {
        return 0;
    }
  
    // check ip header
    char ip_dest[MAX_IP_LEN];
    char ip_source[MAX_IP_LEN];
    int ip_hdr_offset;
    struct iphdr* ips = (struct iphdr*)( packet + sizeof(struct ethhdr) );
  
    printf("%d", ips->protocol);
    fflush(stdout);
    if ( ips->protocol != UDP_IN_IP_HDR ) {
        return 0;
    }
  
    if ( ips->version == IPV4_VERSION )
    {
        inet_ntop( AF_INET, &(ips->daddr), ip_dest, MAX_IP_LEN );
        inet_ntop( AF_INET, &(ips->saddr), ip_source, MAX_IP_LEN );
  
        ip_hdr_offset = ips->ihl*4;
    }
    else
    {
        struct ipv6hdr* ips6 = (struct ipv6hdr*)( packet + sizeof(struct ethhdr) );
  
        inet_ntop( AF_INET6, &(ips6->daddr), ip_dest, MAX_IP_LEN );
        inet_ntop( AF_INET6, &(ips6->saddr), ip_source, MAX_IP_LEN );
  
        ip_hdr_offset = sizeof(struct ipv6hdr);
    } 
  
    if ( req_ip_dest && strcmp( req_ip_dest, ip_dest ) != 0 ) {
        return 0;
    }
    if ( req_ip_dest && strcmp( req_ip_dest, ip_source ) != 0 ) {
        return 0;
    }
  
    // check udp header
    struct udphdr* udp;
    udp = (struct udphdr*)( packet + sizeof(struct ethhdr) + ip_hdr_offset );
  
    if ( req_port_dest && atoi( req_port_dest) != ntohs(udp->dest) ) {
        return 0;
    }
    if ( req_port_source && atoi( req_port_source) != ntohs(udp->source) ) {
        return 0;
    }
     
    return 1;
}

void* sniff( void* args_struct_ptr ) 
{
    /* Creates a packet socket at l2 and checks all incoming
      packages of udp protocol for matching with requirements 
      if some are given  */

    sniff_args_t* args = (sniff_args_t*)args_struct_ptr;

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
  
        int pkt_len = recvfrom(raw_socket, 
                               buffer, 
                               USHRT_MAX, 
                               MSG_DONTWAIT, 
                               (struct sockaddr*)&addr_info, 
                               &info_len);
        if ( pkt_len == -1 )
        {
            if ( errno != EAGAIN ) {
                ERROR_EXIT("Error in recvfrom\n");
            }
            else {
                continue;
            }
        }
  
        if ( packet_meets_reqs(args->req_ip_source,
                               args->req_ip_dest, 
                               args->req_port_source, 
                               args->req_port_dest,  
                               buffer, 
                               addr_info)) {

            pthread_mutex_lock(args->stat_mtx_ptr);
            *(args->pkt_len_ptr) = pkt_len;
            pthread_cond_signal(args->new_data_sig_ptr);
            pthread_mutex_unlock(args->stat_mtx_ptr);
        }
    }

    // Unreachable
    free(buffer);
    CLOSESOCKET(raw_socket);
}
