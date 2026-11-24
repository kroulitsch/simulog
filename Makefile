CC = gcc
INCDIR = include
SRCDIR = src
APIDIR = api
CFLAGS = -Wall -Wextra -g -I$(INCDIR)

PROG = main
API = api_app

C_FILES = $(wildcard $(SRCDIR)/*.c)
SRC_OBJS = $(C_FILES:.c=.o)

MAIN_OBJS = $(PROG).o $(SRC_OBJS)
API_OBJS = $(APIDIR)/api.o $(OBJS)
H_FILES = $(wildcard $(INCDIR)/*.h) $(wildcard $(APIDIR)/*.h)

all: $(PROG)

$(PROG): $(MAIN_OBJS)
	$(CC) $(CFLAGS) $(MAIN_OBJS) -o $(PROG)

$(API): $(API_OBJS)
	$(CC) $(CFLAGS) $(API_OBJS) -o $(API)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(APIDIR)/%.o: $(APIDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MAIN_OBJS) $(API_OBJS): $(H_FILES)

v: $(PROG)
	valgrind --show-leak-kinds=all --leak-check=full ./$(PROG)

clean:
	rm -f $(PROG) $(API) $(MAIN_OBJS) $(API_OBJS) *.dot

run: $(PROG)
	./$(PROG)

api: $(API)
	./$(API)

.PHONY: all v clean run api