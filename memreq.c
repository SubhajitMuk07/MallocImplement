#include "memreq.h"

#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <stdint.h>

char* get_memory(unsigned n){
    char *page = sbrk( (intptr_t) n);

    return (page != (char*) -1 ? page : NULL);
}