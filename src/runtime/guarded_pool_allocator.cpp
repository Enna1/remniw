//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#include "guarded_pool_allocator.h"
#include "utils.h"
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>

namespace aphotic_shield {

namespace {
GuardedPoolAllocator *SingletonPtr = nullptr;

size_t roundUpTo(size_t Size, size_t Boundary) {
    return (Size + Boundary - 1) & ~(Boundary - 1);
}

uintptr_t getPageAddr(uintptr_t Ptr, uintptr_t PageSize) {
    return Ptr & ~(PageSize - 1);
}

bool isPowerOfTwo(uintptr_t X) {
    return (X & (X - 1)) == 0;
}
}  // namespace

GuardedPoolAllocator *GuardedPoolAllocator::getSingleton() {
    return SingletonPtr;
}

void GuardedPoolAllocator::init(const options::Options &Opts) {
    if (!Opts.Enabled || Opts.MaxSimultaneousAllocations == 0)
        return;

    Check(Opts.MaxSimultaneousAllocations >= 0,
          "APHOTIC_SHIELD Error: MaxSimultaneousAllocations is < 0.");
    State.MaxSimultaneousAllocations = Opts.MaxSimultaneousAllocations;

    SingletonPtr = this;

    const size_t PageSize = getPageSize();
    assert((PageSize & (PageSize - 1)) == 0);
    State.PageSize = PageSize;

    size_t PoolBytesRequired =
        PageSize * (1 + State.MaxSimultaneousAllocations) /* GuardPage */ +
        PageSize * State.MaxSimultaneousAllocations /* Slot */;
    void *GuardedPoolMemory = reserveGuardedPool(PoolBytesRequired);
    State.GuardedPagePool = reinterpret_cast<uintptr_t>(GuardedPoolMemory);
    State.GuardedPagePoolEnd =
        reinterpret_cast<uintptr_t>(GuardedPoolMemory) + PoolBytesRequired;

    // size_t MetadataBytesRequired =
    //     roundUpTo(State.MaxSimultaneousAllocations * sizeof(*Metadata), PageSize);
    // Metadata =
    //     reinterpret_cast<AllocationMetadata *>(map(MetadataBytesRequired));

    size_t FreeSlotsBytesRequired =
        roundUpTo(State.MaxSimultaneousAllocations * sizeof(*FreeSlots), PageSize);
    FreeSlots = reinterpret_cast<size_t *>(map(FreeSlotsBytesRequired));
}

void *GuardedPoolAllocator::allocate(size_t Size, size_t Alignment) {
    // If GuardedPagePoolEnd == 0, it means APHOTIC_SHIELD is disabled.
    if (State.GuardedPagePoolEnd == 0) {
        return nullptr;
    }

    if (Size == 0)
        Size = 1;
    if (Alignment == 0)
        Alignment = alignof(max_align_t);

    if (!isPowerOfTwo(Alignment) || Alignment > State.PageSize ||
        Size > State.maximumAllocationSize)
        return nullptr;

    // Protect against recursivity.
    if (getThreadLocals()->RecursiveGuard)
        return nullptr;
    ScopedRecursiveGuard SRG;

    size_t Index;
    {
        ScopedLock L(PoolMutex);
        Index = reserveSlot();
    }

    if (Index == kInvalidSlotID)
        return nullptr;

    uintptr_t SlotStart = State.slotToAddr(Index);
    AllocationMetadata *Meta = addrToMetadata(SlotStart);
    uintptr_t SlotEnd = State.slotToAddr(Index) + State.maximumAllocationSize();
    uintptr_t UserPtr;
    // Randomly choose whether to left-align or right-align the allocation, and
    // then apply the necessary adjustments to get an aligned pointer.
    if (getRandomUnsigned32() % 2 == 0)
        UserPtr = alignUp(SlotStart, Alignment);
    else
        UserPtr = alignDown(SlotEnd - Size, Alignment);

    assert(UserPtr >= SlotStart);
    assert(UserPtr + Size <= SlotEnd);

    // If a slot is multiple pages in size, and the allocation takes up a single
    // page, we can improve overflow detection by leaving the unused pages as
    // unmapped.
    const size_t PageSize = State.PageSize;
    allocateInGuardedPool(reinterpret_cast<void *>(getPageAddr(UserPtr, PageSize)),
                          roundUpTo(Size, PageSize));

    Meta->RecordAllocation(UserPtr, Size);
    {
        ScopedLock UL(BacktraceMutex);
        Meta->AllocationTrace.RecordBacktrace(Backtrace);
    }

    return reinterpret_cast<void *>(UserPtr);
}

size_t GuardedPoolAllocator::getPageSize() {
    return sysconf(_SC_PAGESIZE);
}

void *GuardedPoolAllocator::reserveGuardedPool(size_t Size) {
    assert((Size % State.PageSize) == 0);
    void *Ptr = mmap(nullptr, Size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    Check(Ptr != MAP_FAILED,
          "APHOTIC_SHIELD Error: Failed to reserve guarded pool allocator memory");
    return Ptr;
}

void *GuardedPoolAllocator::map(size_t Size) const {
    assert((Size % State.PageSize) == 0);
    void *Ptr =
        mmap(nullptr, Size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    Check(Ptr != MAP_FAILED,
          "APHOTIC_SHIELD Error: Failed to map guarded pool allocator memory");
    return Ptr;
}

}  // namespace aphotic_shield