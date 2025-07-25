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
    double rtt_min = -1;
    double rtt_max = 0;
    double rtt_sum = 0;
    double rtt_sum_squares = 0; 

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    resolve_host(argv[1], &target_addr);
    printf("PING %s (%s) 56(84) bytes of data.\n", argv[1], inet_ntoa(target_addr.sin_addr));

   sockfd = create_socket();
   setup_signal_handler(signal_handler);

    int id = getpid() & 0XFFFF; // 32 -> 16 BITS

    while(!interrupted) {
        struct timeval send_time;
        double rtt;

        send_packet(sockfd, &target_addr, id, seq, &send_time);
        packets_sent ++;

        rtt = receive_packet(sockfd, &send_time);
        if (rtt > 0) {
            packets_received ++;
            // stats
            if (rtt_min < 0 || rtt < rtt_min) rtt_min = rtt;
            if (rtt > rtt_max) rtt_max = rtt;
            rtt_sum += rtt;
            rtt_sum_squares += rtt * rtt;

        }

        seq++;
        sleep(1);

    }
    double avg = rtt_sum / packets_received;
    double variance = (rtt_sum_squares / packets_received) - (avg * avg);
    double mdev = sqrt(variance);

    printf("\n--- ping statistics ---\n");
    printf("%d packets transmitted, %d received, %.1f%% packet loss\n",
           packets_sent, packets_received, 
           packets_sent > 0 ? (100.0 * (packets_sent - packets_received)) / packets_sent : 0.0);
    printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", rtt_min, avg, rtt_max, mdev);


    close(sockfd);

    return 0;
}
