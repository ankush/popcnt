all: count_bits

count_bits: count_bits.c Makefile
	gcc -march=native -g -std=gnu99 -fprofile-use -O2 count_bits.c -o count_bits
