CC=gcc 
CFLAGS=-Wall -march=native

default: check_compiler compile

compile: SNOWVi_arm.c
	$(CC) SNOWVi_arm.c $(CFLAGS) -o main

check_compiler:
	@which $(CC) > /dev/null || (echo "ERROR: $(CC) is not installed" && exit 1)

check:
	@$(CC) SNOWVi_arm.c $(CFLAGS) -o main
	@./main
	@rm -rf *.o *.exe *.out main

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out main
