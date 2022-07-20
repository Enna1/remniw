//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

namespace aphotic_shield {

enum Error : uint8_t {
    UNKNOWN,
    USE_AFTER_FREE,
    DOUBLE_FREE,
    INVALID_FREE,
    BUFFER_OVERFLOW,
    BUFFER_UNDERFLOW
};

class GuardedPoolAllocator;

// Install the SIGSEGV signal handler for printing error message
// e.g.(use-after-free, heap-buffer-overflow, heap-buffer-underflow, etc)
void installSegvSignalHandler(GuardedPoolAllocator *GPA);

}  // namespace aphotic_shield
