CC=gcc
CFLAGS=-march=native

ifeq ($(OS),Windows_NT)
	DIFF = fc
else
	DIFF = diff
endif

default: compile

compile: ROCCA_arm.c
	$(CC) ROCCA_arm.c $(CFLAGS) -o main

test: compile
	./main
	rm -rf *.o *.exe *.out main

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out main