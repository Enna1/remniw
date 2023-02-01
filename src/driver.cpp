#include "antlr4-runtime.h"
#include "codegen/asm/AsmCodeGenetator.h"
#include "codegen/asm/TargetInfo.h"
#include "codegen/ir/IRCodeGenerator.h"
#include "config.h"
#include "frontend/AST.h"
#include "frontend/ASTPrinter.h"
#include "frontend/FrontEnd.h"
#include "semantic/SymbolTable.h"
#include "semantic/TypeAnalysis.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
// #include <unistd.h>

using namespace antlr4;
using namespace remniw;

#define DEBUG_TYPE "remniw-Driver"

llvm::cl::OptionCategory RemniwCat("remniw compiler options");

static llvm::cl::opt<bool> EmitLLVM(
    "emit-llvm",
    llvm::cl::desc("Output LLVM IR (human-readable LLVM assembly language format)"),
    llvm::cl::init(false), llvm::cl::cat(RemniwCat));

static llvm::cl::opt<std::string>
    InputFilename(llvm::cl::Positional, llvm::cl::desc("<input remniw source code>"),
                  llvm::cl::cat(RemniwCat));

static llvm::cl::opt<std::string>
    OutputFilename("o", llvm::cl::desc("Override output filename"),
                   llvm::cl::init("a.out"), llvm::cl::value_desc("filename"),
                   llvm::cl::cat(RemniwCat));

static llvm::cl::opt<Target> CodegenTarget(
    "target", llvm::cl::desc("Choose codegen target:"), llvm::cl::cat(RemniwCat),
    llvm::cl::values(clEnumVal(x86, "X86 assembly"), clEnumVal(riscv, "RISCV assembly")),
    llvm::cl::init(x86));

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

    LLVM_DEBUG(llvm::outs() << "===== IR Code Generator ===== \n");
    IRCodeGenerator IRCG(&TheLLVMContext);
    std::unique_ptr<llvm::Module> M = IRCG.emit(AST.get());

    LLVM_DEBUG(M->print(llvm::outs(), nullptr));
    if (EmitLLVM) {
        std::error_code EC;
        llvm::ToolOutputFile Out(OutputFilename, EC, llvm::sys::fs::OF_Text);
        if (EC) {
            llvm::errs() << EC.message() << '\n';
            return 1;
        }
        // WriteBitcodeToFile(*M.get(), Out.os());
        M->print(Out.os(), nullptr);
        Out.keep();
        return 0;
    }

    LLVM_DEBUG(llvm::outs() << "===== Asm Code Generator ===== \n");
    llvm::SmallString<64> TempPath;
    int FD;
    if (llvm::sys::fs::createTemporaryFile(llvm::sys::path::filename(OutputFilename), "s",
                                           FD, TempPath)) {
        llvm::errs() << "createTemporaryFile failed\n";
        return 1;
    }
    llvm::raw_fd_ostream TmpOut(FD, /*shouldClose=*/true);
    AsmCodeGenerator ASMCG(CodegenTarget);
    ASMCG.compile(M.get(), TmpOut);
    TmpOut.close();

    // Invoke clang to compile and link assembly code to executable file.
    llvm::SmallVector<llvm::StringRef> CCParams;
    {
        CCParams.push_back("clang");
        CCParams.push_back(TempPath.c_str());                    /* assembly filename */
        CCParams.push_back("-L" CMAKE_LIBRARY_OUTPUT_DIRECTORY); /* see config.h.in */
        CCParams.push_back("-Wl,-whole-archive");
        CCParams.push_back("-laphotic_shield"); /* link aphotic_shield */
        CCParams.push_back("-Wl,-no-whole-archive");
        CCParams.push_back("-o");
        CCParams.push_back(OutputFilename.c_str()); /* executable filename */
        // CCParams.push_back("-v");
    };
    std::string ErrMsg;
    llvm::ErrorOr<std::string> Program = llvm::sys::findProgramByName("clang");
    if (!Program)
        ErrMsg = Program.getError().message();
    if (llvm::sys::ExecuteAndWait(Program.get(), CCParams, llvm::None, {}, 0, 0,
                                  &ErrMsg)) {
        llvm::errs() << "execvp(clang) failed: " << ErrMsg << '\n';
        exit(EXIT_FAILURE);
    }

    return 0;
}
