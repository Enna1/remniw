#pragma once

#include "AsmBuilder.h"
#include "AsmContext.h"
#include "AsmPrinter.h"
#include "AsmRewriter.h"
#include "BrgTreeBuilder.h"
#include "TargetInfo.h"
#include "codegen/asm/X86/X86AsmBuilder.h"
#include "codegen/asm/X86/X86AsmRewriter.h"

namespace remniw {

class AsmCodeGenerator {
public:
    AsmCodeGenerator(Target TheTarget): TheTarget(TheTarget), AsmCtx() {}

    void compile(llvm::Module *M, llvm::raw_ostream &OS) {
        if (TheTarget == Target::x86) {
            X86AsmBuilder AB;
            BrgTreeBuilder BB(M->getDataLayout(), AB.getTargetInfo(), AsmCtx);
            BB.visit(*M);
            AB.build(BB.getFunctions());

            llvm::SmallVector<AsmFunction *> &AsmFunctions = AB.getAsmFunctions();
            X86AsmRewriter Rewriter(AB.getTargetInfo());
            Rewriter.rewrite(AsmFunctions);

            AsmPrinter Printer(AB.getTargetInfo(), OS, AsmFunctions,
                               BB.getConstantStrings(), BB.getGlobalCtors());
            Printer.print();
        }
    }

private:
    Target TheTarget;
    AsmContext AsmCtx;
};

}  // namespace remniw
