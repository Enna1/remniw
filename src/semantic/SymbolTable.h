#pragma once

#include "frontend/AST.h"
#include "frontend/RecursiveASTVisitor.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseMapInfo.h"

namespace remniw {

struct SymbolTable {
    SymbolTable() {}

    bool addFunction(llvm::StringRef FunctionName, FunctionDeclAST *Function) {
        return Functions.insert({FunctionName, Function}).second;
    }

    bool addVariable(llvm::StringRef VariableName, VarDeclAST *Variable,
                     FunctionDeclAST *Function) {
        return VariableDecls.insert({std::make_pair(VariableName, Function), Variable})
            .second;
    }

    bool addVariableRef(VariableExprAST *VariableExpr, ASTNode *VarOrFuncDecl) {
        return VariableRefs.insert({VariableExpr, VarOrFuncDecl}).second;
    }

    bool addReturnExpr(ExprAST *ReturnExpr, FunctionDeclAST *Function) {
        return ReturnExprs.insert({ReturnExpr, Function}).second;
    }

    FunctionDeclAST *getFunction(llvm::StringRef FunctionName) {
        if (Functions.count(FunctionName))
            return Functions[FunctionName];
        return nullptr;
    }

    VarDeclAST *getVariable(llvm::StringRef VariableName, FunctionDeclAST *Function) {
        auto Key = std::make_pair(VariableName, Function);
        if (VariableDecls.count(Key))
            return VariableDecls[Key];
        return nullptr;
    }

    ASTNode *getDeclForVariableExpr(const VariableExprAST *VariableExpr) {
        if (VariableRefs.count(VariableExpr))
            return VariableRefs.lookup(VariableExpr);
        return nullptr;
    }

    void print(llvm::raw_ostream &OS) {
        for (auto &p : Functions)
            OS << "Function: '" << p.first << "' " << p.second << "\n";
        for (auto &p : VariableDecls)
            OS << "Variable: '" << p.first.first << "' " << p.second << " (of function '"
               << p.first.second->getName() << "')\n";
        for (auto &p : ReturnExprs)
            OS << "ReturnExpr: '" << p.first << "' (of function '"
               << p.second->getName() << "')\n";
    }

    // < FunctionName, FunctionDeclAST* >
    llvm::DenseMap<llvm::StringRef, FunctionDeclAST *> Functions;
    // < <VariableName, FunctionDeclAST*>, VarDeclAST* >
    llvm::DenseMap<std::pair<llvm::StringRef, FunctionDeclAST *>, VarDeclAST *> VariableDecls;
    // < ExprAST of ReturnStmt, FunctionDeclAST* >
    llvm::DenseMap<ExprAST *, FunctionDeclAST *> ReturnExprs;
    // < VariableExprAST*, ASTNode*(VarDeclAST* or FunctionDeclAST*) >
    llvm::DenseMap<VariableExprAST *, ASTNode *> VariableRefs;
};

class SymbolTableBuilder: public RecursiveASTVisitor<SymbolTableBuilder> {
public:
    SymbolTableBuilder(): SymTab(), CurrentFunction(nullptr) {}

    void build(ProgramAST *AST) { visitProgram(AST); }

    SymbolTable &getSymbolTale() { return SymTab; }

    bool actBeforeVisitProgram(ProgramAST * Program) {
        for (auto *Function : Program->getFunctions())
            SymTab.addFunction(Function->getName(), Function);
        return false;
    }

    bool actBeforeVisitFunction(FunctionDeclAST *Function) {
        CurrentFunction = Function;
        return false;
    }

    void actAfterVisitVarDeclNode(VarDeclAST *VarDeclNode) {
        SymTab.addVariable(VarDeclNode->getName(), VarDeclNode, CurrentFunction);
    }

    void actAfterVisitReturnStmt(ReturnStmtAST *ReturnStmt) {
        SymTab.addReturnExpr(ReturnStmt->getExpr(), CurrentFunction);
    }

    void actAfterVisitVariableExpr(VariableExprAST * VariableExpr) {
        if (auto *VarDeclNode =
            SymTab.getVariable(VariableExpr->getName(), CurrentFunction)) {
            SymTab.addVariableRef(VariableExpr, VarDeclNode);
        } else if (auto *Function = SymTab.getFunction(VariableExpr->getName())) {
            SymTab.addVariableRef(VariableExpr, Function);
        }
    }

private:
    SymbolTable SymTab;
    FunctionDeclAST *CurrentFunction = nullptr;
};

}  // namespace remniw
