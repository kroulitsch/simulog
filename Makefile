CC = gcc
PROG = logic

all: $(PROG).c
	$(CC) $(PROG).c -o $(PROG)

v: $(PROG)
	valgrind --show-leak-kinds=all --leak-check=full ./$(PROG)

clone:
	cd ..
	rm -rf simulog
	git clone https://github.com/kroulitsch/simulog.git --branch master

pull:
	git pull

clean:
	rm -f $(PROG)

run:
	./$(PROG)