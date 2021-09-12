#pragma once

#include "AsmInstruction.h"
#include "LiveInterval.h"
#include "llvm/ADT/ilist.h"
#include <cstdint>
#include <string>
#include <unordered_map>

namespace remniw {

class AsmFunction {
public:
    using InstListType = llvm::ilist<AsmInstruction>;

    AsmFunction(std::string FuncName, int64_t StackSizeInBytes):
        FuncName(FuncName), StackSizeInBytes(StackSizeInBytes) {
        InstList.Parent = this;
    }

    std::string FuncName;
    int64_t StackSizeInBytes;
    InstListType InstList;
    std::unordered_map<uint32_t, remniw::LiveRanges> RegLiveRangesMap;

    AsmFunction(const AsmFunction &) = delete;
    AsmFunction &operator=(const AsmFunction &) = delete;

    std::unordered_map<uint32_t, remniw::LiveRanges> &getRegLiveRangesMap() {
        return RegLiveRangesMap;
    }

    // Required by llvm::ilist_node_with_parent
    // Returns a pointer to a member of the instruction list.
    static InstListType AsmFunction::*getSublistAccess(AsmInstruction *) {
        return &AsmFunction::InstList;
    }

    // Return the underlying instruction list container.
    // Currently you need to access the underlying instruction list container
    // directly if you want to modify it.
    const InstListType &getInstList() const { return InstList; }
    InstListType &getInstList() { return InstList; }

    // Instruction iterators...
    using iterator = InstListType::iterator;
    using const_iterator = InstListType::const_iterator;
    using reverse_iterator = InstListType::reverse_iterator;
    using const_reverse_iterator = InstListType::const_reverse_iterator;

    // Instruction iterator methods
    inline iterator begin() { return InstList.begin(); }
    inline const_iterator begin() const { return InstList.begin(); }
    inline iterator end() { return InstList.end(); }
    inline const_iterator end() const { return InstList.end(); }

    inline reverse_iterator rbegin() { return InstList.rbegin(); }
    inline const_reverse_iterator rbegin() const { return InstList.rbegin(); }
    inline reverse_iterator rend() { return InstList.rend(); }
    inline const_reverse_iterator rend() const { return InstList.rend(); }

    inline size_t size() const { return InstList.size(); }
    inline bool empty() const { return InstList.empty(); }
    inline const AsmInstruction &front() const { return InstList.front(); }
    inline AsmInstruction &front() { return InstList.front(); }
    inline const AsmInstruction &back() const { return InstList.back(); }
    inline AsmInstruction &back() { return InstList.back(); }
};

};  // namespace remniw
