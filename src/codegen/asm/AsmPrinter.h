#pragma once

#include "codegen/asm/AsmFunction.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"

namespace remniw {

class AsmPrinter {
private:
    const TargetInfo &TI;
    llvm::raw_ostream &OS;
    llvm::SmallVector<AsmFunction *> &AsmFunctions;
    llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> GlobalVariables;
    llvm::SmallVector<llvm::Function *> GlobalCtors;

public:
    AsmPrinter(const TargetInfo &TI, llvm::raw_ostream &OS,
               llvm::SmallVector<AsmFunction *> &AsmFunctions,
               llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> GVs,
               llvm::SmallVector<llvm::Function *> GlobalCtors):
        TI(TI),
        OS(OS), AsmFunctions(AsmFunctions), GlobalVariables(GVs),
        GlobalCtors(GlobalCtors) {}

    void print() {
        for (auto *AsmFunc : AsmFunctions) {
            EmitFunctionDeclaration(AsmFunc);
            EmitFunctionBody(AsmFunc);
        }
        EmitGlobalVariables();
        EmitInitArray();
    }

    virtual void EmitFunctionDeclaration(AsmFunction *F) {}

    virtual void EmitFunctionBody(AsmFunction *F) {}

    virtual void EmitGlobalVariables() {}

    void EmitInitArray() {}
};

}  // namespace remniw
