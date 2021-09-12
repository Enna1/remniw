#pragma once

#include "AST.h"
#include "RecursiveASTVisitor.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseMapInfo.h"

namespace remniw {

struct SymbolTable {
    SymbolTable() {}

    bool addFunction(llvm::StringRef FunctionName, FunctionAST *Function) {
        return Functions.insert({FunctionName, Function}).second;
    }

    bool addVariable(llvm::StringRef VariableName, VarDeclNodeAST *Variable,
                     FunctionAST *Function) {
        return Variables.insert({std::make_pair(VariableName, Function), Variable})
            .second;
    }

    bool addReturnExpr(ExprAST *ReturnExpr, FunctionAST *Function) {
        return ReturnExprs.insert({ReturnExpr, Function}).second;
    }

    FunctionAST *getFunction(llvm::StringRef FunctionName) {
        if (Functions.count(FunctionName))
            return Functions[FunctionName];
        return nullptr;
    }

    VarDeclNodeAST *getVariable(llvm::StringRef VariableName, FunctionAST *Function) {
        auto Key = std::make_pair(VariableName, Function);
        if (Variables.count(Key))
            return Variables[Key];
        return nullptr;
    }

    void print(llvm::raw_ostream &OS) {
        for (auto &p : Functions)
            OS << "Function: '" << p.first << "' " << p.second << "\n";
        for (auto &p : Variables)
            OS << "Variable: '" << p.first.first << "' " << p.second << " (of function '"
               << p.first.first << "')\n";
        for (auto &p : ReturnExprs)
            OS << "ReturnExpr: '" << p.first << "' (of function '"
               << p.second->getFuncName() << "')\n";
    }

    // < FunctionName, FunctionAST* >
    llvm::DenseMap<llvm::StringRef, FunctionAST *> Functions;
    // < <VariableName, FunctionAST*>, VarDeclNodeAST* >
    llvm::DenseMap<std::pair<llvm::StringRef, FunctionAST *>, VarDeclNodeAST *> Variables;
    // <ExprAST of ReturnStmt, FunctionAST*>
    llvm::DenseMap<ExprAST *, FunctionAST *> ReturnExprs;
};

class SymbolTableBuilder: public RecursiveASTVisitor<SymbolTableBuilder> {
public:
    SymbolTableBuilder(): SymTab(), CurrentFunction(nullptr) {}

    void build(ProgramAST *AST) { visitProgram(AST); }

    SymbolTable &getSymbolTale() { return SymTab; }

    bool actBeforeVisitFunction(FunctionAST *Function) {
        CurrentFunction = Function;
        return false;
    }

    void actAfterVisitFunction(FunctionAST *Function) {
        SymTab.addFunction(CurrentFunction->getFuncName(), Function);
    }

    void actAfterVisitVarDeclNode(VarDeclNodeAST *VarDeclNode) {
        SymTab.addVariable(VarDeclNode->getName(), VarDeclNode, CurrentFunction);
    }

    void actAfterVisitReturnStmt(ReturnStmtAST *ReturnStmt) {
        SymTab.addReturnExpr(ReturnStmt->getExpr(), CurrentFunction);
    }

private:
    SymbolTable SymTab;
    FunctionAST *CurrentFunction;
};

}  // namespace remniw
