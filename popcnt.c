#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define NTHREADS 6

typedef struct {
    unsigned long long* start;
    unsigned long long length;
    unsigned long long result;
} Work;

void* count_bits(void* work);


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s inputfile", argv[0]);
        exit(1);
    }

    int input_fd = open(argv[1], O_RDONLY);
    struct stat file_stat;
    fstat(input_fd, &file_stat);
    size_t file_size = file_stat.st_size;

    // Read 64 bits = 8 bytes at a time
    unsigned long long* start =
        (unsigned long long*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, input_fd, 0);
    unsigned long long* end = start + file_size / 8;

    madvise(start, file_size, MADV_SEQUENTIAL);

    pthread_t tids[NTHREADS];
    Work work[NTHREADS];

    unsigned long long chunk_size = (end - start) / NTHREADS;
    for (int i = 0; i < NTHREADS; i++) {
        work[i].start = start + (chunk_size * i);
        work[i].length = chunk_size;
        work[i].result = 0;
        pthread_create(&tids[i], NULL, count_bits, &work[i]);
    }

    // Remove edge case from loop for avoiding dependent instructions in tight loop
    unsigned long long* last_work = start + chunk_size * NTHREADS;
    size_t pending_bytes = file_size - ((last_work - start) * 8);

    unsigned char* char_iterator = (unsigned char*)last_work;
    unsigned char* char_iterator_end = char_iterator + pending_bytes;

    unsigned long long ones = 0;
    while (char_iterator != char_iterator_end) {
        ones += __builtin_popcount(*char_iterator);
        char_iterator++;
    }

    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(tids[i], NULL);
        ones += work[i].result;
    }
    munmap(start, file_size);

    size_t total_bits = file_size * 8;
    printf("# of 0s: %llu\n", total_bits - ones);
    printf("# of 1s: %llu\n", ones);
    printf("1/0 ratio: %f%%\n", ones / (double)total_bits * 100);
}


void* count_bits(void* data) {
    Work* work = (Work*)data;
    unsigned long long* iterator = work->start;
    unsigned long long* end = iterator + work->length;
    unsigned long long ones = 0;

    while (iterator != end) {
        // with march=native, this translates to single instruction on x86
        ones += __builtin_popcountll(*iterator);
        iterator++;
    }

    work->result = ones;
    return NULL;
}
