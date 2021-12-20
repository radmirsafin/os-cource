#ifndef SIMPLE_ALLOCATOR_H
#define SIMPLE_ALLOCATOR_H

int get_free_size();
void mysetup(void *buf, int size);
void *myalloc(int size);
void myfree(void *p);

#endif //SIMPLE_ALLOCATOR_H
