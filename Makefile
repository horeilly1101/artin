CFLAGS = -Wall -Wextra -g

default: program

program:
	gcc src/frac.c src/matrix.c src/main.c -o artin $(CFLAGS)