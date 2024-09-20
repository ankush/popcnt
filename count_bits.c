#include <stdio.h>
#include <string.h>

#define BUFSIZE 1024

int main(int argc, char* argv[]) {
	FILE* input;


	if (argc == 2) {
		input = fopen(argv[1], "r");
	} else {
		input = stdin;
	}

	unsigned long long buffer[BUFSIZE];
	int bytes_read = 0;
	unsigned long long ones = 0;
	unsigned long long total_bits = 0;

	while((bytes_read = fread(buffer, sizeof(unsigned long long), BUFSIZE, input)) != 0) {
		for (int i = 0; i < bytes_read; i++) {
			ones += __builtin_popcountll(buffer[i]);
			total_bits += 64;
		}
	}
	printf("1/0 ratio: %f%%\n", ones / (double) total_bits * 100);
}
