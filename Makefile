CC = gcc
CFLAGS = -Wall -Wextra -g
PROG = main

C_FILES = $(wildcard *.c)
OBJS = $(C_FILES:.c=.o)
H_FILES = $(wildcard *.h)

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS): $(H_FILES)

v: $(PROG)
	valgrind --show-leak-kinds=all --leak-check=full ./$(PROG)

clone:
	cd .. && rm -rf simulog && git clone https://github.com/kroulitsch/simulog.git --branch main

pull:
	git pull

clean:
	rm -f $(PROG) $(OBJS)

run: $(PROG)
	./$(PROG)

.PHONY: all v clone pull clean run