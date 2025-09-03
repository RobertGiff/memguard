#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>

#define WORKING_SET_MB 64
#define STRIDE 64
#define NUM_ITERATIONS 100  // <-- Change this to set how many iterations you want

double time_diff_sec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main() {
    // Pin to CPU 0
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &set) != 0) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }

    // Allocate working set
    size_t sz = WORKING_SET_MB * 1024 * 1024;
    char *mem = aligned_alloc(4096, sz);
    if (!mem) {
        perror("aligned_alloc");
        return 1;
    }

    // Initialize memory
    for (size_t i = 0; i < sz; i++) {
        mem[i] = 0xAB;
    }

    volatile uint64_t sum = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        for (size_t i = 0; i < sz; i += STRIDE) {
            sum += mem[i];
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = time_diff_sec(start, end);
    printf("Test duration: %.6f seconds\n", elapsed);
    printf("Final sum: %lu\n", sum);

    free(mem);
    return 0;
}

