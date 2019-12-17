all:BRF-based\ greedy\ approach.c
	gcc -o BRF-based\ greedy\ approach BRF-based\ greedy\ approach.c; ./BRF-based\ greedy\ approach
style:
	astyle --style=kr --indent=spaces=4 --suffix=none *.c;
