all: count_bits

count_bits: count_bits.c Makefile
	gcc -std=c11 -O2 count_bits.c -o count_bits
