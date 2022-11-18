CC=gcc
CFLAGS=-march=native
ifeq ($(OS),Windows_NT)
	DIFF = fc
else
	DIFF = diff
endif

default: check_compiler compile check_output

compile: SNOWVi_x86.cc
	$(CC) SNOWVi_x86.cc $(CFLAGS) -o main

check_compiler:
	@which $(CC) > /dev/null || (echo "ERROR: $(CC) is not installed" && exit 1)

check_output:
	@rm expected -f
	@echo "50 17 19 e1 75 e4 9f b7 41 ba bf 6b a5 de 60 fe " >> expected
	@echo "cd a8 b3 4d 7e c4 c6 42 97 55 c1 9d 2f 67 18 71 " >> expected
	@echo "89 57 d3 26 cb 46 50 2c eb 81 4c cd 6e a5 3a ae " >> expected
	@echo "dd 6c 92 fb f3 92 1e 8b d7 31 7b e2 20 15 31 bb " >> expected
	@echo "09 3e e8 72 e9 eb 40 34 e9 b7 1a 4a c2 b5 4b d9 " >> expected
	@echo "f0 0f 5a dc 06 d2 e6 b5 9f b7 5a 01 be f6 13 14 " >> expected
	@echo "1c 8a b2 02 ee 38 e2 85 0c ca 60 6a b8 75 cd 12 " >> expected
	@echo "41 03 b3 2f a5 14 5d df 54 e7 a0 7b 0f 3e b7 7a " >> expected
	@./main > actual
	@$(DIFF) expected actual > /dev/null 2>&1 && echo "Output is correct" || echo "Output is wrong"
	@rm expected actual -f

.PHONY: clean
clean:
	rm -rf *.o *.exe *.out main