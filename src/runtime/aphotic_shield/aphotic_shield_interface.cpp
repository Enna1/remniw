#include "aphotic_shield_interface.h"
#include "aphotic_shield_allocator.h"
#include <stddef.h>

extern "C" {
void as_init() {
    aphotic_shield::initAphoticShield();
}

void *as_alloc(size_t size) {
    return aphotic_shield::AphoticShieldAllocate(size, 0);
}

void as_dealloc(void *ptr) {
    aphotic_shield::AphoticShieldDeallocate(ptr, 0, 0);
}
}
