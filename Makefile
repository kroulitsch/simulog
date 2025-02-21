default:
	gcc logic.c -o logic

valgrind:
	gcc -g logic.c -o logic
	valgrind --show-leak-kinds=all --leak-check=full ./logic

clone:
	rm -rf simulog
	git clone https://github.com/kroulitsch/simulog.git --branch master