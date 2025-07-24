#include "./includes/init.h"
#include "./includes/icmp.h"

static volatile int interrupted = 0;

void signal_handler(int sig) {
    (void)sig;
    interrupted = 1;  // Juste un flag pour dire "stop"
}

int main(int argc, char **argv)
{
    struct sockaddr_in target_addr;
    int sockfd;
    int seq = 1;
    int packets_sent = 0;
    int packets_received = 0;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    resolve_host(argv[1], &target_addr);
    printf("PING %s (%s): 56 data bytes\n", argv[1], inet_ntoa(target_addr.sin_addr));

   sockfd = create_socket();
   setup_signal_handler(signal_handler);

    int id = getpid() & 0XFFFF; // 32 -> 16 BITS

    while(!interrupted) {
        send_packet(sockfd, &target_addr, id, seq);
        packets_sent ++;

        receive_packet(sockfd);
        packets_received ++;

        seq++;
        sleep(1);

    }
    printf("\n--- ping statistics ---\n");
    printf("%d packets transmitted, %d received, %.1f%% packet loss\n",
           packets_sent, packets_received, 
           packets_sent > 0 ? (100.0 * (packets_sent - packets_received)) / packets_sent : 0.0);

    close(sockfd);

    return 0;
}
