#include "AST.h"
#include "ASTPrinter.h"
#include "ir/IRCodeGenerator.h"
#include "FrontEnd.h"
#include "SymbolTable.h"
#include "Type.h"
#include "TypeAnalysis.h"
#include "antlr4-runtime.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>

using namespace antlr4;
using namespace remniw;

#define DEBUG_TYPE "remniw"

static llvm::cl::OptionCategory RemniwCat("remniw compiler options");

static llvm::cl::opt<std::string>
    InputFilename(llvm::cl::Positional, llvm::cl::desc("<input remniw source code>"),
                  llvm::cl::cat(RemniwCat));

static llvm::cl::opt<std::string>
    OutputFilename("o", llvm::cl::desc("Override output filename"),
                   llvm::cl::init("a.out"), llvm::cl::value_desc("filename"),
                   llvm::cl::cat(RemniwCat));

int main(int argc, char* argv[]) {
    llvm::cl::HideUnrelatedOptions(RemniwCat);
    llvm::cl::SetVersionPrinter(
        [](llvm::raw_ostream& OS) { OS << "remniw compiler 0.1\n"; });
    llvm::cl::ParseCommandLineOptions(argc, argv, "remniw compiler\n");

    llvm::LLVMContext TheLLVMContext;
    remniw::TypeContext TheTypeContext;

    std::ifstream Stream;
    Stream.open(InputFilename);
    if (!Stream.good()) {
        llvm::errs() << "error: no such file: '" << InputFilename << "'\n";
        return 1;
    }
    FrontEnd FE(TheTypeContext);
    std::unique_ptr<ProgramAST> AST = FE.parse(Stream);

    LLVM_DEBUG({
        llvm::outs() << "===== AST Printer ===== \n";
        ASTPrinter PrettyPrinter(llvm::outs());
        PrettyPrinter.print(AST.get());
    });

    LLVM_DEBUG(llvm::outs() << "===== Symbol Table ===== \n");
    SymbolTableBuilder SymTabBuilder;
    SymTabBuilder.build(AST.get());
    LLVM_DEBUG(SymTabBuilder.getSymbolTale().print(llvm::outs()));

    LLVM_DEBUG(llvm::outs() << "===== Type Analysis ===== \n");
    TypeAnalysis TA(SymTabBuilder.getSymbolTale(), TheTypeContext);
    TA.solve(AST.get());
    LLVM_DEBUG({
        for (auto Constraint : TA.getConstraints())
            Constraint.print(llvm::outs());
    });

    LLVM_DEBUG(llvm::outs() << "===== Code Generator ===== \n");
    IRCodeGenerator CG(&TheLLVMContext);
    std::unique_ptr<llvm::Module> M = CG.emit(AST.get());

    std::error_code EC;
    llvm::ToolOutputFile Out(OutputFilename, EC, llvm::sys::fs::OF_Text);
    if (EC) {
        llvm::errs() << EC.message() << '\n';
        return 1;
    }
    LLVM_DEBUG(M->print(llvm::outs(), nullptr));
    // WriteBitcodeToFile(*M.get(), Out.os());
    M->print(Out.os(), nullptr);
    Out.keep();

    return 0;
}
