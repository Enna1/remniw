// RUN: %cxx_aphotic_shield %s -o %t
// RUN: %expect_crash %t 2>&1 | FileCheck %s

// CHECK: APHOTIC-SHIELD detected a memory error
// CHECK: Double Free on address 0x{{[a-f0-9]+}}

#include "aphotic_shield/aphotic_shield_interface.h"

int main() {
    as_init();

    char *ptr = (char *)as_alloc(4000);
    as_dealloc(ptr);
    as_dealloc(ptr);

    return 0;
}
