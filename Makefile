NAME = ft_ping

CC = cc
CFLAGS = -Wall -Wextra

SRC = main.c init.c utils.c icmp.c

all: $(NAME)

$(NAME): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(NAME) -lm

clean:
	rm -f $(NAME)

re: clean all

.PHONY: all clean re