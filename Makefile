default:
	gcc logic.c -o logic

valgrind:
	gcc -g logic.c -o logic
	valgrind --show-leak-kinds=all --leak-check=full ./logic