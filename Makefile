
CFLAGS=--std=c99 -Wall -pedantic -ggdb3

all:
	gcc $(CFLAGS) -o aeq aeq.c -lcurses `pkg-config --cflags --libs jack` -lm

clean:
	rm -rf *.o aeq
