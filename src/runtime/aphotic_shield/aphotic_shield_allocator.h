#pragma once

#include <stddef.h>

namespace aphotic_shield {

void initAphoticShield();
void *AphoticShieldAllocate(size_t Size, size_t Alignment);
void AphoticShieldDeallocate(void *Ptr, size_t Size, size_t Alignment);

}  // namespace aphotic_shield
