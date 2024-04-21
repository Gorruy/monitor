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
#include <sys/ioctl.h>
#include <net/if.h>
#include <ctype.h>

#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>

#include "sniffer.h"
#include "helpers.h"

#define UDP_IN_IP_HDR 17
#define IPV4_VERSION 4
#define MAX_IP_LEN 40
#define MAX_HEADERS_SIZE 66

// array full of zeros for compare purposes
static char zeros[MAX_ADDR_SZ];

static int packet_meets_reqs(sniff_args_t *args,
                             uint8_t *packet, 
                             struct sockaddr_ll *addr_info)
{
    // Drop all tx packets
    if ( addr_info->sll_pkttype == PACKET_OUTGOING ) {
        return 0;
    }

    // check l2 header
    struct ethhdr* eth = (struct ethhdr*)packet;
    size_t protocol = ntohs(eth->h_proto);
  
    if ( protocol != ETH_P_IP && protocol != ETH_P_IPV6 ) {
        return 0;
    }
  
    // check ip header
    int ip_hdr_offset;
    struct ipv6hdr* ips6;
    struct iphdr* ips = (struct iphdr*)( packet + sizeof(struct ethhdr) );
  
    if ( ips->protocol != UDP_IN_IP_HDR ) {
        return 0;
    }
  
    if ( ips->version == IPV4_VERSION ) {
        if ( memcmp( args->req_ip_dest, zeros, sizeof(uint32_t) ) != 0) { // If 0, required ip isn't given
            if ( memcmp( args->req_ip_dest, &ips->daddr, sizeof(uint32_t) ) != 0 ) {
                return 0;
            }
        }
        if ( memcmp( args->req_ip_source, zeros, sizeof(uint32_t) ) != 0) {
            if ( memcmp( args->req_ip_source, &ips->saddr, sizeof(uint32_t) ) != 0 ) {
                return 0;
            }
        }
        ip_hdr_offset = ips->ihl*4;
    }
    else {
        ips6 = (struct ipv6hdr*)( packet + sizeof(struct ethhdr) );
        if ( memcmp( args->req_ip_dest, zeros, MAX_ADDR_SZ ) != 0) {
            if ( memcmp( args->req_ip_dest, &ips6->daddr, MAX_ADDR_SZ ) != 0 ) {
                return 0;
            }
        }
        if ( memcmp( args->req_ip_source, zeros, MAX_ADDR_SZ ) != 0) {
            if ( memcmp( args->req_ip_source, &ips6->saddr, MAX_ADDR_SZ ) != 0 ) {
                return 0;
            }
        }
        ip_hdr_offset = sizeof(struct ipv6hdr);
    } 
  
    // check udp header
    struct udphdr* udp;
    udp = (struct udphdr*)( packet + sizeof(struct ethhdr) + ip_hdr_offset );
  
    if ( args->req_port_dest ) {
        if ( args->req_port_dest != ntohs(udp->dest) ) {
            return 0;
        }
    }
    if ( args->req_port_source ) {
        if ( args->req_port_source != ntohs(udp->source) ) {
            return 0;
        }
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
    struct sockaddr_ll int_to_bind;

    sniff_args_t* args = (sniff_args_t*)args_struct_ptr;
    socklen_t info_len = sizeof(struct sockaddr_ll);

    memset( &addr_info, 0, sizeof(struct sockaddr_ll) );
    memset( &rq, 0, sizeof(struct ifreq) );
    memset( &mreq, 0, sizeof(struct packet_mreq) );
    memset( &int_to_bind, 0, sizeof(struct sockaddr_ll) );

    uint8_t* buffer = (uint8_t*)malloc(USHRT_MAX); // 65535 for max size of udp packet
    if (!buffer) {
      THREAD_ERROR_RETURN("Malloc error!\n");
    }

    SOCKET raw_socket = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );
    if ( NOTVALIDSOCKET(raw_socket) ) {
      THREAD_ERROR_RETURN("Error in socket creation\n");
    }

    int_to_bind.sll_ifindex = args->interface;
    int_to_bind.sll_family = AF_PACKET;
    int_to_bind.sll_protocol = htons(ETH_P_IP);

    if ( bind(raw_socket, 
              (struct sockaddr*)&int_to_bind, 
              sizeof(struct sockaddr_ll)) < 0 ) {
        THREAD_ERROR_RETURN("Can't bind to interface!");
    }

    mreq.mr_ifindex = args->interface;
    mreq.mr_type = PACKET_MR_PROMISC;

    if ( setsockopt(raw_socket, 
                    SOL_PACKET, 
                    PACKET_MR_PROMISC,
                    (void*)&mreq, 
                    (socklen_t)sizeof(struct packet_mreq)) < 0 ) {
        THREAD_ERROR_RETURN("Can't turn socket to promiscuous mode!");
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
                THREAD_ERROR_RETURN("Error in recvfrom!");
            }
            else {
                continue;
            }
        }

        if ( packet_meets_reqs(args,  
                               buffer, 
                               &addr_info)) {
            pthread_mutex_lock(args->pkt_mtx);
            *(args->pkt_len_ptr) += pkt_len;
            *(args->pkt_num_ptr) += 1;
            pthread_mutex_unlock(args->pkt_mtx);
        }
    }

    free(buffer);
    CLOSESOCKET(raw_socket);

    return (void *) 1;
}
