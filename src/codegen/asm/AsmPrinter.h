#pragma once

#include "codegen/asm/AsmFunction.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"

namespace remniw {

class AsmPrinter {
protected:
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

    void emitToStreamer() {
        for (auto *AsmFunc : AsmFunctions) {
            emitFunctionDeclaration(AsmFunc);
            emitFunctionBody(AsmFunc);
        }
        emitGlobalVariables();
        emitInitArray();
    }

    virtual void emitFunctionDeclaration(AsmFunction *F) = 0;

    virtual void emitFunctionBody(AsmFunction *F) = 0;

    virtual void emitGlobalVariables() = 0;

    virtual void emitInitArray() = 0;

    virtual void PrintAsmInstruction(const AsmInstruction &I,
                                     llvm::raw_ostream &OS) const = 0;

    virtual void PrintAsmOperand(const AsmOperand &Op, llvm::raw_ostream &OS) const = 0;

    virtual void PrintRegister(uint32_t Reg, llvm::raw_ostream &OS) const = 0;
};

}  // namespace remniw
