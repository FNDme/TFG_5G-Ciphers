CC=gcc
CFLAGS=-march=native
ifeq ($(OS),Windows_NT)
	DIFF = fc
else
	DIFF = diff
endif

default: compile

compile: ROCCA_x86.c
	$(CC) ROCCA_x86.c $(CFLAGS) -o rocca.out

test: compile
	./rocca.out

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out rocca.out