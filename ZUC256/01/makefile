CC=gcc
ifeq ($(OS),Windows_NT)
	DIFF = fc
else
	DIFF = diff
endif

default: check_compiler compile check_output

compile: ZUC256.c
	$(CC) ZUC256.c $(CFLAGS) -o main

check_compiler:
	@which $(CC) > /dev/null || (echo "ERROR: $(CC) is not installed" && exit 1)

check_output:
	@rm expected -f
	@echo "e457e206 cee79e16 7da20fd0 3bbb22cc " >> expected
	@echo "a2ec34f0 e4e12c0b 0ad0fb23 6051348a " >> expected
	@echo "f9779552 454c3dbb 397d19b3 28390332 " >> expected
	@echo "11b9ae54 6094770b 5016e134 620ebf4a " >> expected
	@echo "302c9be3 b65db142 2b564caa 9caeca83 " >> expected
	@./main > actual
	@$(DIFF) expected actual > /dev/null 2>&1 && echo "Output is correct" || echo "Output is wrong"
	@rm expected actual -f

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out main