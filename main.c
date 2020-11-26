#include "math.h"
#include "malloc.h"
#include "stdint.h"

/**
 * Эти две функции вы должны использовать для аллокации
 * и освобождения памяти в этом задании. Считайте, что
 * внутри они используют buddy аллокатор с размером
 * страницы равным 4096 байтам.
 **/

/**
 * Аллоцирует участок размером 4096 * 2^order байт,
 * выровненный на границу 4096 * 2^order байт. order
 * должен быть в интервале [0; 10] (обе границы
 * включительно), т. е. вы не можете аллоцировать больше
 * 4Mb за раз.
 **/
void *alloc_slab(int order) {
    int size = 4096 * (int) pow(2, order);
    void * allocated_ptr = memalign(size, size);
    printf("SLAB memory allocated: %p - %p\n", allocated_ptr, allocated_ptr + size);
    return allocated_ptr;
}

/**
 * Освобождает участок ранее аллоцированный с помощью
 * функции alloc_slab.
 **/
void free_slab(void *slab) {
    printf("Free memory: %p", slab);
    free(slab);
}


struct object {
    struct object* next;
};

struct slab {
    int free_count;
    struct object* free_obj_head;
    struct slab* prev;
    struct slab* next;
};


/**
 * Эта структура представляет аллокатор, вы можете менять
 * ее как вам удобно. Приведенные в ней поля и комментарии
 * просто дают общую идею того, что вам может понадобится
 * сохранить в этой структуре.
 **/
struct cache {
    struct slab* empty_head; /* список пустых SLAB-ов для поддержки cache_shrink */
    struct slab* used_head;  /* список частично занятых SLAB-ов */
    struct slab* full_head;  /* список заполненых SLAB-ов */

    int object_size;         /* размер аллоцируемого объекта */
    int slab_order;          /* используемый размер SLAB-а */
    int slab_objects;        /* количество объектов в одном SLAB-е */
};


/**
 * Функция инициализации будет вызвана перед тем, как
 * использовать это кеширующий аллокатор для аллокации.
 * Параметры:
 *  - cache - структура, которую вы должны инициализировать
 *  - object_size - размер объектов, которые должен
 *    аллоцировать этот кеширующий аллокатор
 **/
void cache_setup(struct cache *cache, int object_size)
{
    cache->slab_order = 10;
    cache->object_size = object_size;
    int bytes_available = (int) (4096 * pow(2, cache->slab_order) - sizeof(struct slab));
    cache->slab_objects = bytes_available / object_size;
}

struct slab* create_slab(int slab_order, int slab_objects, int object_size) {
    void *allocated_ptr = alloc_slab(slab_order);
    struct slab* slab = (struct slab*) allocated_ptr;
    slab->free_count = slab_objects;

    struct object* prev_obj = allocated_ptr + sizeof(struct slab) + 1;
    slab->free_obj_head = prev_obj;

    for (int i = 0; i < slab_objects - 1; i++) {
        struct object* next_obj = (void*) prev_obj + object_size + 1;
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

/**
 * Функция аллокации памяти из кеширующего аллокатора.
 * Должна возвращать указатель на участок памяти размера
 * как минимум object_size байт (см cache_setup).
 * Гарантируется, что cache указывает на корректный
 * инициализированный аллокатор.
 **/
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

/**
 * Функция освобождения памяти назад в кеширующий аллокатор.
 * Гарантируется, что ptr - указатель ранее возвращенный из
 * cache_alloc.
 **/
void cache_free(struct cache *cache, void *ptr)
{
    uintptr_t xor_mask = 4096 * pow(2, cache->slab_order) - 1;
    xor_mask = ~xor_mask;
    struct slab* source = (struct slab *) ((uintptr_t) ptr & xor_mask);

    remove_slab_from_cache(cache, source);

    source->free_count += 1;
    struct object* free_obj = ptr;
    free_obj->next = source->free_obj_head;
    source->free_obj_head = free_obj;

    add_slab_to_cache(cache, source);
}


/**
 * Функция должна освободить все SLAB, которые не содержат
 * занятых объектов. Если SLAB не использовался для аллокации
 * объектов (например, если вы выделяли с помощью alloc_slab
 * память для внутренних нужд вашего алгоритма), то освбождать
 * его не обязательно.
 **/
void cache_shrink(struct cache *cache)
{
    while (cache->empty_head) {
        struct slab* to_free = cache->empty_head;
        cache->empty_head = cache->empty_head->next;
        free_slab(to_free);
    }
}

/**
 * Функция освобождения будет вызвана когда работа с
 * аллокатором будет закончена. Она должна освободить
 * всю память занятую аллокатором. Проверяющая система
 * будет считать ошибкой, если не вся память будет
 * освбождена.
 **/
void cache_release(struct cache *cache)
{
    cache_shrink(cache);
    struct slab* to_free = NULL;
    while (cache->used_head) {
        to_free = cache->used_head;
        cache->used_head = cache->used_head->next;
        free_slab(to_free);
    }
    while (cache->full_head) {
        to_free = cache->full_head;
        cache->full_head = cache->full_head->next;
        free_slab(to_free);
    }
}


int main() {
    printf("Size of slab: %lu\n", sizeof(struct slab));
    printf("Size of cache: %lu\n", sizeof(struct cache));

    struct cache* cache = malloc(sizeof(struct cache));
    cache_setup(cache, 32768);

    void * allocated_ptr = cache_alloc(cache);
    printf("Allocated: %p\n", allocated_ptr);
    cache_free(cache, allocated_ptr);

    cache_release(cache);
    cache_shrink(cache);

    free(cache);
}
