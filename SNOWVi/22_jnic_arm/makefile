CC=gcc
CFLAGS=-march=native -O2

default: check_compiler compile

compile: SNOWVi_arm.c SNOWVi_arm_imp.c
	$(CC) SNOWVi_arm.c $(CFLAGS) -o snowvi_base.out
	$(CC) SNOWVi_arm_imp.c $(CFLAGS) -o snowvi_imp.out

check_compiler:
	@which $(CC) > /dev/null || (echo "ERROR: $(CC) is not installed" && exit 1)

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out