#pragma once

#include "frontend/AST.h"
#include "frontend/RecursiveASTVisitor.h"
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
        return VariableDecls.insert({std::make_pair(VariableName, Function), Variable})
            .second;
    }

    bool addVariableRef(VariableExprAST *VariableExpr, ASTNode *VarOrFuncDecl) {
        return VariableRefs.insert({VariableExpr, VarOrFuncDecl}).second;
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
               << p.first.second->getFuncName() << "')\n";
        for (auto &p : ReturnExprs)
            OS << "ReturnExpr: '" << p.first << "' (of function '"
               << p.second->getFuncName() << "')\n";
    }

    // < FunctionName, FunctionAST* >
    llvm::DenseMap<llvm::StringRef, FunctionAST *> Functions;
    // < <VariableName, FunctionAST*>, VarDeclNodeAST* >
    llvm::DenseMap<std::pair<llvm::StringRef, FunctionAST *>, VarDeclNodeAST *> VariableDecls;
    // < ExprAST of ReturnStmt, FunctionAST* >
    llvm::DenseMap<ExprAST *, FunctionAST *> ReturnExprs;
    // < VariableExprAST*, ASTNode*(VarDeclNodeAST* or FunctionAST*) >
    llvm::DenseMap<VariableExprAST *, ASTNode *> VariableRefs;
};

class SymbolTableBuilder: public RecursiveASTVisitor<SymbolTableBuilder> {
public:
    SymbolTableBuilder(): SymTab(), CurrentFunction(nullptr) {}

    void build(ProgramAST *AST) { visitProgram(AST); }

    SymbolTable &getSymbolTale() { return SymTab; }

    bool actBeforeVisitProgram(ProgramAST * Program) {
        for (auto *Function : Program->getFunctions())
            SymTab.addFunction(Function->getFuncName(), Function);
        return false;
    }

    bool actBeforeVisitFunction(FunctionAST *Function) {
        CurrentFunction = Function;
        return false;
    }

    void actAfterVisitVarDeclNode(VarDeclNodeAST *VarDeclNode) {
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
    FunctionAST *CurrentFunction;
};

}  // namespace remniw
