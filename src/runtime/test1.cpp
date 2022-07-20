#include "error_report.h"
#include "guarded_pool_allocator.h"
#include "options.h"

static aphotic_shield::GuardedPoolAllocator GuardedAlloc;

int main() {
    aphotic_shield::options::Options Opts;
    GuardedAlloc.init(Opts);
    aphotic_shield::installSegvSignalHandler(&GuardedAlloc);

    char *ptr = (char *)GuardedAlloc.allocate(4000, 8);
    char ch = *(ptr + 4096);

    return 0;
}
