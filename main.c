#include "./includes/init.h"
#include "./includes/icmp.h"

int main(int argc, char **argv)
{
    struct sockaddr_in target_addr;
    int sockfd;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    resolve_host(argv[1], &target_addr);
    fprintf(stderr, "Resolved %s to %s\n", argv[1], inet_ntoa(target_addr.sin_addr));
    sockfd = create_socket();

    int id = getpid() & 0XFFFF; // 32 -> 16 BITS
    send_packet(sockfd, &target_addr, id, 1);

    return 0;
}
