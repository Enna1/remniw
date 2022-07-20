//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#pragma once

#include "error_report.h"
#include "mutex.h"
#include "options.h"
#include "utils.h"

namespace aphotic_shield {

struct AllocatorState {
    constexpr AllocatorState() {}

    // Returns whether the provided Ptr is an allocation owned by guarded pool.
    APHOTIC_SHIELD_ALWAYS_INLINE bool pointerIsMine(const void *Ptr) const {
        uintptr_t P = reinterpret_cast<uintptr_t>(Ptr);
        return P < GuardedPagePoolEnd && GuardedPagePool <= P;
    }

    // Returns the address of the N-th guarded slot.
    uintptr_t slotToAddr(size_t N) const;

    // Gets the nearest slot to the provided Ptr.
    size_t getNearestSlot(uintptr_t Ptr) const;

    // Returns whether the provided pointer is a guard page or not. The provided Ptr
    // must be within memory owned by this pool.
    bool isGuardPage(uintptr_t Ptr) const;

    // Returns the largest allocation that is supported by this pool(i.e. PageSize).
    size_t maximumAllocationSize() const { return PageSize; }

    // The number of guarded slots that this pool holds.
    size_t MaxSimultaneousAllocations = 0;

    // Pointer to the pool of guarded slots.
    uintptr_t GuardedPagePool = 0;
    uintptr_t GuardedPagePoolEnd = 0;

    // Page size in bytes.
    size_t PageSize = 0;

    // Set when double-free or invalid-free errors are detected.
    // Used when diagnosing what error happened during error report.
    Error FailureType = Error::UNKNOWN;
    uintptr_t FailureAddress = 0;
};

struct AllocationMetadata {
    // Records the given allocation metadata into this struct.
    void RecordAllocation(uintptr_t Addr, size_t Size);
    // Record that this allocation is now deallocated.
    void RecordDeallocation();

    // The address of this allocation. If zero, the rest of this struct isn't
    // valid, as the allocation has never occurred.
    uintptr_t Addr = 0;
    // Represents the actual size of the allocation.
    size_t Size = 0;
    // Whether this allocation has been deallocated yet.
    bool IsDeallocated = false;
};

class GuardedPoolAllocator {
public:
    constexpr GuardedPoolAllocator() {}
    GuardedPoolAllocator(const GuardedPoolAllocator &) = delete;
    GuardedPoolAllocator &operator=(const GuardedPoolAllocator &) = delete;
    ~GuardedPoolAllocator() = default;

    // Initialise the the members of this class. Create the allocation
    // pool using the provided options. See options.inc for runtime configuration
    // options.
    void init(const options::Options &Opts);

    // Allocate memory in a guarded slot, with the specified `Alignment`. Returns
    // nullptr if the pool is empty, if the alignnment is not a power of two, or
    // if the size/alignment makes the allocation too large for this pool to
    // handle.
    void *allocate(size_t Size, size_t Alignment);

    // Deallocate memory in a guarded slot. The provided pointer must have been
    // allocated using this pool. This will set the guarded slot as inaccessible.
    void deallocate(void *Ptr);

    // Returns a pointer to the Metadata region, or nullptr if it doesn't exist.
    const AllocationMetadata *getMetadataRegion() const { return Metadata; }

    // Returns a pointer to the AllocatorState region.
    const AllocatorState *getAllocatorState() const { return &State; }

    // Returns whether the provided pointer is an allocation owned by guarded pool.
    APHOTIC_SHIELD_ALWAYS_INLINE bool pointerIsMine(const void *Ptr) const {
        return State.pointerIsMine(Ptr);
    }

private:
    static constexpr size_t kInvalidSlotID = ~0;

    AllocatorState State;
    // Record the number allocations that we've made.
    size_t NumAllocations = 0;
    //  Pointer to the allocation metadata.
    AllocationMetadata *Metadata = nullptr;
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
    // Reserve guarded pool allocator memory using mmap.
    void *reserveGuardedPool(size_t Size);
    // allocateInGuardedPool() Ptr and Size must be a subrange of the previously
    // reserved pool range.
    void allocateInGuardedPool(void *Ptr, size_t Size) const;
    // deallocateInGuardedPool() Ptr and Size must be an exact pair previously
    // passed to allocateInGuardedPool().
    void deallocateInGuardedPool(void *Ptr, size_t Size) const;

    AllocationMetadata *addrToMetadata(uintptr_t Ptr) const;

    void *map(size_t Size) const;

    // Reserve a slot in GuardedPagePool for a new allocation.
    // Returns kInvalidSlotID if no slot is available to be reserved.
    size_t reserveSlot();

    // Unreserve the guarded slot.
    void freeSlot(size_t SlotIndex);

    // Use xorshift32, a class of pseudorandom number generators, see
    // https://en.wikipedia.org/wiki/Xorshift.
    uint32_t getRandomUnsigned32();

    // Raise a SEGV and set the `FailureType` and `FailureAddress` fields in the
    // Allocator's State in order to diagnose what error happened. Used when
    // double-free or invalid-free errors are detected.
    void trapOnAddress(uintptr_t Address, Error E);
};

}  // namespace aphotic_shield
