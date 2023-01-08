#pragma once

#include "codegen/ir/IRCodeGeneratorImpl.h"
#include "frontend/AST.h"
#include "llvm/IR/LLVMContext.h"

namespace remniw {

class IRCodeGenerator {
private:
    IRCodeGeneratorImpl* pImpl;

public:
    IRCodeGenerator(llvm::LLVMContext* LLVMContext):
        pImpl(new IRCodeGeneratorImpl(LLVMContext)) {}
    ~IRCodeGenerator() { delete pImpl; }
    std::unique_ptr<llvm::Module> emit(ProgramAST*);
};

}  // namespace remniw