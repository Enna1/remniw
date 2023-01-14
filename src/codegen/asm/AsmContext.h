#pragma once

#include "AsmSymbol.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Value.h"

namespace remniw {

class AsmContext {
public:
    AsmSymbol *getOrCreateSymbol(llvm::Value *V);

private:
    llvm::DenseMap<llvm::Value *, std::unique_ptr<AsmSymbol>> SymbolTable;
    llvm::StringMap<bool> UsedNames;
    // The next ID to dole out to an unnamed assembler temporary symbol with
    // a given prefix.
    llvm::StringMap<unsigned> NextID;
};

}  // namespace remniw
