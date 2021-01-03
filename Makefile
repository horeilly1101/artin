CFLAGS = -Wall -Wextra -g

default: program

program:
	gcc src/matrix.c src/main.c -o artin $(CFLAGS)