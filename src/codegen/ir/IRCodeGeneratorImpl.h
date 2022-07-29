#pragma once

#include "AST.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <unordered_map>

namespace remniw {

class IRCodeGeneratorImpl {
private:
    llvm::LLVMContext *TheLLVMContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> IRB;
    std::unordered_map<std::string, llvm::Value *> NamedValues;
    llvm::GlobalVariable *InputFmtStr;
    llvm::GlobalVariable *OutputFmtStr;

public:
    IRCodeGeneratorImpl(llvm::LLVMContext *);
    std::unique_ptr<llvm::Module> codegen(ProgramAST *);
    llvm::Value *codegenFunction(FunctionAST *);
    llvm::Value *codegenNumberExpr(NumberExprAST *);
    llvm::Value *codegenVariableExpr(VariableExprAST *);
    llvm::Value *codegenVarDeclNode(VarDeclNodeAST *);
    llvm::Value *codegenFunctionCallExpr(FunctionCallExprAST *);
    llvm::Value *codegenNullExpr(NullExprAST *);
    llvm::Value *codegenSizeofExpr(SizeofExprAST *);
    llvm::Value *codegenRefExpr(RefExprAST *);
    llvm::Value *codegenDerefExpr(DerefExprAST *);
    llvm::Value *codegenArraySubscriptExpr(ArraySubscriptExprAST *);
    llvm::Value *codegenInputExpr(InputExprAST *);
    llvm::Value *codegenBinaryExpr(BinaryExprAST *);
    llvm::Value *codegenLocalVarDeclStmt(LocalVarDeclStmtAST *);
    llvm::Value *codegenEmptyStmt(EmptyStmtAST *);
    llvm::Value *codegenOutputStmt(OutputStmtAST *);
    llvm::Value *codegenAllocStmt(AllocStmtAST *);
    llvm::Value *codegenDeallocStmt(DeallocStmtAST *);
    llvm::Value *codegenBlockStmt(BlockStmtAST *);
    llvm::Value *codegenReturnStmt(ReturnStmtAST *);
    llvm::Value *codegenIfStmt(IfStmtAST *);
    llvm::Value *codegenWhileStmt(WhileStmtAST *);
    llvm::Value *codegenAssignmentStmt(AssignmentStmtAST *);

private:
    llvm::Value *codegenExpr(ExprAST *);
    llvm::Value *codegenStmt(StmtAST *);
    llvm::Type *REMNIWTypeToLLVMType(remniw::Type *);
    llvm::Value *emitLibCall(llvm::StringRef LibFuncName, llvm::Type *ReturnType,
                             llvm::ArrayRef<llvm::Type *> ParamTypes,
                             llvm::ArrayRef<llvm::Value *> Operands,
                             bool IsVaArgs = false);
    llvm::Value *emitPrintf(llvm::Value *Fmt, llvm::Value *VAList);
    llvm::Value *emitScanf(llvm::Value *Fmt, llvm::Value *VAList);
    llvm::Value *emitMalloc(llvm::Type *ReturnType, llvm::Value *Size);
    llvm::Value *emitFree(llvm::Value *Ptr);
    uint64_t getSizeOfREMNIWType(remniw::Type *);
};

}  // namespace remniw