#include <stdio.h>
#include <string.h>

#define BUFSIZE 8192

int main(int argc, char* argv[]) {
	FILE* input;


	if (argc == 2) {
		input = fopen(argv[1], "r");
	} else {
		input = stdin;
	}

	unsigned char buffer[BUFSIZE];
	int bytes_read = 0;
	unsigned long long ones = 0;
	unsigned long long total_bits = 0;

	while((bytes_read = fread(buffer, 1, sizeof(buffer), input)) != 0) {
		for (int i = 0; i < bytes_read; i++) {
			ones += __builtin_popcount(buffer[i]);
			total_bits += 8;
		}
	}
	printf("1/0 ratio: %f%%\n", ones / (double) total_bits * 100);
}
