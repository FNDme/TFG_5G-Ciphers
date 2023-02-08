CC=g++
CFLAGS=-march=tigerlake -mavx
ifeq ($(OS),Windows_NT)
	DIFF = fc
else
	DIFF = diff
endif

default: compile

compile: SNOWV_x86.cc
	$(CC) SNOWV_x86.cc $(CFLAGS) -o main

test: compile
	./main

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out main