#include "./includes/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void perror_exit(const char *msg)
{
    perror(msg);   // prints: <msg>: <system error>
    exit(EXIT_FAILURE);
}
