#pragma once

#include "AsmBuilder.h"
#include "AsmContext.h"
#include "AsmPrinter.h"
#include "AsmRewriter.h"
#include "BrgTreeBuilder.h"

namespace remniw {

class AsmCodeGenerator {
public:
    AsmCodeGenerator(AsmBuilder& AB, llvm::Module *M, llvm::raw_ostream &OS):
        AB(AB), OS(OS), DL(M->getDataLayout()), AsmCtx() {
        BrgTreeBuilder BB(DL, AsmCtx);
        BB.visit(*M);
        AB.build(BB.getFunctions());
        AsmRewriter Rewriter(AB.getAsmFunctions());
        remniw::AsmPrinter Printer(OS, Rewriter.getAsmFunctions(),
                                   BB.getConstantStrings(),
                                   BB.getGlobalCtors());
        Printer.print();
    }

private:
    AsmBuilder& AB;
    llvm::raw_ostream &OS;
    const llvm::DataLayout &DL;
    AsmContext AsmCtx;
};

}  // namespace remniw
