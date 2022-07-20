//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#pragma once

#include "options.h"
#include "utils.h"

namespace aphotic_shield {

struct AllocatorState {
    constexpr AllocatorState() {}

    APHOTIC_SHIELD_ALWAYS_INLINE bool pointerIsMine(const void *Ptr) const {
        uintptr_t P = reinterpret_cast<uintptr_t>(Ptr);
        return P < GuardedPagePoolEnd && GuardedPagePool <= P;
    }

    uintptr_t slotToAddr(size_t N) const;

    size_t getNearestSlot(uintptr_t Ptr) const;

    bool isGuardPage(uintptr_t Ptr) const;

    size_t MaxSimultaneousAllocations = 0;

    uintptr_t GuardedPagePool = 0;
    uintptr_t GuardedPagePoolEnd = 0;

    size_t PageSize = 0;
};

class GuardedPoolAllocator {
public:
    constexpr GuardedPoolAllocator() {}
    GuardedPoolAllocator(const GuardedPoolAllocator &) = delete;
    GuardedPoolAllocator &operator=(const GuardedPoolAllocator &) = delete;
    ~GuardedPoolAllocator() = default;

    static GuardedPoolAllocator *getSingleton();

    void init(const options::Options &Opts);
    void *allocate(size_t Size, size_t Alignment);

private:
    AllocatorState State;

    // Pointer to the free slots list which stores the indexes of freed slot in
    // GuardedPagePool.
    size_t *FreeSlots = nullptr;
    // The current length of the free slots list.
    size_t FreeSlotsLength = 0;

    // Get the page size using sysconf(_SC_PAGESIZE).
    // We should only call this function once, and cahe the result in
    // AllocatorState::PageSize.
    static size_t getPageSize();

    void *reserveGuardedPool(size_t Size);

    void *map(size_t Size) const;
};

}  // namespace aphotic_shield