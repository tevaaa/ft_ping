#include "./includes/init.h"
#include "./includes/icmp.h"
# include "./includes/config.h"

static volatile int interrupted = 0;

void signal_handler(int sig) {
    (void)sig;
    interrupted = 1;
}

int main(int argc, char **argv)
{
    struct sockaddr_in target_addr;
    int sockfd;
    int seq = 0;
    int packets_sent = 0;
    int packets_received = 0;
    double rtt_min = -1;
    double rtt_max = 0;
    double rtt_sum = 0;
    double rtt_sum_squares = 0;

    t_ping_config config = parse_arguments(argc, argv);
    
    int id = getpid() & 0XFFFF; // 32 -> 16 BITS

    resolve_host(config.target, &target_addr);
    sockfd = create_socket(config.ttl);

    printf("PING %s (%s): %d data bytes", 
        config.target, inet_ntoa(target_addr.sin_addr), config.packetsize);
    if (config.verbose)
            printf(", id 0x%x = %d\n", id, id);
    else
            printf("\n");

   // sockfd = create_socket();
   setup_signal_handler(signal_handler);

    while(!interrupted && (config.count == 0 || seq < config.count)) {
        struct timeval send_time;
        double rtt;

        send_packet(sockfd, &target_addr, id, seq, config.packetsize, &send_time);
        packets_sent ++;

        rtt = receive_packet(sockfd, &send_time, config, id);
        if (rtt > 0) {
            packets_received ++;
            // stats
            if (rtt_min < 0 || rtt < rtt_min) rtt_min = rtt;
            if (rtt > rtt_max) rtt_max = rtt;
            rtt_sum += rtt;
            rtt_sum_squares += rtt * rtt;

        }
        if (seq == config.count - 1 && config.count > 0)
            break;
        seq++;
        usleep(config.interval * 1000000);
    }
    double avg = 0;
    double variance = 0;
    double mdev = 0;

    if (packets_received > 0) {
        avg = rtt_sum / packets_received;
        variance = (rtt_sum_squares / packets_received) - (avg * avg);
        mdev = sqrt(variance);
    }

    printf("--- %s ping statistics ---\n", config.target);
    printf("%d packets transmitted, %d packets received, %0.f%% packet loss\n",
           packets_sent, packets_received, 
           packets_sent > 0 ? (100.0 * (packets_sent - packets_received)) / packets_sent : 0.0);
    if (packets_received > 0)
        printf("round-trip min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", rtt_min, avg, rtt_max, mdev);

    close(sockfd);

    return 0;
}