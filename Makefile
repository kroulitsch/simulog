CC = gcc
CFLAGS = -Wall -Wextra -g
PROG = main

SRCS = $(wildcard *.h) $(wildcard *.c)
C_FILES = $(wildcard *.c)

all: $(PROG)

$(PROG): $(C_FILES)
	$(CC) $(CFLAGS) $(C_FILES) -o $(PROG)

v: $(PROG)
	valgrind --show-leak-kinds=all --leak-check=full ./$(PROG)

clone:
	cd .. && rm -rf simulog && git clone https://github.com/kroulitsch/simulog.git --branch master

pull:
	git pull

clean:
	rm -f $(PROG)

run: $(PROG)
	./$(PROG)