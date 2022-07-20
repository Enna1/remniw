#include "aphotic_shield_interface.h"

int main() {
    as_init();

    char *ptr = (char *)as_alloc(4000);
    as_dealloc(ptr);
    char ch = *ptr;

    return 0;
}
