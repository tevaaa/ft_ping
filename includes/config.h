#ifndef CONFIG_H
# define CONFIG_H

typedef struct s_ping_config {
    char    *target;           // Hostname/IP cible
    int     verbose;           // -v flag
    int     count;             // -c count (0 = infini)
    double  interval;          // -i interval (secondes)
    int     timeout;           // -W timeout (secondes)  
    int     numeric;           // -n flag (pas de reverse DNS)
    int     packetsize;        // -s size (data octets)
} t_ping_config;

// Fonctions de parsing
t_ping_config   parse_arguments(int argc, char **argv);
void            print_usage(char *program_name);
void            print_version(void);

#endif