#ifndef MALLOC_H_
#define MALLOC_H_
#include <stddef.h>

void* malloc(size_t size);
void* calloc(size_t number, size_t size);
void* realloc(void *ptr, size_t size);
void free(void* ptr);

#endif /*MALLOC_H_*/
