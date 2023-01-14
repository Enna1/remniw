#pragma once

#include "codegen/asm/AsmFunction.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"

namespace remniw {

class AsmPrinter {
private:
    const TargetInfo &TI;
    llvm::raw_fd_ostream *OS;

public:
    AsmPrinter(const TargetInfo &TI): TI(TI), OS(&llvm::outs()) {}
    virtual ~AsmPrinter() = default;

    llvm::raw_fd_ostream &outStreamer() { return *OS; }

    void
    emitToStreamer(llvm::raw_fd_ostream &Out,
                   const llvm::SmallVector<std::unique_ptr<AsmFunction>> &AsmFunctions,
                   const llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> &GVs,
                   const llvm::SmallVector<llvm::Function *> &GlobalCtors) {
        OS = &Out;
        for (const auto &AsmFunc : AsmFunctions) {
            emitFunctionDeclaration(AsmFunc.get());
            emitFunctionBody(AsmFunc.get());
        }
        emitGlobalVariables(GVs);
        emitInitArray(GlobalCtors);
    }

    virtual void emitFunctionDeclaration(const AsmFunction *F) = 0;

    virtual void emitFunctionBody(const AsmFunction *F) = 0;

    virtual void emitGlobalVariables(
        const llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> &GVs) = 0;

    virtual void
    emitInitArray(const llvm::SmallVector<llvm::Function *> &GlobalCtors) = 0;

    virtual void PrintAsmInstruction(const AsmInstruction &I,
                                     llvm::raw_ostream &OS) const = 0;

    virtual void PrintAsmOperand(const AsmOperand &Op, llvm::raw_ostream &OS) const = 0;

    virtual void PrintRegister(uint32_t Reg, llvm::raw_ostream &OS) const = 0;
};

}  // namespace remniw
