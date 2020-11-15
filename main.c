#include "stdio.h"
#include "stdlib.h"
#include "stddef.h"
#include "stdbool.h"


struct metadata {
    struct metadata *next;
    struct metadata *prev;
    int size;
    bool is_free;
    void *last_address;
};

void print_metadata(struct metadata *meta) {
    printf("struct metadata { address=%p, next=%p, prev=%p, size=%d, is_free=%d, last_address=%p }\n",
           meta, meta->next, meta->prev, meta->size, meta->is_free, meta->last_address);
}

struct metadata *metadata_list = NULL;

void write_metadata(void *address, struct metadata *meta) {
    struct metadata *new_meta_ptr = address;
    *new_meta_ptr = *meta;
    new_meta_ptr->last_address = address + sizeof(struct metadata) + new_meta_ptr->size;
    printf("New metadata written:\n");
    print_metadata(new_meta_ptr);
}

void reduce_block(struct metadata *meta, int size) {
    printf("Reduce block to %d bytes:\n", size);
    print_metadata(meta);
    meta->size -= size;
    meta->last_address = (void*) meta + sizeof(struct metadata) + meta->size;
    printf("Block after reducing:\n");
    print_metadata(meta);
}

void my_setup(void *buf, int size) {
    printf("Initialize memory: size=%d, start_address=%p \n", size, buf);

    struct metadata head;
    head.next = NULL;
    head.prev = NULL;
    head.size = size;
    head.is_free = true;
    head.last_address = NULL;

    printf("Size of metadata record in bytes: %lu\n", sizeof(struct metadata));

    write_metadata(buf, &head);
    metadata_list = buf;
}


void *my_alloc(int size) {
    printf("Allocation request received: size=%d\n", size);

    int memory_need = (int) sizeof(struct metadata) + size + 1;
    printf("Needed memory in bytes: %d\n", memory_need);

    struct metadata *head = metadata_list;
    while (head != NULL) {

        if (head->is_free && head->size > memory_need) {
            printf("Suitable block found: %p\n", head);

            struct metadata new_block_meta;
            new_block_meta.next = head->next;
            new_block_meta.prev = head;
            new_block_meta.size = size;
            new_block_meta.is_free = false;

            reduce_block(head, memory_need);

            void *new_block_meta_ptr = head->last_address + 1;
            write_metadata(new_block_meta_ptr, &new_block_meta);

            if (head->next != NULL) {
                head->next->prev = new_block_meta_ptr;
            }
            head->next = new_block_meta_ptr;


            return new_block_meta_ptr + sizeof(struct metadata) + 1;
        }

        if (head->is_free && head->size == size) {
            printf("Suitable block found: %p\n", head);
            head->is_free = false;
            return head + sizeof(struct metadata) + 1;
        }

        head = head->next;
    }
    return NULL;
}

void my_free(void *ptr)
{
    printf("Free request received: address=%p\n", ptr);
    struct metadata *meta = (struct metadata*) (ptr - sizeof(struct metadata) - 1);
    printf("Try to free block:\n");
    print_metadata(meta);
    meta->is_free = true;
}


void print_memory_map() {
    struct metadata *head = metadata_list;
    printf("\n=== MEMORY MAP ===\n");
    while (head != NULL) {
        print_metadata(head);
        head = head->next;
    }
}


int main() {
    void *ptr = malloc(1024);

    if (ptr == NULL) {
        printf("Memory not allocated.\n");
        exit(0);
    } else {
        printf("Memory allocated: Start address: %p.\n", ptr);
    }

    my_setup(ptr, 200);

    print_memory_map();

    my_alloc(120);

    void* my_blk = my_alloc(10);
    my_free(my_blk);

    my_alloc(10);

    print_memory_map();

    free(ptr);
    return 0;
}
