#ifndef ICMP_H
# define ICMP_H

# include <netinet/in.h> // sockaddr_in
# include <stdio.h>
# include <string.h>
# include <sys/socket.h>
# include <netinet/ip_icmp.h>
# include <netinet/ip.h>
# include <arpa/inet.h>

int  build_icmp_packet(char *buf, int id, int seq);
uint16_t checksum(void *data, int len);
void send_packet(int sockfd, struct sockaddr_in *dest, int id, int seq);
void receive_packet(int sockfd);

#endif
