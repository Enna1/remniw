#include <stddef.h>

extern "C" {
void as_init();
void *as_alloc(size_t size);
void as_dealloc(void *ptr);
}