#pragma once

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

namespace remniw {

class AsmSymbol {
public:
    enum SymbolKindTy
    {
        SymbolKindFunction,
        SymbolKindBasicBlock,
        SymbolKindGlobalVariable,
    };

    AsmSymbol(SymbolKindTy Kind, std::string Name):
        Kind(Kind), Name(Name), BasicBlockPrefix(".L"), GlobalPrefix(".L") {}

    bool isFunction() const { return Kind == SymbolKindFunction; }

    bool isBasicblock() const { return Kind == SymbolKindBasicBlock; }

    bool isGlobalVariable() const { return Kind == SymbolKindGlobalVariable; }

    std::string getName() const { return Name; }

    void print(llvm::raw_ostream& OS) const {
        if (Kind == SymbolKindFunction) {
            OS << Name;
        }
        if (Kind == SymbolKindBasicBlock) {
            OS << BasicBlockPrefix << Name;
        }
        if (Kind == SymbolKindGlobalVariable) {
            OS << GlobalPrefix << Name;
        }
    }

private:
    SymbolKindTy Kind;
    std::string Name;
    llvm::StringRef BasicBlockPrefix;
    llvm::StringRef GlobalPrefix;
};

}  // namespace remniw
