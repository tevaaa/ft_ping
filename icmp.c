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
    icmp->un.echo.id = id;
    icmp->un.echo.sequence = seq;
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

void send_packet(int sockfd, struct sockaddr_in *dest, int id, int seq) {
    char packet[64];
    int packet_len = build_icmp_packet(packet, id, seq);

    ssize_t sent = sendto(sockfd, packet, packet_len, 0, (struct sockaddr *)dest, sizeof(*dest));

    if (sent < 0)
        perror("sendto");
    else
        printf("✅ ICMP echo request sent (seq=%d)\n", seq);

}

void receive_packet(int sockfd) {
    char buffer[1024];
    struct sockaddr_in sender;
    socklent_t sender_len = sizeof(sender);

    ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender, &sender_len);
    //buffer = [ IP HEADER ][ ICMP HEADER ][ ICMP DATA ]


    if (received < 0) {
        perror("recvfrom");
        return;
    }

    // Parse IP header
    struct ip *ip_header = (struct ip*)buffer;
    int ip_header_len = ip_header->ip_hl * 4; // *4 -> we received the amount of 32bits (4 octets) we have in the header 5 = 5*4 = 20 octets

    // Parse ICMP header
    struct icmphdr *icmp = (struct icmphdr *)(buffer + ip_header_len);

    if (icmp->type == ICMP_ECHOREPLY)
    {
        printf("✅ Received ICMP echo reply from %s\n", inet_ntoa(sender.sin_addr));
        printf("    type=%d code=%d id=%d seq=%d\n",
               icmp->type, icmp->code,
               ntohs(icmp->un.echo.id),
               ntohs(icmp->un.echo.sequence));
    }
    else
    {
        printf("⚠️ Received non-echo packet (type=%d code=%d)\n", icmp->type, icmp->code);
    }
}


// receive_packet()