CC=gcc
CFLAGS=-march=native

default: check_compiler compile

compile: AES_x86.cc
	$(CC) AES_x86.cc $(CFLAGS) -o aes.out

check_compiler:
	@which $(CC) > /dev/null || (echo "ERROR: $(CC) is not installed" && exit 1)

check:
	@$(CC) AES_x86.cc $(CFLAGS) -o aes.out
	@./aes.out
	@rm -rf *.o *.exe *.out aes.out

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out aes.out
