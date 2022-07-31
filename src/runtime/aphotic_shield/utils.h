//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdio>

#define APHOTIC_SHIELD_ALWAYS_INLINE inline __attribute__((always_inline))

namespace {
void die(const char *Message) {
    fprintf(stderr, "%s", Message);
    __builtin_trap();
}
}  // namespace

namespace aphotic_shield {

APHOTIC_SHIELD_ALWAYS_INLINE void Check(bool Condition, const char *Message) {
    if (Condition)
        return;
    die(Message);
}

}  // namespace aphotic_shield
