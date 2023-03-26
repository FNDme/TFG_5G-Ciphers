CC=gcc
CFLAGS=-march=native -O2
ifeq ($(OS),Windows_NT)
	DIFF = fc
else
	DIFF = diff
endif

default: check_compiler compile

compile: SNOWVi_x86.c
	$(CC) SNOWVi_x86.c $(CFLAGS) -o snowvi

check_compiler:
	@which $(CC) > /dev/null || (echo "ERROR: $(CC) is not installed" && exit 1)

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out snowvi.exe