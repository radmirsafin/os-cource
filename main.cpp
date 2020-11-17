#include <iostream>
#include <cstdio>
#include <malloc.h>
#include <cmath>
#include <ctime>
#include <cstdlib>


int metadata_size = 0;
struct metadata {
    struct metadata *next;
    struct metadata *prev;
    bool is_free;
    int size;
};

struct metadata *memory_blocks = NULL;

void print_memory_map() {
    struct metadata *current = memory_blocks;
    printf("\n=> Memory map\n");
    while (current != NULL) {
        printf("metadata { address=%p, next=%p, prev=%p, size=%d, is_free=%d, end=%p}\n",
               current, current->next, current->prev, current->size, current->is_free,
               reinterpret_cast<unsigned char *>(current) + metadata_size + current->size);
        current = current->next;
    }
}

void mysetup(void *buf, int size) {
    printf("Initialize memory: size=%d, start_address=%p \n", size, buf);

    metadata_size = sizeof(struct metadata);

    struct metadata *initial = static_cast<metadata *>(buf);
    initial->next = NULL;
    initial->prev = NULL;
    initial->size = size - metadata_size;
    initial->is_free = true;

    memory_blocks = static_cast<metadata *>(buf);
}

void *myalloc(int size) {
    struct metadata *current = memory_blocks;
    while (current != NULL) {
        if (current->is_free) {
            if (current->size == size) {
                current->is_free = false;
                return reinterpret_cast<unsigned char *>(current) + metadata_size;
            } else if (current->size > metadata_size + size) {
                current->size -= metadata_size + size + 1;

                unsigned char *alloc_block_addr =
                        reinterpret_cast<unsigned char *>(current) + metadata_size + current->size + 1;
                struct metadata *new_block = reinterpret_cast<metadata *>(alloc_block_addr);
                new_block->next = current->next;
                new_block->prev = current;
                new_block->size = size;
                new_block->is_free = false;

                if (current->next != NULL) {
                    current->next->prev = new_block;
                }
                current->next = new_block;

                return alloc_block_addr + metadata_size;
            }
        }
        current = current->next;
    }
    return NULL;
}

struct metadata *merge_blocks(struct metadata *first, struct metadata *second) {
    first->size += metadata_size + second->size + 1;
    if (second->next != NULL) {
        second->next->prev = first;
    }
    first->next = second->next;
    return first;
}

void myfree(void *p) {
    unsigned char *free_block_addr = reinterpret_cast<unsigned char *>(p) - metadata_size;
    struct metadata *free_block = reinterpret_cast<metadata *>(free_block_addr);
    free_block->is_free = true;

    while (free_block->next && free_block->next->is_free) {
        free_block = merge_blocks(free_block, free_block->next);
    }

    while (free_block->prev && free_block->prev->is_free) {
        free_block = merge_blocks(free_block->prev, free_block);
    }
}

int get_free_size() {
    int free = 0;
    struct metadata *current = memory_blocks;
    while (current != NULL) {
        if (current->is_free) {
            free += current->size;
        }
        current = current->next;
    }
    return free;
}

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

    if (max_size_percent >= 0.9) {
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

    if (effective_size_percent >= 0.9) {
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
    max_size_test();
    effective_size_test();
    allocation_test();
    return 0;
}
