#include "AsmContext.h"
#include "AsmSymbol.h"
#include "llvm/ADT/Twine.h"

namespace remniw {

AsmSymbol* AsmContext::getOrCreateSymbol(llvm::Value* V) {
    if (SymbolTable.count(V))
        return SymbolTable[V].get();

    llvm::StringRef Name = V->getName();
    if (Name.empty()) {
        Name = "tmp";
    }
    std::string NewName = Name.str();
    if (UsedNames.count(Name)) {
        unsigned& NextUniqueID = NextID[Name];
        NewName += std::to_string(NextUniqueID++);
    } else {
        UsedNames.insert(std::make_pair(Name, true));
    }

    if (auto* BB = llvm::dyn_cast<llvm::BasicBlock>(V)) {
        SymbolTable[V] =
            std::make_unique<AsmSymbol>(AsmSymbol::SymbolKindBasicBlock, NewName);
    } else if (auto* F = llvm::dyn_cast<llvm::Function>(V)) {
        SymbolTable[V] =
            std::make_unique<AsmSymbol>(AsmSymbol::SymbolKindFunction, NewName);
    } else if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(V)) {
        SymbolTable[V] =
            std::make_unique<AsmSymbol>(AsmSymbol::SymbolKindGlobalVariable, NewName);
    } else {
        llvm_unreachable("Unexpected AsmSymbol");
    }
    return SymbolTable[V].get();
}

}  // namespace remniw
