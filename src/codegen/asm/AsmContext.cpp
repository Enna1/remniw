#include "AsmContext.h"
#include "AsmSymbol.h"
#include "llvm/ADT/Twine.h"

namespace remniw {

AsmSymbol* AsmContext::getOrCreateSymbol(llvm::Value* V) {
    if (SymbolTable.count(V))
        return SymbolTable[V];

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

    AsmSymbol* Symbol = nullptr;
    if (auto* BB = llvm::dyn_cast<llvm::BasicBlock>(V))
        Symbol = new AsmSymbol(AsmSymbol::SymbolKindBasicBlock, NewName);
    if (auto* F = llvm::dyn_cast<llvm::Function>(V))
        Symbol = new AsmSymbol(AsmSymbol::SymbolKindFunction, NewName);
    if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(V))
        Symbol = new AsmSymbol(AsmSymbol::SymbolKindGlobalVariable, NewName);
    SymbolTable[V] = Symbol;
    return Symbol;
}

}  // namespace remniw
