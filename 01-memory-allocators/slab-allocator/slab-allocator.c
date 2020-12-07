#include "math.h"
#include "malloc.h"
#include "stdint.h"
#include "stdio.h"

void *last_alloc_start;
void *last_alloc_end;

void *alloc_slab(size_t order) {
    size_t size = 4096 * (size_t) pow(2, order);
    void * allocated_ptr = memalign(size, size);
    last_alloc_start = allocated_ptr;
    last_alloc_end = allocated_ptr + size;
    printf("Memory allocated: %p - %p\n", last_alloc_start, last_alloc_end);
    return allocated_ptr;
}

void free_slab(void *slab) {
    printf("Memory freed: %p\n", slab);
    free(slab);
}

struct object {
    struct object* next;
};

struct slab {
    size_t free_count;
    struct object* free_obj_head;
    struct slab* prev;
    struct slab* next;
};

struct cache {
    struct slab* empty_head;
    struct slab* used_head;
    struct slab* full_head;

    size_t object_size;
    size_t slab_order;
    size_t slab_objects;
};

void cache_setup(struct cache *cache, size_t object_size)
{
    cache->slab_order = 10;
    cache->object_size = object_size;
    size_t bytes_available = (size_t) (4096 * pow(2, cache->slab_order) - sizeof(struct slab));
    cache->slab_objects = (bytes_available / object_size) - 1;
}

struct slab* create_slab(size_t slab_order, size_t slab_objects, size_t object_size) {
    void *allocated_ptr = alloc_slab(slab_order);

    struct slab* slab = (struct slab*) allocated_ptr;
    slab->free_count = slab_objects;

    struct object* prev_obj = (struct object*) (allocated_ptr + sizeof(struct slab) + 1);
    slab->free_obj_head = prev_obj;

    for (size_t i = 0; i < slab_objects - 1; i++) {
        struct object* next_obj = (struct object*) ((void*) prev_obj + object_size);
        prev_obj->next = next_obj;
        prev_obj = next_obj;
    }
    return slab;
}

void remove_slab_from_cache(struct cache* cache, struct slab* slab) {
    if (slab == cache->empty_head) {
        cache->empty_head = slab->next;
    } else if (slab == cache->used_head) {
        cache->used_head = slab->next;
    } else if (slab == cache->full_head) {
        cache->full_head = slab->next;
    }

    if (slab->prev) {
        slab->prev->next = slab->next;
    }
    if (slab->next) {
        slab->next->prev = slab->prev;
    }
}

void add_slab_to_cache(struct cache* cache, struct slab* slab) {
    if (slab->free_count == 0) {
        slab->next = cache->full_head;
        if (cache->full_head) {
            cache->full_head->prev = slab;
        }
        cache->full_head = slab;
    } else if (slab->free_count != cache->slab_objects) {
        slab->next = cache->used_head;
        if (cache->used_head) {
            cache->used_head->prev = slab;
        }
        cache->used_head = slab;
    } else {
        slab->next = cache->empty_head;
        if (cache->empty_head) {
            cache->empty_head->prev = slab;
        }
        cache->empty_head = slab;
    }
}

void *cache_alloc(struct cache *cache) {
    struct slab *source = NULL;
    if (cache->used_head != NULL) {
        source = cache->used_head;
        remove_slab_from_cache(cache, source);
    } else if (cache->empty_head != NULL) {
        source = cache->empty_head;
        remove_slab_from_cache(cache, source);
    } else {
        source = create_slab(cache->slab_order,
                             cache->slab_objects,
                             cache->object_size);
    }

    source->free_count -= 1;
    void *allocated_ptr = source->free_obj_head;
    source->free_obj_head = source->free_obj_head->next;

    add_slab_to_cache(cache, source);
    return allocated_ptr;
}

void cache_free(struct cache *cache, void *ptr)
{
    uintptr_t xor_mask = 4096 * pow(2, cache->slab_order) - 1;
    xor_mask = ~xor_mask;
    struct slab* source = (struct slab *) ((uintptr_t) ptr & xor_mask);

    remove_slab_from_cache(cache, source);

    source->free_count += 1;
    struct object* free_obj = (struct object*) ptr;
    free_obj->next = source->free_obj_head;
    source->free_obj_head = free_obj;

    add_slab_to_cache(cache, source);
}

void free_slab_list(struct slab* head) {
    struct slab* to_free = head;
    while (head) {
        to_free = head;
        head = head->next;
        free_slab(to_free);
    }
}

void cache_shrink(struct cache *cache)
{
    free_slab_list(cache->empty_head);
}

void cache_release(struct cache *cache)
{
    free_slab_list(cache->empty_head);
    free_slab_list(cache->used_head);
    free_slab_list(cache->full_head);
}

int main() {
    struct cache* cache = malloc(sizeof(struct cache));
    cache_setup(cache, 32);
    for (int i = 0; i < 10000000; ++i) {
        void* p = cache_alloc(cache);
        if (p > last_alloc_end || p < last_alloc_start) {
            fprintf(stderr, "Allocation error: %p\n", p);
            return 1;
        }
    }
    cache_release(cache);
    free(cache);
}
