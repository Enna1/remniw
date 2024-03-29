#pragma once

#include "frontend/RecursiveASTVisitor.h"
#include "llvm/Support/raw_ostream.h"

namespace remniw {

class ASTPrinter: public RecursiveASTVisitor<ASTPrinter> {
private:
    unsigned Ind;
    llvm::raw_ostream &Out;

public:
    ASTPrinter(llvm::raw_ostream &Out): RecursiveASTVisitor(), Ind(0), Out(Out) {}

    void print(ProgramAST *AST) { visitProgram(AST); }

    bool actBeforeVisitVarDeclNode(VarDeclAST *Node) {
        Out.indent(Ind) << "VarDecl " << Node << " '" << Node->getName() << "' "
                        << *Node->getType() << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        return false;
    }

    bool actBeforeVisitNumberExpr(NumberExprAST *Node) {
        Out.indent(Ind) << "NumberExpr " << Node << " '" << Node->getValue() << "' "
                        << (Node->isLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << "> ";
        if (auto *Ty = Node->getType())
            Out << *Ty;
        Out << "\n";
        return false;
    }

    bool actBeforeVisitDeclRefExpr(DeclRefExprAST *Node) {
        Out.indent(Ind) << "DeclRefExpr " << Node << " '" << Node->getName() << "' "
                        << (Node->isLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << "> ";
        if (auto *Ty = Node->getType())
            Out << *Ty;
        Out << "\n";
        return false;
    }

    void visitFunctionCallExpr(FunctionCallExprAST *Node) {
        Out.indent(Ind) << "FunctionCallExpr " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Out.indent(Ind + 1) << "Callee:\n";
        Ind += 2;
        visitExpr(Node->getCallee());
        Ind -= 2;
        Out.indent(Ind + 1) << "Args:\n";
        Ind += 2;
        for (auto *Arg : Node->getArgs())
            visitExpr(Arg);
        Ind -= 2;
    }

    bool actBeforeVisitNullExpr(NullExprAST *Node) {
        Out.indent(Ind) << "NullExpr " << Node << ", "
                        << (Node->isLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << ">\n";
        return false;
    }

    bool actBeforeVisitAddrOfExpr(AddrOfExprAST *Node) {
        Out.indent(Ind) << "AddrOfExpr " << Node << ", "
                        << (Node->isLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << "> ";
        if (auto *Ty = Node->getType())
            Out << *Ty;
        Out << "\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitAddrOfExpr(AddrOfExprAST *) { Ind -= 1; }

    bool actBeforeVisitDerefExpr(DerefExprAST *Node) {
        Out.indent(Ind) << "DerefExpr " << Node << ", "
                        << (Node->isLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << "> ";
        if (auto *Ty = Node->getType())
            Out << *Ty;
        Out << "\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitDerefExpr(DerefExprAST *) { Ind -= 1; }

    bool actBeforeVisitArraySubscriptExpr(ArraySubscriptExprAST *Node) {
        Out.indent(Ind) << "ArraySubscriptExpr " << Node << ", "
                        << (Node->isLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << "> ";
        if (auto *Ty = Node->getType())
            Out << *Ty;
        Out << "\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitArraySubscriptExpr(ArraySubscriptExprAST *) { Ind -= 1; }

    bool actBeforeVisitInputExpr(InputExprAST *Node) {
        Out.indent(Ind) << "InputExpr " << Node << ", "
                        << (Node->isLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << "> ";
        if (auto *Ty = Node->getType())
            Out << *Ty;
        Out << "\n";
        return false;
    }

    bool actBeforeVisitBinaryExpr(BinaryExprAST *Node) {
        Out.indent(Ind) << "BinaryExpr " << Node << " '" << Node->getOpString() << "' "
                        << (Node->isLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << "> ";
        if (auto *Ty = Node->getType())
            Out << *Ty;
        Out << "\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitBinaryExpr(BinaryExprAST *) { Ind -= 1; }

    bool actBeforeVisitLocalVarDeclStmt(LocalVarDeclStmtAST *Node) {
        Out.indent(Ind) << "LocalVarDeclStmt " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitLocalVarDeclStmt(LocalVarDeclStmtAST *) { Ind -= 1; }

    bool actBeforeVisitEmptyStmt(EmptyStmtAST *Node) {
        Out.indent(Ind) << "EmptyStmt " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        return false;
    }

    bool actBeforeVisitOutputStmt(OutputStmtAST *Node) {
        Out.indent(Ind) << "OutputStmt " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitOutputStmt(OutputStmtAST *) { Ind -= 1; }

    bool actBeforeVisitAllocStmt(AllocStmtAST *Node) {
        Out.indent(Ind) << "AllocStmt " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitAllocStmt(AllocStmtAST *) { Ind -= 1; }

    bool actBeforeVisitDeallocStmt(DeallocStmtAST *Node) {
        Out.indent(Ind) << "DeallocStmt " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitDeallocStmt(DeallocStmtAST *) { Ind -= 1; }

    bool actBeforeVisitBlockStmtAST(BlockStmtAST *Node) {
        Out.indent(Ind) << "BlockStmt " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitBlockStmtAST(BlockStmtAST *) { Ind -= 1; }

    bool actBeforeVisitReturnStmt(ReturnStmtAST *Node) {
        Out.indent(Ind) << "ReturnStmt " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitReturnStmt(ReturnStmtAST *) { Ind -= 1; }

    void visitIfStmt(IfStmtAST *Node) {
        Out.indent(Ind) << "IfStmt " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Out.indent(Ind + 1) << "Cond:\n";
        Ind += 2;
        visitExpr(Node->getCond());
        Ind -= 2;
        Out.indent(Ind + 1) << "Then:\n";
        Ind += 2;
        visitStmt(Node->getThen());
        Ind -= 2;
        if (Node->getElse()) {
            Out.indent(Ind + 1) << "Else:\n";
            Ind += 2;
            visitStmt(Node->getElse());
            Ind -= 2;
        }
    }

    void visitWhileStmt(WhileStmtAST *Node) {
        Out.indent(Ind) << "WhileStmt" << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Out.indent(Ind + 1) << "Cond:\n";
        Ind += 2;
        visitExpr(Node->getCond());
        Ind -= 2;
        Out.indent(Ind + 1) << "Body:\n";
        Ind += 2;
        visitStmt(Node->getBody());
        Ind -= 2;
    }

    bool actBeforeVisitAssignmentStmt(AssignmentStmtAST *Node) {
        Out.indent(Ind) << "AssignmentStmt " << Node << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitAssignmentStmt(AssignmentStmtAST *) { Ind -= 1; }

    void visitFunction(FunctionDeclAST *Node) {
        Out.indent(Ind) << "Function " << Node << " '" << Node->getName() << "' "
                        << *Node->getType() << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Out.indent(Ind + 1) << "ParamDecls:\n";
        Ind += 2;
        for (auto *ParmDecl : Node->getParamDecls())
            visitVarDecl(ParmDecl);
        Ind -= 2;
        Out.indent(Ind + 1) << "LocalVarDecls:\n";
        Ind += 2;
        visitLocalVarDeclStmt(Node->getLocalVarDecls());
        Ind -= 2;
        Out.indent(Ind + 1) << "Body:\n";
        Ind += 2;
        for (auto *Stmt : Node->getBody())
            visitStmt(Stmt);
        Ind -= 2;
        Out.indent(Ind + 1) << "Return:\n";
        Ind += 2;
        visitReturnStmt(Node->getReturn());
        Ind -= 2;
        Out.indent(Ind) << "\n";
    }

private:
    static std::ostream &indent(std::ostream &O, int Size) {
        return O << std::string(Size, ' ');
    }
};

}  // namespace remniw
