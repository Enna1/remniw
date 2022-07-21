//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
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

}  // namespace aphotic_shield