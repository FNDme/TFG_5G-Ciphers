CC=gcc
CFLAGS=-march=native

default: check_compiler compile

compile: SNOWVi_arm.cc
	$(CC) SNOWVi_arm.cc $(CFLAGS) -o main

check_compiler:
	@which $(CC) > /dev/null || (echo "ERROR: $(CC) is not installed" && exit 1)

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out main

