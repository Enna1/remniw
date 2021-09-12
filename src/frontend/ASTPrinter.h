#pragma once

#include "RecursiveASTVisitor.h"
#include "llvm/Support/raw_ostream.h"

namespace remniw {

class ASTPrinter: public RecursiveASTVisitor<ASTPrinter> {
private:
    unsigned Ind;
    llvm::raw_ostream &Out;

public:
    ASTPrinter(llvm::raw_ostream &Out): RecursiveASTVisitor(), Ind(0), Out(Out) {}

    void print(ProgramAST *AST) { visitProgram(AST); }

    bool actBeforeVisitVarDeclNode(VarDeclNodeAST *Node) {
        Out.indent(Ind) << "VarDeclNode " << Node << " '" << Node->getName() << "' "
                        << *Node->getType() << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        return false;
    }

    bool actBeforeVisitNumberExpr(NumberExprAST *Node) {
        Out.indent(Ind) << "NumberExpr " << Node << " '" << Node->getValue() << "' "
                        << (Node->IsLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << ">\n";
        return false;
    }

    bool actBeforeVisitVariableExpr(VariableExprAST *Node) {
        Out.indent(Ind) << "VariableExpr " << Node << " '" << Node->getName() << "' "
                        << (Node->IsLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << ">\n";
        ;
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
                        << (Node->IsLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << ">\n";
        return false;
    }

    bool actBeforeVisitAllocExpr(AllocExprAST *Node) {
        Out.indent(Ind) << "AllocExpr " << Node << ", "
                        << (Node->IsLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << ">\n";
        return false;
    }

    bool actBeforeVisitRefExpr(RefExprAST *Node) {
        Out.indent(Ind) << "RefExpr " << Node << ", "
                        << (Node->IsLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitRefExpr(RefExprAST *) { Ind -= 1; }

    bool actBeforeVisitDerefExpr(DerefExprAST *Node) {
        Out.indent(Ind) << "DerefExpr " << Node << ", "
                        << (Node->IsLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitDerefExpr(DerefExprAST *) { Ind -= 1; }

    bool actBeforeVisitInputExpr(InputExprAST *Node) {
        Out.indent(Ind) << "InputExpr " << Node << ", "
                        << (Node->IsLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << ">\n";
        return false;
    }

    bool actBeforeVisitBinaryExpr(BinaryExprAST *Node) {
        Out.indent(Ind) << "BinaryExpr " << Node << " '" << Node->getOpString() << "' "
                        << (Node->IsLValue() ? "lvalue" : "rvalue") << " <"
                        << Node->getLine() << ':' << Node->getCol() << ">\n";
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

    bool actBeforeVisitBasicAssignmentStmt(BasicAssignmentStmtAST *Node) {
        Out.indent(Ind) << "BasicAssignmentStmt " << Node << " <" << Node->getLine()
                        << ':' << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitBasicAssignmentStmt(BasicAssignmentStmtAST *) { Ind -= 1; }

    bool actBeforeVisitDerefAssignmentStmt(DerefAssignmentStmtAST *Node) {
        Out.indent(Ind) << "DerefAssignmentStmt " << Node << " <" << Node->getLine()
                        << ':' << Node->getCol() << ">\n";
        Ind += 1;
        return false;
    }

    void actAfterVisitDerefAssignmentStmt(DerefAssignmentStmtAST *) { Ind -= 1; }

    void visitFunction(FunctionAST *Node) {
        Out.indent(Ind) << "Function " << Node << " '" << Node->getFuncName() << "' "
                        << *Node->getType() << " <" << Node->getLine() << ':'
                        << Node->getCol() << ">\n";
        Out.indent(Ind + 1) << "ParamDecls:\n";
        Ind += 2;
        for (auto *ParmDecl : Node->getParamDecls())
            visitVarDeclNode(ParmDecl);
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
