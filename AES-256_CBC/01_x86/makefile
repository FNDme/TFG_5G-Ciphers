CC=gcc
CFLAGS=-Wall -march=native

default: check_compiler compile

compile: AES_x86.c
	$(CC) AES_x86.c $(CFLAGS) -o main

check_compiler:
	@which $(CC) > /dev/null || (echo "ERROR: $(CC) is not installed" && exit 1)

check:
	@$(CC) AES_x86.c $(CFLAGS) -o main
	@./main.exe
	@rm -rf *.o *.exe *.out main

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out main
