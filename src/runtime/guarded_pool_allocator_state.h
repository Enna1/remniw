//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#pragma once

#include "utils.h"
#include <cstdint>

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

    size_t maximumAllocationSize() const { return PageSize; }

    size_t MaxSimultaneousAllocations = 0;

    uintptr_t GuardedPagePool = 0;
    uintptr_t GuardedPagePoolEnd = 0;

    size_t PageSize = 0;
};

}  // namespace aphotic_shield