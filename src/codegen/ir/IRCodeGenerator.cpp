#include "IRCodeGenerator.h"
#include "llvm/IR/Module.h"

namespace remniw {

std::unique_ptr<llvm::Module> IRCodeGenerator::emit(ProgramAST *AST) {
    return pImpl->codegen(AST);
}

}  // namespace remniw