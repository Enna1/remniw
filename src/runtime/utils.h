#pragma once

#include <cstdio>

#define APHOTIC_SHIELD_ALWAYS_INLINE inline __attribute__((always_inline))

namespace aphotic_shield {

void die(const char *Message) {
    fprintf(stderr, "%s", Message);
    __builtin_trap();
}

APHOTIC_SHIELD_ALWAYS_INLINE void Check(bool Condition, const char *Message) {
    if (Condition)
        return;
    die(Message);
}

}  // namespace aphotic_shield