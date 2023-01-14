#include "frontend/FrontEnd.h"
#include "RemniwLexer.h"
#include "RemniwParser.h"
#include "antlr4-runtime.h"
#include "frontend/ASTBuilder.h"
#include "llvm/Support/Debug.h"

using namespace antlr4;

#define DEBUG_TYPE "remniw-FrontEnd"

namespace remniw {

std::unique_ptr<ProgramAST> FrontEnd::parse(std::istream& Stream) {
    ANTLRInputStream Input(Stream);
    RemniwLexer Lexer(&Input);
    CommonTokenStream Tokens(&Lexer);
    Tokens.fill();
    LLVM_DEBUG({
        llvm::outs() << "===== Lexer ===== \n";
        for (auto token : Tokens.getTokens()) {
            llvm::outs() << token->toString() << "\n";
        }
    });

    RemniwParser Parser(&Tokens);
    RemniwParser::ProgramContext* Program = Parser.program();
    LLVM_DEBUG({
        llvm::outs() << "===== Parser ===== \n";
        llvm::outs() << Program->toStringTree(&Parser, true) << "\n";
        if (Parser.getNumberOfSyntaxErrors())
            llvm::errs() << "===== Parser Failed ===== \n";
    });

    ASTBuilder Builder(TheTypeContext);
    return Builder.build(Program);
}

}  // namespace remniw
