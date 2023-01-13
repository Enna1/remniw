#pragma once

#include "codegen/asm/AsmBuilder.h"
#include "codegen/asm/AsmContext.h"
#include "codegen/asm/AsmPrinter.h"
#include "codegen/asm/AsmRewriter.h"
#include "codegen/asm/BrgTreeBuilder.h"
#include "codegen/asm/TargetInfo.h"
#include "codegen/asm/X86/X86AsmBuilder.h"
#include "codegen/asm/X86/X86AsmPrinter.h"
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

            X86AsmPrinter Printer(AB.getTargetInfo(), OS, AsmFunctions,
                                  BB.getConstantStrings(), BB.getGlobalCtors());
            Printer.emitToStreamer();
        }
    }

private:
    void initializeTarget() {
        // if (TheTarget == Target::x86) {
        //     AB = std::make_unique<X86AsmBuilder>();
        //     AR = std::make_unique<X86AsmRewriter>(AB->getTargetRegisterInfo());
        //     // AP = std::make_unique<AsmPrinter>();
        // }
        // else if (TheTarget == Target::riscv) {

        // }
    }

private:
    Target TheTarget;
    AsmContext AsmCtx;
    std::unique_ptr<AsmBuilder> AB;
    std::unique_ptr<AsmRewriter> AR;
    // std::unique_ptr<AsmPrinter> AP;
};

}  // namespace remniw
