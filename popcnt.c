#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define NTHREADS 8

typedef struct {
    u_int64_t* start;
    u_int64_t length;
    u_int64_t result;
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
    u_int64_t* start = (u_int64_t*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, input_fd, 0);
    u_int64_t* end = start + file_size / sizeof(start);

    pthread_t tids[NTHREADS];
    Work work[NTHREADS];

    u_int64_t* iterator = start;
    u_int64_t chunk_size = (end - start) / NTHREADS;
    for (int i = 0; i < NTHREADS; i++) {
        work[i].start = iterator;
        work[i].length = chunk_size;
        work[i].result = 0;
        pthread_create(&tids[i], NULL, count_bits, &work[i]);
        iterator += chunk_size;
    }

    // Remove edge case from loop for avoiding dependent instructions in tight loop
    size_t pending_bytes = file_size - ((iterator - start) * sizeof(iterator));

    unsigned char* char_iterator = (unsigned char*)iterator;
    unsigned char* char_iterator_end = char_iterator + pending_bytes;

    u_int64_t ones = 0;
    while (char_iterator != char_iterator_end) {
        ones += __builtin_popcount(*char_iterator);
        char_iterator++;
    }

    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(tids[i], NULL);
        ones += work[i].result;
    }
    munmap(start, file_size);

    size_t total_bits = file_size * CHAR_BIT;
    printf("# of 0s: %lu\n", total_bits - ones);
    printf("# of 1s: %lu\n", ones);
    printf("1/0 ratio: %f%%\n", ones / (double)total_bits * 100);
}


void* count_bits(void* data) {
    Work* work = (Work*)data;
    u_int64_t* iterator = work->start;
    u_int64_t* end = iterator + work->length;
    u_int64_t ones = 0;
    madvise(iterator, work->length * sizeof(iterator), MADV_SEQUENTIAL);


    while (iterator != end) {
        // with march=native, this translates to single instruction on x86
        ones += __builtin_popcountll(*iterator);
        iterator++;
    }

    work->result = ones;
    return NULL;
}
