all: popcnt

popcnt: popcnt.c Makefile
	rm -f popcnt.gcda
	gcc -march=native -g -std=gnu99 -fprofile-generate -O2 popcnt.c -o $@
	./popcnt random.dat
	gcc -march=native -g -std=gnu99 -fprofile-use -O2 popcnt.c -o $@

clean:
	rm -f popcnt popcnt.gcda
