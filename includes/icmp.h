#ifndef ICMP_H
# define ICMP_H
# define MAX_PACKET_SIZE 65399 + sizeof(struct icmphdr)


# include <netinet/in.h> // sockaddr_in
# include <stdio.h>
# include <string.h>
# include <sys/socket.h>
# include <netinet/ip_icmp.h>
# include <netinet/ip.h>
# include <arpa/inet.h>
# include <sys/time.h>
# include <sys/uio.h>

int  build_icmp_packet(char *buf, int id, int seq, int data_size);
uint16_t checksum(void *data, int len);
void send_packet(int sockfd, struct sockaddr_in *dest, int id, int seq, int data_size, struct timeval *send_time);
double receive_packet(int sockfd, struct timeval *send_time, int verbose, int timeout_sec);

#endif
