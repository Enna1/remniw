//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's aphotic_shield_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstddef>
#include <cstdint>

namespace aphotic_shield {
namespace options {

struct Options {
// Read the options from the included definitions file.
#define APHOTIC_SHIELD_OPTION(Type, Name, DefaultValue, Description)                     \
    Type Name = DefaultValue;
#include "options.inc"
#undef APHOTIC_SHIELD_OPTION

    void setDefaults() {
#define APHOTIC_SHIELD_OPTION(Type, Name, DefaultValue, Description) Name = DefaultValue;
#include "options.inc"
#undef APHOTIC_SHIELD_OPTION
    }
};

void initOptions(const char *OptionsStr);
Options &getOptions();

}  // namespace options
}  // namespace aphotic_shield
