#include "codegen/asm/AsmCodeGenetator.h"
#include "codegen/asm/TargetInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include <memory>
#include <string>

static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                                llvm::cl::desc("<input bitcode>"),
                                                llvm::cl::init("-"));
static llvm::cl::opt<std::string>
    OutputFilename("o", llvm::cl::desc("Override output filename"), llvm::cl::init("-"),
                   llvm::cl::value_desc("filename"));

static llvm::cl::opt<remniw::Target> CodegenTarget(
    "target", llvm::cl::desc("Choose codegen target:"),
    llvm::cl::values(clEnumVal(remniw::Target::x86, "emit X86 assembly"),
                     clEnumVal(remniw::Target::riscv, "emit RISCV assembly")),
    llvm::cl::init(remniw::Target::x86));

int main(int argc, char *argv[]) {
    // parse arguments from command line
    llvm::cl::ParseCommandLineOptions(argc, argv, "remniw-llc\n");

    // prepare llvm context to read bitcode file
    llvm::LLVMContext Context;
    llvm::SMDiagnostic Error;
    std::unique_ptr<llvm::Module> M = parseIRFile(InputFilename, Error, Context);
    if (!M) {
        Error.print(argv[0], llvm::errs());
        return 1;
    }

    std::error_code EC;
    llvm::ToolOutputFile Out(OutputFilename, EC, llvm::sys::fs::OF_Text);
    remniw::AsmCodeGenerator CG(CodegenTarget);
    CG.compile(M.get(), Out.os());
    Out.keep();

    return 0;
}
