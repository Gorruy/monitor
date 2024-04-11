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
#include <unistd.h>
#include <arpa/inet.h>
#include <limits.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/ipv6.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>

#include "sniffer.h"
#include "helpers.h"

#define UDP_IN_IP_HDR 17
#define IPV4_VERSION 4
#define MAX_IP_LEN 40
#define MAX_HEADERS_SIZE 66

#ifdef DEBUG
#define STATIC static
#else
#define STATIC
#endif


STATIC int packet_meets_reqs(const char* req_ip_source,
                             const char* req_ip_dest,
                             size_t req_port_source,
                             size_t req_port_dest, 
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
  
    if ( ips->protocol != UDP_IN_IP_HDR ) {
        return 0;
    }
  
    if ( ips->version == IPV4_VERSION ) {
        ip_hdr_offset = ips->ihl*4;
    }
    else {
        ip_hdr_offset = sizeof(struct ipv6hdr);
    } 
  
    if ( req_ip_dest && strcmp( req_ip_dest, ip_dest ) != 0 ) {
        return 0;
    }
    if ( req_ip_source && strcmp( req_ip_source, ip_source ) != 0 ) {
        return 0;
    }
  
    // check udp header
    struct udphdr* udp;
    udp = (struct udphdr*)( packet + sizeof(struct ethhdr) + ip_hdr_offset );
  
    if ( req_port_dest && req_port_dest != ntohs(udp->dest) ) {
        return 0;
    }
    if ( req_port_source && req_port_source != ntohs(udp->source) ) {
        return 0;
    }
     
    return 1;
}

void* sniff( void* args_struct_ptr ) 
{
    /* Creates a packet socket at l2 and checks all incoming
      packages of udp protocol for matching with requirements 
      if some are given  */

    struct sockaddr_ll addr_info;
    struct ifreq rq;
    struct packet_mreq mreq;

    sniff_args_t* args = (sniff_args_t*)args_struct_ptr;
    socklen_t info_len = sizeof(struct sockaddr_ll);

    memset( &addr_info, 0, sizeof(struct sockaddr_ll) );
    memset( &rq, 0, sizeof(struct ifreq) );
    memset( &mreq,0, sizeof(struct packet_mreq) );

    uint8_t* buffer = (uint8_t*)malloc(USHRT_MAX); // 65535 for max size of udp packet
    if (!buffer) {
      THREAD_ERROR_RETURN("Malloc error!\n");
    }

    SOCKET raw_socket = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );
    if ( NOTVALIDSOCKET(raw_socket) ) {
      THREAD_ERROR_RETURN("Error in socket creation\n");
    }

    strcpy(rq.ifr_name, args->interface);
    if ( setsockopt(raw_socket, 
                    SOL_SOCKET, 
                    SO_BINDTODEVICE, 
                    (void *)&rq, 
                    sizeof(rq)) < 0) {
        THREAD_ERROR_RETURN("Can't bind to interface!\n");
    }

    mreq.mr_ifindex = if_nametoindex(args->interface);
    mreq.mr_type = PACKET_MR_PROMISC;
    mreq.mr_alen = 6;

    if ( setsockopt(raw_socket, 
                    SOL_PACKET, 
                    PACKET_MR_PROMISC,
                    (void*)&mreq, 
                    (socklen_t)sizeof(struct packet_mreq)) < 0 ) {
        THREAD_ERROR_RETURN("Can't turn socket to promiscouous mode!\n");
    }

    while (!break_signal) {
        memset(buffer, 0, MAX_HEADERS_SIZE);
  
        int pkt_len = recvfrom(raw_socket, 
                               buffer, 
                               USHRT_MAX, 
                               MSG_DONTWAIT, 
                               (struct sockaddr*)&addr_info, 
                               &info_len);
        if ( pkt_len == -1 )
        {
            if ( errno != EAGAIN ) {
                THREAD_ERROR_RETURN("Error in recvfrom!\n");
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
            *(args->pkt_len_ptr) += pkt_len;
            *(args->pkt_num_ptr) += 1;
        }
    }

    // todo: fix unreachable
    free(buffer);
    CLOSESOCKET(raw_socket);

    return (void *) 1;
}
