CC = gcc
CFLAGS = -g3 -std=c99 -pedantic -Wall -Wextra

.PHONY: all clean
all: main
main: main.o getarg.h
	$(CC) -o main main.c $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f main.o main
