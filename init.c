#include "./includes/init.h"

void resolve_host(const char *target, struct sockaddr_in *addr) {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_RAW; 
    hints.ai_protocol = IPPROTO_ICMP;

    int ret = getaddrinfo(target, NULL, &hints, &res);
    if (ret != 0)
        perror_exit("getaddrinfo");

    *addr = *(struct sockaddr_in *)res->ai_addr;
    freeaddrinfo(res);
}

int create_socket(void)
{
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    printf("Socket created with fd: %d\n", sockfd);
    if (sockfd < 0)
        perror_exit("socket");

    return sockfd;
}

void setup_signal_handler(void (*handler)(int))
{
    if (signal(SIGINT, handler) == SIG_ERR)
        perror_exit("signal");
}
