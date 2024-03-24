#include "sniffer.h"


int sniff( int req_ip_source, int req_ip_dest, int req_port_source, int req_port_dest ) 
{
    uint8_t* buffer = (uint8_t*)malloc(USHRT_MAX); // 65535 for max size of udp packet
    if (!buffer)
    {
        fprintf(stderr, "Malloc error!");
        return -1;
    }
    struct sockaddr s_info;
    socklen_t s_info_len = sizeof(s_info);

    SOCKET raw_socket = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_IP) );

    if (NOTVALIDSOCKET(raw_socket))
    {
        fprintf(stderr, "Socket creation error!");
        return -1;
    }

    while (1)
    {
        struct ethhdr* eth;
        struct iphdr* ips;
        struct udphdr* udp;

        uint8_t mac_dest;
        uint8_t ip_dest;
        uint8_t ip_src;
        uint8_t port_src;
        uint8_t port_dest;

        memset(buffer, 0, USHRT_MAX);

        int pkt_len;
        if ( (pkt_len = recvfrom(raw_socket, buffer, USHRT_MAX, NULL, &s_info, &s_info_len)) == -1 )
        {
            fprintf(stderr, "Recv error!");
            return -1;
        }
        else
        {
            // getting l2 header
            eth = (struct ethhdr*)buffer;
            mac_dest = eth->h_dest;

            // getting ip header
            ips = (struct iphdr*)(buffer + sizeof(struct ethhdr));
            ip_dest = ips->daddr;
            ip_src = ips->saddr;
            if ( req_ip_dest && req_ip_dest != ip_dest )
              continue;
            if ( req_ip_source && req_ip_source != ip_src )
              continue;

            // getting udp header
            udp = (struct udphdr*)(buffer + sizeof(struct ethhdr) + ips->ihl*4 );
            port_src = udp->source;
            port_dest = udp->dest;
            if ( req_port_dest && req_port_dest != port_dest )
              continue;
            if ( req_port_source && req_port_source != port_src )
              continue;
        }
    }

    return 0;
}
