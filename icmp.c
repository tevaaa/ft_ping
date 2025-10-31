#include "./includes/icmp.h"

/*
struct icmphdr {
  uint8_t  type;      // 8 pour echo request
  uint8_t  code;      // 0
  uint16_t checksum;  // À calculer
  uint16_t id;        // getpid()
  uint16_t sequence;  // Incrémenté à chaque ping
};*/

int build_icmp_packet(char *buf, int id, int seq, int data_size)
{
    struct icmphdr *icmp = (struct icmphdr *)buf;
    int total_size = sizeof(struct icmphdr) + data_size;

    memset(buf, 0, total_size);
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(id);
    icmp->un.echo.sequence = htons(seq);
    icmp->checksum = 0;

    if (data_size >= (int)sizeof(struct timeval)) {
        struct timeval *tv = (struct timeval *)(buf + sizeof(struct icmphdr));
        gettimeofday(tv, NULL);
    }

    icmp->checksum = checksum((void *)icmp, total_size);
    return total_size;
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

void send_packet(int sockfd, struct sockaddr_in *dest, int id, int seq, int data_size, struct timeval *send_time) {
    char packet[MAX_PACKET_SIZE];
    int packet_len = build_icmp_packet(packet, id, seq, data_size);

    gettimeofday(send_time, NULL);

    ssize_t sent = sendto(sockfd, packet, packet_len, 0, (struct sockaddr *)dest, sizeof(*dest));

    if (sent < 0)
        perror("sendto");
}

double receive_packet(int sockfd, struct timeval *send_time, t_ping_config config, int our_id) {
    char buffer[MAX_PACKET_SIZE];
    struct sockaddr_in sender;
    struct timeval recv_time;

    struct iovec iov = { .iov_base = buffer, .iov_len = sizeof(buffer) };
    struct msghdr msg = { 
        .msg_name = &sender,
        .msg_namelen = sizeof(sender),
        .msg_iov = &iov, 
        .msg_iovlen = 1 
    };

    struct timeval timeout;
    timeout.tv_sec = config.timeout;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt timeout");
        return -1;
    }

    while (1) {
        ssize_t nb_bytes = recvmsg(sockfd, &msg, 0);
        gettimeofday(&recv_time, NULL);

        if (nb_bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return -1; // Timeout
            }
            perror("recvmsg");
            return -1;
        }

        // Parse IP header
        struct ip *ip_header = (struct ip*)buffer;
        int ip_header_len = ip_header->ip_hl * 4;
        nb_bytes -= ip_header_len;

        // Parse ICMP header
        struct icmphdr *icmp = (struct icmphdr *)(buffer + ip_header_len);
        
        char ip_str[INET_ADDRSTRLEN];
        char display_addr[256];
        
        if (icmp->type == ICMP_ECHO && ntohs(icmp->un.echo.id) == our_id) {
            continue;
        }

        inet_ntop(AF_INET, &sender.sin_addr, ip_str, INET_ADDRSTRLEN);
        
        if (config.numeric) {
            strcpy(display_addr, ip_str);
        } else {
            char hostname[256];
            if (getnameinfo((struct sockaddr*)&sender, sizeof(sender), 
                           hostname, sizeof(hostname), NULL, 0, 0) == 0) {
                strcpy(display_addr, hostname);
            } else {
                strcpy(display_addr, ip_str);
            }
        }
        if (icmp->type == ICMP_TIME_EXCEEDED) {
            if (icmp->code == ICMP_EXC_TTL) {
                inet_ntop(AF_INET, &sender.sin_addr, ip_str, INET_ADDRSTRLEN);
                printf("%d bytes from %s (%s): Time to live exceeded\n", (int)nb_bytes, display_addr, ip_str);
            }
            if (config.verbose) {
                // Le paquet original est après l'en-tête ICMP d'erreur
        struct ip *orig_ip = (struct ip*)((char*)icmp + sizeof(struct icmphdr));
        int orig_ip_len = orig_ip->ip_hl * 4;
        
        // Affiche l'IP header original
        print_hexdump("IP Hdr Dump:", (unsigned char*)orig_ip, orig_ip_len);
                //printf("ICMP: type=%d code=%d from %s\n", icmp->type, icmp->code, ip_str);
            }
            return -1;
        }
        else if (icmp->type == ICMP_ECHOREPLY && ntohs(icmp->un.echo.id) == our_id) {
            double rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 + 
                         (recv_time.tv_usec - send_time->tv_usec) / 1000.0;

            inet_ntop(AF_INET, &sender.sin_addr, ip_str, INET_ADDRSTRLEN);
            printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
                (int)nb_bytes, ip_str, ntohs(icmp->un.echo.sequence), ip_header->ip_ttl, rtt);
            return rtt;
        }
        else if (icmp->type == ICMP_DEST_UNREACH) {
            inet_ntop(AF_INET, &sender.sin_addr, ip_str, INET_ADDRSTRLEN);
            
            char *error_msg;
            switch (icmp->code) {
                case ICMP_HOST_UNREACH: error_msg = "Destination Host Unreachable"; break;
                case ICMP_NET_UNREACH: error_msg = "Destination Net Unreachable"; break;
                case ICMP_PORT_UNREACH: error_msg = "Destination Port Unreachable"; break;
                default: error_msg = "Destination Unreachable"; break;
            }
            
            printf("%d bytes from %s: %s\n", (int)nb_bytes, ip_str, error_msg);
            return -1;
        }
        else if (config.verbose) {
            inet_ntop(AF_INET, &sender.sin_addr, ip_str, INET_ADDRSTRLEN);
            printf("From %s: icmp_type=%d icmp_code=%d\n", 
                   ip_str, icmp->type, icmp->code);
        }
    }
}
