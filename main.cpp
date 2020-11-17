#include "iostream"
#include "cstdio"
#include "malloc.h"
#include "cmath"

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
    printf("Size of metadata in bytes: %d \n", metadata_size);

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

void test_max_size() {
    int memory_size = 102400;

    void *ptr = malloc(memory_size);
    if (ptr == NULL) {
        printf("Can't allocate memory.\n");
    } else {
        printf("Test memory allocated: Start address: %p.\n", ptr);
    }

    mysetup(ptr, memory_size);

    printf("[max_size_test] Memory state:");
    print_memory_map();

    int bytes_to_allocate = 51200;
    int last_allocated = 0;
    while (true) {
        void *allocated_ptr = myalloc(bytes_to_allocate);
        if (allocated_ptr != NULL) {
            printf("[max_size_test] Current max size: %f %% (%d / %d)\n",
                   (double) bytes_to_allocate / memory_size * 100, bytes_to_allocate, memory_size);

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

    printf("[max_size_test] Test finished. Max size: %f %% (%d / %d)\n",
           (double) bytes_to_allocate / memory_size * 100, bytes_to_allocate, memory_size);

    printf("[max_size_test] Memory state:");
    print_memory_map();

}

int main() {
    test_max_size();
    return 0;
}
