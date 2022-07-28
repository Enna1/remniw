//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's aphotic_shield_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#include "guarded_pool_allocator.h"
#include "mutex.h"
#include "utils.h"
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>

namespace aphotic_shield {

namespace {

size_t roundUpTo(size_t Size, size_t Boundary) {
    return (Size + Boundary - 1) & ~(Boundary - 1);
}

uintptr_t getPageAddr(uintptr_t Ptr, uintptr_t PageSize) {
    return Ptr & ~(PageSize - 1);
}

bool isPowerOfTwo(uintptr_t X) {
    return (X & (X - 1)) == 0;
}

uintptr_t alignUp(uintptr_t Ptr, size_t Alignment) {
    assert(isPowerOfTwo(Alignment) && "Alignment must be a power of two!");
    assert(Alignment != 0 && "Alignment should be non-zero");
    if ((Ptr & (Alignment - 1)) == 0)
        return Ptr;

    Ptr += Alignment - (Ptr & (Alignment - 1));
    return Ptr;
}

uintptr_t alignDown(uintptr_t Ptr, size_t Alignment) {
    assert(isPowerOfTwo(Alignment) && "Alignment must be a power of two!");
    assert(Alignment != 0 && "Alignment should be non-zero");
    if ((Ptr & (Alignment - 1)) == 0)
        return Ptr;

    Ptr -= Ptr & (Alignment - 1);
    return Ptr;
}

size_t addrToSlot(const AllocatorState *State, uintptr_t Ptr) {
    size_t ByteOffsetFromPoolStart = Ptr - State->GuardedPagePool;
    return ByteOffsetFromPoolStart / (State->maximumAllocationSize() + State->PageSize);
}
}  // namespace

uintptr_t AllocatorState::slotToAddr(size_t N) const {
    return GuardedPagePool + (PageSize * (1 + N)) + (maximumAllocationSize() * N);
}

size_t AllocatorState::getNearestSlot(uintptr_t Ptr) const {
    if (Ptr <= GuardedPagePool + PageSize)
        return 0;
    if (Ptr > GuardedPagePoolEnd - PageSize)
        return MaxSimultaneousAllocations - 1;

    if (!isGuardPage(Ptr))
        return addrToSlot(this, Ptr);

    if (Ptr % PageSize <= PageSize / 2)
        return addrToSlot(this, Ptr - PageSize);  // Round down.
    return addrToSlot(this, Ptr + PageSize);      // Round up.
}

bool AllocatorState::isGuardPage(uintptr_t Ptr) const {
    assert(pointerIsMine(reinterpret_cast<void *>(Ptr)));
    size_t PageOffsetFromPoolStart = (Ptr - GuardedPagePool) / PageSize;
    size_t PagesPerSlot = maximumAllocationSize() / PageSize;
    return (PageOffsetFromPoolStart % (PagesPerSlot + 1)) == 0;
}

void AllocationMetadata::RecordAllocation(uintptr_t AllocAddr, size_t AllocSize) {
    Addr = AllocAddr;
    Size = AllocSize;
    IsDeallocated = false;
}

void AllocationMetadata::RecordDeallocation() {
    IsDeallocated = true;
}

void GuardedPoolAllocator::init(const options::Options &Opts) {
    if (!Opts.Enabled || Opts.MaxSimultaneousAllocations == 0)
        return;

    Check(Opts.MaxSimultaneousAllocations >= 0,
          "APHOTIC_SHIELD Error: MaxSimultaneousAllocations is < 0.");
    State.MaxSimultaneousAllocations = Opts.MaxSimultaneousAllocations;

    const size_t PageSize = getPageSize();
    assert((PageSize & (PageSize - 1)) == 0);
    State.PageSize = PageSize;

    size_t PoolBytesRequired =
        PageSize * (1 + State.MaxSimultaneousAllocations) /* GuardPage */ +
        State.maximumAllocationSize() * State.MaxSimultaneousAllocations /* Slot */;
    void *GuardedPoolMemory = reserveGuardedPool(PoolBytesRequired);
    State.GuardedPagePool = reinterpret_cast<uintptr_t>(GuardedPoolMemory);
    State.GuardedPagePoolEnd =
        reinterpret_cast<uintptr_t>(GuardedPoolMemory) + PoolBytesRequired;

    size_t MetadataBytesRequired =
        roundUpTo(State.MaxSimultaneousAllocations * sizeof(*Metadata), PageSize);
    Metadata = reinterpret_cast<AllocationMetadata *>(map(MetadataBytesRequired));

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
        Size > State.maximumAllocationSize())
        return nullptr;

    size_t Index;
    {
        ScopedLock L(PoolMutex);
        Index = reserveSlot();
    }

    if (Index == kInvalidSlotID)
        return nullptr;

