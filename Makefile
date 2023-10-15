
CC=gcc
CFLAGS=-g3 -Wall
FILE=main

all:
	$(CC) -o $(FILE) $(CFLAGS) $(FILE).c
clean:
	rm -f $(FILE)
