// RUN: %cxx_aphotic_shield %s -o %t
// RUN: %expect_crash %t 2>&1 | FileCheck %s

// CHECK: APHOTIC-SHIELD detected a memory error
// CHECK: Buffer Overflow on address 0x{{[a-f0-9]+}}

#include "aphotic_shield/aphotic_shield_interface.h"
#include <unistd.h>

unsigned pageSize() {
    return sysconf(_SC_PAGESIZE);
}

int main() {
    as_init();

    char *ptr = (char *)as_alloc(pageSize());
    char ch = *(ptr + pageSize());

    return 0;
}
