#include "./includes/icmp.h"

/*
struct icmphdr {
  uint8_t  type;      // 8 pour echo request
  uint8_t  code;      // 0
  uint16_t checksum;  // À calculer
  uint16_t id;        // getpid()
  uint16_t sequence;  // Incrémenté à chaque ping
};*/

int build_icmp_packet(char *buf, int id, int seq)
{
    struct icmphdr *icmp = (struct icmphdr *)buf;

    memset(icmp, 0, sizeof(struct icmphdr));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(id);
    icmp->un.echo.sequence = htons(seq);
    icmp->checksum = 0;

    icmp->checksum = checksum((void *)icmp, sizeof(struct icmphdr));
    return sizeof(struct icmphdr);
}

uint16_t checksum(void *data, int len) {
    uint32_t sum = 0;
    uint16_t *ptr = data;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    if (len == 1)
        sum += *(uint8_t *)ptr;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

void send_packet(int sockfd, struct sockaddr_in *dest, int id, int seq, struct timeval *send_time) {
    char packet[64];
    int packet_len = build_icmp_packet(packet, id, seq);

    gettimeofday(send_time, NULL);

    ssize_t sent = sendto(sockfd, packet, packet_len, 0, (struct sockaddr *)dest, sizeof(*dest));

    if (sent < 0)
        perror("sendto");
}

double receive_packet(int sockfd, struct timeval *send_time, int verbose) {
    char buffer[1024];
    struct sockaddr_in sender;
    struct timeval recv_time;

    struct iovec iov = { .iov_base = buffer, .iov_len = sizeof(buffer) };
    struct msghdr msg = { 
        .msg_name = &sender,
        .msg_namelen = sizeof(sender),
        .msg_iov = &iov, 
        .msg_iovlen = 1 
    };

    ssize_t nb_bytes = recvmsg(sockfd, &msg, 0);
    gettimeofday(&recv_time, NULL);

    if (nb_bytes < 0) {
        perror("recvmsg");
        return -1;
    }

    // Parse IP header
    struct ip *ip_header = (struct ip*)buffer;
    int ip_header_len = ip_header->ip_hl * 4; // *4 -> we received the amount of 32bits (4 octets) we have in the header 5 = 5*4 = 20 octets

    // Parse ICMP header
    struct icmphdr *icmp = (struct icmphdr *)(buffer + ip_header_len);
    if (icmp->type == ICMP_ECHOREPLY)
    {
        double rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 + 
                     (recv_time.tv_usec - send_time->tv_usec) / 1000.0;

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sender.sin_addr, ip_str, INET_ADDRSTRLEN);
        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n",
            (int)nb_bytes, ip_str, ntohs(icmp->un.echo.sequence), ip_header->ip_ttl, rtt);
        return rtt;
    }
    else {
        if (icmp->type == ICMP_DEST_UNREACH) {
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &sender.sin_addr, ip_str, INET_ADDRSTRLEN);
            
            char *error_msg;
            switch (icmp->code) {
                case ICMP_HOST_UNREACH: error_msg = "Destination Host Unreachable"; break;
                case ICMP_NET_UNREACH: error_msg = "Destination Net Unreachable"; break;
                case ICMP_PORT_UNREACH: error_msg = "Destination Port Unreachable"; break;
                default: error_msg = "Destination Unreachable"; break;
            }
            
            printf("%d bytes from %s: %s\n", (int)nb_bytes, ip_str, error_msg);
        }
        else if (verbose) {
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &sender.sin_addr, ip_str, INET_ADDRSTRLEN);
            printf("From %s: icmp_type=%d icmp_code=%d\n", 
                   ip_str, icmp->type, icmp->code);
        }
    }
    return -1;
}
