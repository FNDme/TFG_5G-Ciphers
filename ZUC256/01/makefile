CC=gcc
CFLAGS=

default: compile

compile: ZUC256.c
	$(CC) ZUC256.c $(CFLAGS) -o main

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out main