NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRCDIR = .
SRCS = $(SRCDIR)/main.c \
       $(SRCDIR)/utils.c \
       $(SRCDIR)/logger.c \
       $(SRCDIR)/scheduler.c \
       $(SRCDIR)/dongle.c \
       $(SRCDIR)/coder.c \
       $(SRCDIR)/monitor.c \
       $(SRCDIR)/simulation.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
