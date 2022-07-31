#include "aphotic_shield_allocator.h"
#include "guarded_pool_allocator.h"
#include "options.h"
#include <stdlib.h>

namespace aphotic_shield {

static aphotic_shield::GuardedPoolAllocator GuardedAlloc;

// Get environment variable `APHOTIC_SHIELD_OPTIONS` to initialize GuardedPoolAllocator.
// Install SIGSEGV signal handler to print useful error report.
void initAphoticShield() {
    aphotic_shield::options::initOptions(getenv("APHOTIC_SHIELD_OPTIONS"));
    aphotic_shield::options::Options &Opts = aphotic_shield::options::getOptions();
    GuardedAlloc.init(Opts);
    aphotic_shield::installSegvSignalHandler(&GuardedAlloc);
}

// If cannot allocate in GuardedPoolAllocator(e.g. Size > 4096, no free slot,
// GuardedPoolAllocator is not enabled), fallback to malloc.
void *AphoticShieldAllocate(size_t Size, size_t Alignment) {
    if (void *Ptr = GuardedAlloc.allocate(Size, Alignment)) {
        return Ptr;
    }
    return malloc(Size);
}

// First check if Ptr belongs to GuardedPoolAllocator,
// if no, it must be allocated via malloc, lets call free to deallocate it.
void AphoticShieldDeallocate(void *Ptr, size_t Size, size_t Alignment) {
    if (GuardedAlloc.pointerIsMine(Ptr)) {
        GuardedAlloc.deallocate(Ptr);
        return;
    }
    return free(Ptr);
}

}  // namespace aphotic_shield
