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
    AsmCodeGenerator(Target TheTarget): TheTarget(TheTarget) { initializeTarget(); }

    void compile(llvm::Module *M, llvm::raw_fd_ostream &OS) {
        // LLVM IR -> BrgTree
        BB->build(*M);
        const auto &BrgFunctions = BB->getFunctions();

        // BrgTree -> Assembly
        AB->build(BrgFunctions);
        auto &AsmFunctions = AB->getAsmFunctions();

        // Register allocation, insert prologue and epilogue
        AR->rewrite(AsmFunctions);

        // Emit assembly to file stream
        AP->emitToStreamer(OS, AsmFunctions, BB->getConstantStrings(), BB->getGlobalCtors());
    }

private:
    void initializeTarget() {
        if (TheTarget == Target::x86) {
            AB = std::make_unique<X86AsmBuilder>();
            AR = std::make_unique<X86AsmRewriter>(AB->getTargetInfo());
            AP = std::make_unique<X86AsmPrinter>(AB->getTargetInfo());
            BB = std::make_unique<BrgTreeBuilder>(AB->getTargetInfo(), AsmCtx);
        } else if (TheTarget == Target::riscv) {
            llvm_unreachable("unimplemented");
        }
    }

private:
    Target TheTarget;
    AsmContext AsmCtx;
    std::unique_ptr<AsmBuilder> AB;
    std::unique_ptr<AsmRewriter> AR;
    std::unique_ptr<AsmPrinter> AP;
    std::unique_ptr<BrgTreeBuilder> BB;
};

}  // namespace remniw
