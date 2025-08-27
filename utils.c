#include "./includes/utils.h"
#include "./includes/config.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

void perror_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_usage(char *program_name) {
    printf("Usage: %s [OPTION...] HOST...\n", program_name);
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf("  -c NUMBER                  stop after sending NUMBER packets\n");
    printf("  -i NUMBER                  wait NUMBER seconds between sending each packet\n");
    printf("  -n                         do not resolve host addresses\n");
    printf("  -v                         verbose output\n");
    printf("  -W NUMBER                  NUMBER of seconds to wait for response\n");
    printf("  -s NUMBER                  send NUMBER data octets\n");
    printf("  -?                         give this help list\n");
    exit(0);
}

void print_version(void) {
    printf("ft_ping version 1.0\n");
    exit(0);
}

int is_valid_number(char *str) {
    if (!str || *str == '\0') return 0;
    
    if (*str == '-') str++;
    
    while (*str) {
        if (*str != '.' && (*str < '0' || *str > '9'))
            return 0;
        str++;
    }
    return 1;
}

t_ping_config parse_arguments(int argc, char **argv) {
    t_ping_config config = {0};
    
    // Valeurs par défaut
    config.interval = 1.0;
    config.timeout = 1;
    config.packetsize = 56;
    config.count = 0;
    
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-v") == 0) {
            config.verbose = 1;
        }

        else if (strcmp(argv[i], "-?") == 0) {
            print_usage(argv[0]);
        }

        else if (strcmp(argv[i], "-c") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "%s: option requires an argument -- c\n", argv[0]);
                exit(2);
            }
            if (!is_valid_number(argv[i])) {
                fprintf(stderr, "%s: invalid count: '%s'\n", argv[0], argv[i]);
                exit(2);
            }
            config.count = atoi(argv[i]);
            if (config.count <= 0) {
                fprintf(stderr, "%s: invalid count: %d\n", argv[0], config.count);
                exit(2);
            }
        }
        // Flag -i (interval)
        else if (strcmp(argv[i], "-i") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "%s: option requires an argument -- i\n", argv[0]);
                exit(2);
            }
            if (!is_valid_number(argv[i])) {
                fprintf(stderr, "%s: invalid interval: '%s'\n", argv[0], argv[i]);
                exit(2);
            }
            config.interval = atof(argv[i]);
            if (config.interval < 0) {
                fprintf(stderr, "%s: invalid interval: %.2f\n", argv[0], config.interval);
                exit(2);
            }
        }
        // Option -W (timeout)
        else if (strcmp(argv[i], "-W") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "%s: option requires an argument -- W\n", argv[0]);
                exit(2);
            }
            if (!is_valid_number(argv[i])) {
                fprintf(stderr, "%s: invalid timeout: '%s'\n", argv[0], argv[i]);
                exit(2);
            }
            config.timeout = atoi(argv[i]);
            if (config.timeout <= 0) {
                fprintf(stderr, "%s: invalid timeout: %d\n", argv[0], config.timeout);
                exit(2);
            }
        }
        // Option -n (numeric, pas de reverse DNS)
        else if (strcmp(argv[i], "-n") == 0) {
            config.numeric = 1;
        }
        // Option -s (packet size)
        else if (strcmp(argv[i], "-s") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "%s: option requires an argument -- s\n", argv[0]);
                exit(2);
            }
            if (!is_valid_number(argv[i])) {
                fprintf(stderr, "%s: invalid packet size: '%s'\n", argv[0], argv[i]);
                exit(2);
            }
            config.packetsize = atoi(argv[i]);
            if (config.packetsize < 0 || config.packetsize > 65399) {
                fprintf(stderr, "%s: invalid packet size: %d\n", argv[0], config.packetsize);
                exit(2);
            }
        }
        else if (argv[i][0] == '-' && argv[i][1] != '\0' && argv[i][2] != '\0') {
            char *flags = argv[i] + 1;
            while (*flags) {
                if (*flags == 'v') config.verbose = 1;
                else if (*flags == 'n') config.numeric = 1;
                else {
                    fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], *flags);
                    fprintf(stderr, "Try '%s -?' for more information.\n", argv[0]);
                    exit(2);
                }
                flags++;
            }
        }
        else if (argv[i][0] != '-') {
            config.target = argv[i];
            break;
        }
        else {
            fprintf(stderr, "%s: invalid option -- '%s'\n", argv[0], argv[i] + 1);
            fprintf(stderr, "Try '%s -?' for more information.\n", argv[0]);
            exit(2);
        }
        i++;
    }
    
    if (!config.target) {
        fprintf(stderr, "%s: missing host operand\n", argv[0]);
        fprintf(stderr, "Try '%s -?' for more information.\n", argv[0]);
        exit(2);
    }
    
    return config;
}