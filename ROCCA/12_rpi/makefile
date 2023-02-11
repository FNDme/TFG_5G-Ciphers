CC=gcc
CFLAGS=-march=native
ifeq ($(OS),Windows_NT)
	DIFF = fc
else
	DIFF = diff
endif

default: compile

compile: ROCCA_arm.c
	$(CC) ROCCA_arm.c $(CFLAGS) -o rocca.out

test: compile
	./rocca.out
	rm -rf *.o *.exe *.out rocca.out
	
.PHONY: clean
clean:
	rm -rf *.o *.exe *.out rocca.out