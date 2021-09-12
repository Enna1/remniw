#pragma once

#include "AsmBuilder.h"
#include "AsmContext.h"
#include "AsmPrinter.h"
#include "AsmRewriter.h"
#include "BrgTreeBuilder.h"

namespace remniw {

class AsmCodeGenerator {
public:
    AsmCodeGenerator(llvm::Module *M, llvm::raw_ostream &OS):
        OS(OS), DL(M->getDataLayout()), AsmCtx() {
        BrgTreeBuilder BBuilder(DL, AsmCtx);
        BBuilder.visit(*M);
        AsmBuilder ABuilder(AsmCtx, BBuilder.getFunctions());
        AsmRewriter Rewriter(ABuilder.getAsmFunctions());
        remniw::AsmPrinter Printer(OS, Rewriter.getAsmFunctions(),
                                   BBuilder.getConstantStrings());
        Printer.print();
    }

private:
    llvm::raw_ostream &OS;
    const llvm::DataLayout &DL;
    AsmContext AsmCtx;
};

}  // namespace remniw