    uintptr_t SlotStart = State.slotToAddr(Index);
    uintptr_t SlotEnd = State.slotToAddr(Index) + State.PageSize;
    AllocationMetadata *Meta = addrToMetadata(SlotStart);
    uintptr_t UserPtr;
    // Randomly choose whether to left-align or right-align the allocation, and
    // then apply the necessary adjustments to get an aligned pointer.
    if (getRandomUnsigned32() % 2 == 0)
        UserPtr = alignUp(SlotStart, Alignment);
    else
        UserPtr = alignDown(SlotEnd - Size, Alignment);
    assert(UserPtr >= SlotStart);
    assert(UserPtr + Size <= SlotEnd);

    const size_t PageSize = State.PageSize;
    allocateInGuardedPool(reinterpret_cast<void *>(getPageAddr(UserPtr, PageSize)),
                          roundUpTo(Size, PageSize));
    Meta->RecordAllocation(UserPtr, Size);

    return reinterpret_cast<void *>(UserPtr);
}

void GuardedPoolAllocator::deallocate(void *Ptr) {
    assert(pointerIsMine(Ptr) && "Pointer is not mine!");
    uintptr_t UPtr = reinterpret_cast<uintptr_t>(Ptr);
    size_t Slot = State.getNearestSlot(UPtr);
    uintptr_t SlotStart = State.slotToAddr(Slot);
    AllocationMetadata *Meta = addrToMetadata(UPtr);
    if (Meta->Addr != UPtr) {
        // If multiple errors occur at the same time, use the first one.
        ScopedLock L(PoolMutex);
        trapOnAddress(UPtr, Error::INVALID_FREE);
    }

    // Intentionally scope the mutex here, so that other threads can access the
    // pool during the expensive markInaccessible() call.
    {
        ScopedLock L(PoolMutex);
        if (Meta->IsDeallocated) {
            trapOnAddress(UPtr, Error::DOUBLE_FREE);
        }

        // Ensure that the deallocation is recorded before marking the page as
        // inaccessible. Otherwise, a racy use-after-free will have inconsistent
        // metadata.
        Meta->RecordDeallocation();
    }

    deallocateInGuardedPool(reinterpret_cast<void *>(SlotStart),
                            State.maximumAllocationSize());

    // And finally, lock again to release the slot back into the pool.
    ScopedLock L(PoolMutex);
    freeSlot(Slot);
}

AllocationMetadata *GuardedPoolAllocator::addrToMetadata(uintptr_t Ptr) const {
    return &Metadata[State.getNearestSlot(Ptr)];
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

size_t GuardedPoolAllocator::reserveSlot() {
    // We won't reuse a slot until we have made at least a single allocation in each slot.
    if (NumAllocations < State.MaxSimultaneousAllocations)
        return NumAllocations++;

    if (FreeSlotsLength == 0)
        return kInvalidSlotID;

    size_t ReservedIndex = getRandomUnsigned32() % FreeSlotsLength;
    size_t SlotIndex = FreeSlots[ReservedIndex];
    FreeSlots[ReservedIndex] = FreeSlots[--FreeSlotsLength];
    return SlotIndex;
}

void GuardedPoolAllocator::freeSlot(size_t SlotIndex) {
    assert(FreeSlotsLength < State.MaxSimultaneousAllocations);
    FreeSlots[FreeSlotsLength++] = SlotIndex;
}

void GuardedPoolAllocator::allocateInGuardedPool(void *Ptr, size_t Size) const {
    assert((reinterpret_cast<uintptr_t>(Ptr) % State.PageSize) == 0);
    assert((Size % State.PageSize) == 0);
    Check(mprotect(Ptr, Size, PROT_READ | PROT_WRITE) == 0,
          "Failed to allocate in guarded pool allocator memory");
}

void GuardedPoolAllocator::deallocateInGuardedPool(void *Ptr, size_t Size) const {
    assert((reinterpret_cast<uintptr_t>(Ptr) % State.PageSize) == 0);
    assert((Size % State.PageSize) == 0);
    // mmap() a PROT_NONE page over the address to release it to the system, if
    // we used mprotect() here the system would count pages in the quarantine
    // against the RSS.
    Check(mmap(Ptr, Size, PROT_NONE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0) !=
              MAP_FAILED,
          "Failed to deallocate in guarded pool allocator memory");
}

uint32_t GuardedPoolAllocator::getRandomUnsigned32() {
    RandomState ^= RandomState << 13;
    RandomState ^= RandomState >> 17;
    RandomState ^= RandomState << 5;
    return RandomState;
}

void GuardedPoolAllocator::trapOnAddress(uintptr_t Address, Error E) {
    State.FailureType = E;
    State.FailureAddress = Address;

    // Raise a SEGV by touching first guard page.
    volatile char *p = reinterpret_cast<char *>(State.GuardedPagePool);
    *p = 0;
    __builtin_trap();
}

}  // namespace aphotic_shield
