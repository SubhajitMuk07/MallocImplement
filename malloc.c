
/* High-level overview: This works by maintaining a list of previously-used
* memory segments that are now free, which seems to be the standard way of
* implementing these functions */

#include <errno.h>
#include <limits.h>

#include "malloc.h"
#include "memreq.h"

/* this header is stored at the beginning of memory segments in the list */
struct header {
  unsigned len;
  struct header *next;
  struct header *prev;
};

struct header* first = NULL; /* initially nothing has been freed */

void *malloc(size_t size) {

  struct header *ptr;
  
  /* we first check to see if there is something in the free list that can
* be reused by malloc */
  if (first != NULL) {
    ptr = first;
    /* find a memory segment in the list that is at least as large as what the
* user requested, because we must allocate contiguous memory segments */
    while (ptr != NULL && ptr->len - sizeof(struct header) < size) {
      ptr = ptr->next;
    }
    /* if we	 found such a segment, allocate it */
    if (ptr != NULL) {
      /* get where the new segment starts */
      int frag_len = ptr->len - (size + sizeof(struct header));
      
      /* if there is leftover memory, we must create a list node for it */
      if (frag_len > 0) {
        struct header* next_frag;
        next_frag = ptr + frag_len;
        if (ptr->next != NULL)
          ptr->next->prev = next_frag;
        if (ptr->prev != NULL)
          ptr->prev->next = next_frag;
        /* create the new header for the new node in the list */
        
        /* For some reason, test2 segfaults here after having called free,
* so I'm guessing a pointer isn't being set correctly somewhere,
* but I can't figure out where. */
        
        next_frag->len = frag_len + sizeof(struct header);
        next_frag->next = ptr->next;
        ptr->next = next_frag;
        next_frag->prev = ptr;
      }

      /* return a pointer to the memory that the user can use */
      return (void *)(ptr + sizeof(struct header));
    }
  }
  
  /* there was not a suitable memory segment in the list,
* so request new memory from the kernel */
  ptr = (struct header *) get_memory(size + sizeof(struct header));
  if (ptr == NULL) {
    errno = ENOMEM;
    return NULL; /* no memory left */
  }
  
  /* the new memory will be added to the list when free() is called */
  return (void *)(ptr + sizeof(struct header));
}

void free(void* ptr) {
  
  /* I run gmake and gcc,
* and I ain't never called malloc without calling free */
  
  struct header *hdr;
  /* find where the header for the freed memory segment begins */
  hdr = ptr - sizeof(struct header);

  /* add the newly freed memory to the linked list */
  if (first == NULL) {
    first = hdr;
    hdr->len = sizeof(struct header) + sizeof(*ptr);
    hdr -> next = NULL;
    hdr -> prev = NULL;
  }
  else {
    hdr->len = sizeof(struct header) + sizeof(*ptr);
    /* link the new memory onto the 'end' of the circular list */
    hdr->next = first;
    hdr->prev = first->prev;
    first->prev->next = hdr;
  }
}

static size_t highest(size_t in) {
  size_t num_bits = 0;

  while (in != 0) {
    ++num_bits;
    in >>= 1;
  }

  return num_bits;
}

void* calloc(size_t number, size_t size) {
  size_t number_size = 0;

  /* This prevents an integer overflow. A size_t is a typedef to an integer
* large enough to index all of memory. If we cannot fit in a size_t, then
* we need to fail.
*/
  if (highest(number) + highest(size) > sizeof(size_t) * CHAR_BIT)
  {
    errno = ENOMEM;
    return NULL;
  }

  number_size = number * size;
  void* ret = malloc(number_size);

  if (ret)
  {
    memset(ret, 0, number_size);
  }

  return ret;
}

void* realloc(void *ptr, size_t size) {
  struct header *hdr;
  hdr = (struct header*) ptr;
  size_t old_size = hdr->len - sizeof(struct header);
  void* ret = malloc(size);

  if (ret) {
    if (ptr) {
      memmove(ret, ptr, old_size < size ? old_size : size);
      free(ptr);
    }

    return ret;
  } else {
    errno = ENOMEM;
    return NULL;
  }
}