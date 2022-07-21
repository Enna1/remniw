//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#pragma once

#include "guarded_pool_allocator_state.h"
#include "mutex.h"
#include "options.h"
#include "utils.h"

namespace aphotic_shield {

class GuardedPoolAllocator {
public:
    constexpr GuardedPoolAllocator() {}
    GuardedPoolAllocator(const GuardedPoolAllocator &) = delete;
    GuardedPoolAllocator &operator=(const GuardedPoolAllocator &) = delete;
    ~GuardedPoolAllocator() = default;

    static GuardedPoolAllocator *getSingleton();

    void init(const options::Options &Opts);
    void *allocate(size_t Size, size_t Alignment);
    void deallocate(void *Ptr);

    APHOTIC_SHIELD_ALWAYS_INLINE bool pointerIsMine(const void *Ptr) const {
        return State.pointerIsMine(Ptr);
    }

private:
    static constexpr size_t kInvalidSlotID = ~0;

    AllocatorState State;
    // Record the number allocations that we've made.
    size_t NumAllocations = 0;
    // Pointer to the free slots list which stores the indexes of freed slot in
    // GuardedPagePool.
    size_t *FreeSlots = nullptr;
    // The current length of the free slots list.
    size_t FreeSlotsLength = 0;
    // RandomState initialized with non-zero seed
    uint32_t RandomState = 0xacd979ce;
    // A mutex to protect the guarded slot and metadata pool.
    Mutex PoolMutex;

    // Get the page size using sysconf(_SC_PAGESIZE).
    // We should only call this function once, and cahe the result in
    // AllocatorState::PageSize.
    static size_t getPageSize();

    void *reserveGuardedPool(size_t Size);
    // allocateInGuardedPool() Ptr and Size must be a subrange of the previously
    // reserved pool range.
    void allocateInGuardedPool(void *Ptr, size_t Size) const;
    // deallocateInGuardedPool() Ptr and Size must be an exact pair previously
    // passed to allocateInGuardedPool().
    void deallocateInGuardedPool(void *Ptr, size_t Size) const;

    void *map(size_t Size) const;

    // Reserve a slot in GuardedPagePool for a new allocation.
    // Returns kInvalidSlotID if no slot is available to be reserved.
    size_t reserveSlot();

    // Use xorshift32, a class of pseudorandom number generators, see
    // https://en.wikipedia.org/wiki/Xorshift.
    uint32_t getRandomUnsigned32();
};

}  // namespace aphotic_shield