#include "simple-allocator.h"

#include <iostream>
#include <cstdio>
#include <malloc.h>
#include <cmath>
#include <ctime>
#include <cstdlib>

int max_size_test(bool progress=false) {
    printf("\n========== TEST: max_size_test ==========\n");

    int memory_size = 1048576;

    void *ptr = malloc(memory_size);
    if (ptr == NULL) {
        printf("ERROR: Can't allocate memory.\n");
        return 1;
    }

    mysetup(ptr, memory_size);

    int bytes_to_allocate = 51200;
    int last_allocated = 0;
    double max_size_percent = 0;
    while (true) {
        void *allocated_ptr = myalloc(bytes_to_allocate);
        if (allocated_ptr != NULL) {
            max_size_percent = (double) bytes_to_allocate / memory_size * 100;
            if (progress) {
                printf("Current max size: %f %% (%d / %d)\n", max_size_percent, bytes_to_allocate, memory_size);
            }
            last_allocated = bytes_to_allocate;
            bytes_to_allocate += (memory_size - bytes_to_allocate) / 2;
            myfree(allocated_ptr);
        } else {
            bytes_to_allocate -= (bytes_to_allocate - last_allocated) / 2;
        }

        if (bytes_to_allocate - last_allocated < 1024) {
            break;
        }
    }

    if (max_size_percent >= 90) {
        printf("[PASSED] Max size: %f %%\n", max_size_percent);
        return 0;
    } else {
        printf("[FAILED] Max size: %f %%\n", max_size_percent);
        return 1;
    }
}

int effective_size_test(bool progress=false) {
    printf("\n========== TEST: effective_size_test ==========\n");

    int memory_size = 1048576;

    void *ptr = malloc(memory_size);
    if (ptr == NULL) {
        printf("ERROR: Can't allocate memory.\n");
        return 1;
    }

    mysetup(ptr, memory_size);

    int bytes_to_allocate = 16;
    int memory_allocated = 0;
    double effective_size_percent = 0;
    while (true) {
        void *allocated_ptr = myalloc(bytes_to_allocate);
        if (allocated_ptr != NULL) {
            memory_allocated += bytes_to_allocate;
            effective_size_percent = (double) memory_allocated / memory_size * 100;
            if (progress) {
                printf("Current effective size: %f %% (%d / %d)\n",
                       effective_size_percent, memory_allocated, memory_size);
            }
        } else {
            break;
        }
    }

    if (effective_size_percent >= 30) {
        printf("[PASSED] Effective size: %f %%\n", effective_size_percent);
        return 0;
    } else {
        printf("[FAILED] Effective size: %f %%\n", effective_size_percent);
        return 1;
    }
}

int allocation_test(bool progress=false) {
    printf("\n========== TEST: allocation_test ==========\n");

    srand(static_cast<unsigned int>(time(0)));

    int memory_size = 1048576;
    void *ptr = malloc(memory_size);
    if (ptr == NULL) {
        printf("ERROR: Can't allocate memory.\n");
        return 1;
    }

    mysetup(ptr, memory_size);

    int free_size_before = get_free_size();
    int allocation_count = 50;
    void** allocated = static_cast<void **>(malloc(sizeof(void *) * 10));

    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < allocation_count; j++) {
            int size = rand() % 10000 + 1;
            void *allocated_ptr = myalloc(size);
            if (allocated_ptr == NULL) {
                printf("ERROR: Can't allocate %d bytes", size);
                return 1;
            } else {
                allocated[j] = allocated_ptr;
            }
        }
        for (int j = 0; j < allocation_count; j++) {
            myfree(allocated[j]);
        }

        if (progress && i % 10 == 0) {
            int memory_used_percent = (double) (memory_size - get_free_size()) / memory_size * 100;
            printf("Loop %d. Memory usage: %d %%\n", i, memory_used_percent);
        }
    }

    int free_size_after = get_free_size();
    if (free_size_before == free_size_after) {
        printf("[PASSED] Free size: %d == %d\n", free_size_before, free_size_after);
        return 0;
    } else {
        printf("[FAILED] Memory leaks detected: %d != %d\n", free_size_before, free_size_after);
        return 1;
    }
}

int main() {
    int failed_tests = 0;
    failed_tests += max_size_test();
    failed_tests += effective_size_test();
    failed_tests += allocation_test();
    return failed_tests;
}
