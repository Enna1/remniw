#pragma once

#include "AsmFunction.h"
#include "llvm/ADT/SmallVector.h"

namespace remniw {

class AsmPrinter {
private:
    llvm::raw_ostream &OS;
    llvm::SmallVector<AsmFunction*> AsmFunctions;
    llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> GlobalVariables;

public:
    AsmPrinter(llvm::raw_ostream &OS, llvm::SmallVector<AsmFunction*> AsmFunctions,
               llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> GVs):
        OS(OS),
        AsmFunctions(AsmFunctions), GlobalVariables(GVs) {}

    void print() {
        for (auto &AsmFunc : AsmFunctions) {
            EmitFunctionDeclaration(AsmFunc);
            EmitFunctionBody(AsmFunc);
        }
        EmitGlobalVariables();
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
            AsmInst.print(OS);
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
};

}  // namespace remniw
