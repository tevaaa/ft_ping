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

/*The sendto() function shall send a message through a connection-mode or connectionless-mode socket. If the socket is connectionless-mode, the message shall be sent to the address specified by dest_addr. If the socket is connection-mode, dest_addr shall be ignored.

The sendto() function takes the following arguments:

- socket
Specifies the socket file descriptor.
- message
Points to a buffer containing the message to be sent.
- length
Specifies the size of the message in bytes.
- flags
Specifies the type of message transmission.
dest_addr
Points to a sockaddr structure containing the destination address. The length and format of the address depend on the address family of the socket.
dest_len
Specifies the length of the sockaddr structure pointed to by the dest_addr argument.
*/

void send_packet(int sockfd, struct sockaddr_in *dest, int id, int seq, struct timeval *send_time) {
    char packet[64];
    int packet_len = build_icmp_packet(packet, id, seq);

    gettimeofday(send_time, NULL);

    ssize_t sent = sendto(sockfd, packet, packet_len, 0, (struct sockaddr *)dest, sizeof(*dest));

    if (sent < 0)
        perror("sendto");
    //else
    //    printf("✅ ICMP echo request sent (seq=%d)\n", seq);

}

double receive_packet(int sockfd, struct timeval *send_time) {
    char buffer[1024];
    struct sockaddr_in sender;
    struct timeval recv_time;

    socklen_t sender_len = sizeof(sender);

    ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender, &sender_len);
    gettimeofday(&recv_time, NULL);

    if (received < 0) {
        perror("recvfrom");
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
        printf("64 bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n", 
               ip_str, ntohs(icmp->un.echo.sequence), ip_header->ip_ttl, rtt);
        return rtt;
    }
    return -1;
}


// receive_packet()