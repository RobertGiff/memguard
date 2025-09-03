#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <sys/syscall.h>
#include <errno.h>

#define WORKING_SET_MB 64
#define STRIDE 64
#define NUM_ITERATIONS 100

#define gettid() syscall(SYS_gettid)

// Definition copied from include/uapi/linux/sched/types.h
struct sched_attr {
    uint32_t size;
    uint32_t sched_policy;
    uint64_t sched_flags;
    int32_t  sched_nice;
    uint32_t sched_priority;
    /* SCHED_DEADLINE specific fields: */
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
};

#define SCHED_DEADLINE 6

// Custom syscall wrapper
int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags) {
    return syscall(SYS_sched_setattr, pid, attr, flags);
}

double time_diff_sec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main() {
    // Set up SCHED_DEADLINE
    struct sched_attr attr = {
        .size = sizeof(struct sched_attr),
        .sched_policy = SCHED_DEADLINE,
        .sched_flags = 0,
        .sched_nice = 0,
        .sched_priority = 0,
        .sched_runtime = 99 * 1000 * 1000,  // 99 ms
        .sched_deadline = 99 * 1000 * 1000, // 99 ms
        .sched_period = 99 * 1000 * 1000    // 99 ms
    };

    // Pin to CPU 0
    //cpu_set_t set;
    //CPU_ZERO(&set);
    //CPU_SET(0, &set);
    //if (sched_setaffinity(0, sizeof(cpu_set_t), &set) != 0) {
    //    perror("sched_setaffinity");
    //    exit(EXIT_FAILURE);
    //}

    if (sched_setattr(0, &attr, 0) != 0) {
        perror("sched_setattr");
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

