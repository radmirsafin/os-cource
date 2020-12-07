#include "simple-allocator.h"

#include <cstdio>

int metadata_size = 0;
struct metadata {
    struct metadata *next;
    struct metadata *prev;
    bool is_free;
    int size;
};

struct metadata *memory_blocks = NULL;

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

struct metadata *merge_blocks(struct metadata *first, struct metadata *second) {
    first->size += metadata_size + second->size + 1;
    if (second->next != NULL) {
        second->next->prev = first;
    }
    first->next = second->next;
    return first;
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
