#pragma once

#include "llvm/Support/raw_ostream.h"
#include <cstdint>
#include <vector>

namespace remniw {

struct LiveRange {
    uint32_t StartPoint;
    uint32_t EndPoint;
    bool UsedAcrossCall;
    void print(llvm::raw_ostream &OS) const {
        OS << "[" << StartPoint << ", " << EndPoint << ")";
    }
};

// Lifetime interval containing holes
struct LiveRanges {
    std::vector<LiveRange> Ranges;
};

// Lifetime interval not containing holes
struct LiveInterval {
    uint32_t StartPoint;
    uint32_t EndPoint;
    uint32_t Reg;
    bool UsedAcrossCall;
};

}  // namespace remniw
