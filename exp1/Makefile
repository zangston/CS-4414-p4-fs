CC = gcc
CFLAGS = -Wall -Wextra

SRCS = fsdump.c
OBJS = $(SRCS:.c=.o)

all: fsdump

fsdump: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) fsdump