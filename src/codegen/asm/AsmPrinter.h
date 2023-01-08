#pragma once

#include "AsmFunction.h"
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

    void EmitFunctionDeclaration(AsmFunction *F) {
        // FIXME
        OS << ".text\n"
           << ".globl " << F->FuncName << "\n"
           << ".type " << F->FuncName << ", @function\n"
           << F->FuncName << ":\n";
    }

    void EmitFunctionBody(AsmFunction *F) {
        for (auto &AsmInst : *F) {
            TI.print(AsmInst, OS);
        }
    }

    void EmitGlobalVariables() {
        for (auto p : GlobalVariables) {
            p.first->print(OS);
            OS << ":\n";
            OS << "\t.asciz ";
            OS << "\"";
            OS.write_escaped(p.second);
            OS << "\"\n";
        }
    }

    void EmitInitArray() {
        for (auto *F : GlobalCtors) {
            OS << ".section\t.init_array,\"aw\",@init_array\n";
            OS << ".p2align\t3\n";
            OS << ".quad\t" << F->getName() << "\n";
        }
    }
};

}  // namespace remniw
