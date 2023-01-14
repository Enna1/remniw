#pragma once

#include "codegen/ir/IRCodeGeneratorImpl.h"
#include "frontend/AST.h"
#include "llvm/IR/LLVMContext.h"

namespace remniw {

class IRCodeGenerator {
private:
    std::unique_ptr<IRCodeGeneratorImpl> pImpl;

public:
    IRCodeGenerator(llvm::LLVMContext* LLVMContext):
        pImpl(std::make_unique<IRCodeGeneratorImpl>(LLVMContext)) {}
    std::unique_ptr<llvm::Module> emit(ProgramAST*);
};

}  // namespace remniw