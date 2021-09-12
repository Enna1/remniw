#include "AsmCodeGenetator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"

using namespace llvm;

static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                                llvm::cl::desc("<input bitcode>"),
                                                llvm::cl::init("-"));
static llvm::cl::opt<std::string>
    OutputFilename("o", llvm::cl::desc("Override output filename"),
                   llvm::cl::init("-"), llvm::cl::value_desc("filename"));

int main(int argc, char *argv[]) {
    // parse arguments from command line
    llvm::cl::ParseCommandLineOptions(argc, argv, "llc-olive\n");

    // prepare llvm context to read bitcode file
    llvm::LLVMContext Context;
    llvm::SMDiagnostic Error;
    std::unique_ptr<llvm::Module> M = parseIRFile(InputFilename, Error, Context);

    std::error_code EC;
    llvm::ToolOutputFile Out(OutputFilename, EC, llvm::sys::fs::OF_Text);

    remniw::AsmCodeGenerator CG(M.get(), Out.os());
    Out.keep();

    return 0;
}
