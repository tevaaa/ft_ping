#ifndef INIT_H
# define INIT_H

# include <netinet/in.h> 
# include "init.h"
# include <string.h>        
# include <unistd.h>        
# include <stdlib.h>        
# include <netdb.h>         
# include <arpa/inet.h>     
# include <sys/socket.h>    
# include <netinet/ip_icmp.h> 
# include <stdio.h>         
# include <signal.h>
# include "utils.h"
# include <math.h>

void    resolve_host(const char *target, struct sockaddr_in *addr);
int     create_socket(int ttl);
void    setup_signal_handler(void (*handler)(int));

#endif
