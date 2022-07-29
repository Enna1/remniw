#pragma once

#include "AST.h"

namespace remniw {

template<typename Derived>
class RecursiveASTVisitor {
public:
    // Return a reference to the derived class.
    Derived &getDerived() { return *static_cast<Derived *>(this); }

    void visitExpr(ExprAST *);
    void visitStmt(StmtAST *);

    bool actBeforeVisitVarDeclNode(VarDeclNodeAST *) { return false; }
    void actAfterVisitVarDeclNode(VarDeclNodeAST *) {}
    void visitVarDeclNode(VarDeclNodeAST *);

    bool actBeforeVisitNumberExpr(NumberExprAST *) { return false; }
    void actAfterVisitNumberExpr(NumberExprAST *) {}
    void visitNumberExpr(NumberExprAST *);

    bool actBeforeVisitVariableExpr(VariableExprAST *) { return false; }
    void actAfterVisitVariableExpr(VariableExprAST *) {}
    void visitVariableExpr(VariableExprAST *);

    bool actBeforeVisitFunctionCallExpr(FunctionCallExprAST *) { return false; }
    void actAfterVisitFunctionCallExpr(FunctionCallExprAST *) {}
    void visitFunctionCallExpr(FunctionCallExprAST *);

    bool actBeforeVisitNullExpr(NullExprAST *) { return false; }
    void actAfterVisitNullExpr(NullExprAST *) {}
    void visitNullExpr(NullExprAST *);

    bool actBeforeVisitAllocExpr(AllocExprAST *) { return false; }
    void actAfterVisitAllocExpr(AllocExprAST *) {}
    void visitAllocExpr(AllocExprAST *);

    bool actBeforeVisitSizeofExpr(SizeofExprAST *) { return false; }
    void actAfterVisitSizeofExpr(SizeofExprAST *) {}
    void visitSizeofExpr(SizeofExprAST *);

    bool actBeforeVisitRefExpr(RefExprAST *) { return false; }
    void actAfterVisitRefExpr(RefExprAST *) {}
    void visitRefExpr(RefExprAST *);

    bool actBeforeVisitDerefExpr(DerefExprAST *) { return false; }
    void actAfterVisitDerefExpr(DerefExprAST *) {}
    void visitDerefExpr(DerefExprAST *);

    bool actBeforeVisitArraySubscriptExpr(ArraySubscriptExprAST *) { return false; }
    void actAfterVisitArraySubscriptExpr(ArraySubscriptExprAST *) {}
    void visitArraySubscriptExpr(ArraySubscriptExprAST *);

    bool actBeforeVisitInputExpr(InputExprAST *) { return false; }
    void actAfterVisitInputExpr(InputExprAST *) {}
    void visitInputExpr(InputExprAST *);

    bool actBeforeVisitBinaryExpr(BinaryExprAST *) { return false; }
    void actAfterVisitBinaryExpr(BinaryExprAST *) {}
    void visitBinaryExpr(BinaryExprAST *);

    bool actBeforeVisitLocalVarDeclStmt(LocalVarDeclStmtAST *) { return false; }
    void actAfterVisitLocalVarDeclStmt(LocalVarDeclStmtAST *) {}
    void visitLocalVarDeclStmt(LocalVarDeclStmtAST *);

    bool actBeforeVisitEmptyStmt(EmptyStmtAST *) { return false; }
    void actAfterVisitEmptyStmt(EmptyStmtAST *) {}
    void visitEmptyStmt(EmptyStmtAST *);

    bool actBeforeVisitOutputStmt(OutputStmtAST *) { return false; }
    void actAfterVisitOutputStmt(OutputStmtAST *) {}
    void visitOutputStmt(OutputStmtAST *);

    bool actBeforeVisitDeallocStmt(DeallocStmtAST *) { return false; }
    void actAfterVisitDeallocStmt(DeallocStmtAST *) {}
    void visitDeallocStmt(DeallocStmtAST *);

    bool actBeforeVisitBlockStmt(BlockStmtAST *) { return false; }
    void actAfterVisitBlockStmt(BlockStmtAST *) {}
    void visitBlockStmt(BlockStmtAST *);

    bool actBeforeVisitReturnStmt(ReturnStmtAST *) { return false; }
    void actAfterVisitReturnStmt(ReturnStmtAST *) {}
    void visitReturnStmt(ReturnStmtAST *);

    bool actBeforeVisitIfStmt(IfStmtAST *) { return false; }
    void actAfterVisitIfStmt(IfStmtAST *) {}
    void visitIfStmt(IfStmtAST *);

    bool actBeforeVisitWhileStmt(WhileStmtAST *) { return false; }
    void actAfterVisitWhileStmt(WhileStmtAST *) {}
    void visitWhileStmt(WhileStmtAST *);

    bool actBeforeVisitAssignmentStmt(AssignmentStmtAST *) { return false; }
    void actAfterVisitAssignmentStmt(AssignmentStmtAST *) {}
    void visitAssignmentStmt(AssignmentStmtAST *);

    bool actBeforeVisitFunction(FunctionAST *) { return false; }
    void actAfterVisitFunction(FunctionAST *) {}
    void visitFunction(FunctionAST *);

    bool actBeforeVisitProgram(ProgramAST *) { return false; }
    void actAfterVisitProgram(ProgramAST *) {}
    void visitProgram(ProgramAST *);
};

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitExpr(ExprAST *Expr) {
    switch (Expr->getKind()) {
    case ASTNode::NumberExpr:
        getDerived().visitNumberExpr(static_cast<NumberExprAST *>(Expr));
        break;
    case ASTNode::VariableExpr:
        getDerived().visitVariableExpr(static_cast<VariableExprAST *>(Expr));
        break;
    case ASTNode::FunctionCallExpr:
        getDerived().visitFunctionCallExpr(static_cast<FunctionCallExprAST *>(Expr));
        break;
    case ASTNode::NullExpr:
        getDerived().visitNullExpr(static_cast<NullExprAST *>(Expr));
        break;
    case ASTNode::AllocExpr:
        getDerived().visitAllocExpr(static_cast<AllocExprAST *>(Expr));
        break;
    case ASTNode::SizeofExpr:
        getDerived().visitSizeofExpr(static_cast<SizeofExprAST *>(Expr));
        break;
    case ASTNode::RefExpr:
        getDerived().visitRefExpr(static_cast<RefExprAST *>(Expr));
        break;
    case ASTNode::DerefExpr:
        getDerived().visitDerefExpr(static_cast<DerefExprAST *>(Expr));
        break;
    case ASTNode::ArraySubscriptExpr:
        getDerived().visitArraySubscriptExpr(static_cast<ArraySubscriptExprAST *>(Expr));
        break;
    case ASTNode::InputExpr:
        getDerived().visitInputExpr(static_cast<InputExprAST *>(Expr));
        break;
    case ASTNode::BinaryExpr:
        getDerived().visitBinaryExpr(static_cast<BinaryExprAST *>(Expr));
        break;
    default: llvm_unreachable("Invalid expr");
    }
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitStmt(StmtAST *Stmt) {
    switch (Stmt->getKind()) {
    case ASTNode::LocalVarDeclStmt:
        getDerived().visitLocalVarDeclStmt(static_cast<LocalVarDeclStmtAST *>(Stmt));
        break;
    case ASTNode::EmptyStmt:
        getDerived().visitEmptyStmt(static_cast<EmptyStmtAST *>(Stmt));
        break;
    case ASTNode::OutputStmt:
        getDerived().visitOutputStmt(static_cast<OutputStmtAST *>(Stmt));
        break;
    case ASTNode::DeallocStmt:
        getDerived().visitDeallocStmt(static_cast<DeallocStmtAST *>(Stmt));
        break;
    case ASTNode::BlockStmt:
        getDerived().visitBlockStmt(static_cast<BlockStmtAST *>(Stmt));
        break;
    case ASTNode::ReturnStmt:
        getDerived().visitReturnStmt(static_cast<ReturnStmtAST *>(Stmt));
        break;
    case ASTNode::IfStmt: getDerived().visitIfStmt(static_cast<IfStmtAST *>(Stmt)); break;
    case ASTNode::WhileStmt:
        getDerived().visitWhileStmt(static_cast<WhileStmtAST *>(Stmt));
        break;
    case ASTNode::AssignmentStmt:
        getDerived().visitAssignmentStmt(static_cast<AssignmentStmtAST *>(Stmt));
        break;
    default: llvm_unreachable("Invalid stmt");
    }
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitVarDeclNode(VarDeclNodeAST *VarDeclNode) {
    if (getDerived().actBeforeVisitVarDeclNode(VarDeclNode))
        return;

    getDerived().actAfterVisitVarDeclNode(VarDeclNode);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitNumberExpr(NumberExprAST *NumberExpr) {
    if (getDerived().actBeforeVisitNumberExpr(NumberExpr))
        return;

    getDerived().actAfterVisitNumberExpr(NumberExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitVariableExpr(VariableExprAST *VariableExpr) {
    if (getDerived().actBeforeVisitVariableExpr(VariableExpr))
        return;

    getDerived().actAfterVisitVariableExpr(VariableExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitFunctionCallExpr(
    FunctionCallExprAST *FunctionCallExpr) {
    if (getDerived().actBeforeVisitFunctionCallExpr(FunctionCallExpr))
        return;

    for (auto *Arg : FunctionCallExpr->getArgs())
        visitExpr(Arg);

    getDerived().actAfterVisitFunctionCallExpr(FunctionCallExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitNullExpr(NullExprAST *NullExpr) {
    if (getDerived().actBeforeVisitNullExpr(NullExpr))
        return;

    getDerived().actAfterVisitNullExpr(NullExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitAllocExpr(AllocExprAST *AllocExpr) {
    if (getDerived().actBeforeVisitAllocExpr(AllocExpr))
        return;

    visitExpr(AllocExpr->getInit());

    getDerived().actAfterVisitAllocExpr(AllocExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitSizeofExpr(SizeofExprAST *SizeofExpr) {
    if (getDerived().actBeforeVisitSizeofExpr(SizeofExpr))
        return;

    getDerived().actAfterVisitSizeofExpr(SizeofExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitRefExpr(RefExprAST *RefExpr) {
    if (getDerived().actBeforeVisitRefExpr(RefExpr))
        return;

    visitExpr(RefExpr->getVar());

    actAfterVisitRefExpr(RefExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitDerefExpr(DerefExprAST *DerefExpr) {
    if (getDerived().actBeforeVisitDerefExpr(DerefExpr))
        return;

    visitExpr(DerefExpr->getPtr());

    getDerived().actAfterVisitDerefExpr(DerefExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitArraySubscriptExpr(
    ArraySubscriptExprAST *ArraySubscriptExpr) {
    if (getDerived().actBeforeVisitArraySubscriptExpr(ArraySubscriptExpr))
        return;

    visitExpr(ArraySubscriptExpr->getBase());
    visitExpr(ArraySubscriptExpr->getSelector());

    getDerived().actAfterVisitArraySubscriptExpr(ArraySubscriptExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitInputExpr(InputExprAST *InputExpr) {
    if (getDerived().actBeforeVisitInputExpr(InputExpr))
        return;

    getDerived().actAfterVisitInputExpr(InputExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitBinaryExpr(BinaryExprAST *BinaryExpr) {
    if (getDerived().actBeforeVisitBinaryExpr(BinaryExpr))
        return;

    visitExpr(BinaryExpr->getLHS());
    visitExpr(BinaryExpr->getRHS());

    getDerived().actAfterVisitBinaryExpr(BinaryExpr);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitLocalVarDeclStmt(
    LocalVarDeclStmtAST *LocalVarDeclStmt) {
    if (getDerived().actBeforeVisitLocalVarDeclStmt(LocalVarDeclStmt))
        return;

    for (auto *VarDeclNode : LocalVarDeclStmt->getVars())
        visitVarDeclNode(VarDeclNode);

    actAfterVisitLocalVarDeclStmt(LocalVarDeclStmt);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitEmptyStmt(EmptyStmtAST *EmptyStmt) {
    if (getDerived().actBeforeVisitEmptyStmt(EmptyStmt))
        return;

    getDerived().actAfterVisitEmptyStmt(EmptyStmt);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitOutputStmt(OutputStmtAST *OutputStmt) {
    if (getDerived().actBeforeVisitOutputStmt(OutputStmt))
        return;

    visitExpr(OutputStmt->getExpr());

    getDerived().actAfterVisitOutputStmt(OutputStmt);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitDeallocStmt(DeallocStmtAST *DeallocStmt) {
    if (getDerived().actBeforeVisitDeallocStmt(DeallocStmt))
        return;

    visitExpr(DeallocStmt->getExpr());

    getDerived().actAfterVisitDeallocStmt(DeallocStmt);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitBlockStmt(BlockStmtAST *BlockStmt) {
    if (getDerived().actBeforeVisitBlockStmt(BlockStmt))
        return;

    for (auto *Stmt : BlockStmt->getStmts())
        visitStmt(Stmt);

    getDerived().actAfterVisitBlockStmt(BlockStmt);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitReturnStmt(ReturnStmtAST *ReturnStmt) {
    if (getDerived().actBeforeVisitReturnStmt(ReturnStmt))
        return;

    visitExpr(ReturnStmt->getExpr());

    getDerived().actAfterVisitReturnStmt(ReturnStmt);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitIfStmt(IfStmtAST *IfStmt) {
    if (getDerived().actBeforeVisitIfStmt(IfStmt))
        return;

    visitExpr(IfStmt->getCond());
    visitStmt(IfStmt->getThen());
    if (IfStmt->getElse())
        visitStmt(IfStmt->getElse());

    getDerived().actAfterVisitIfStmt(IfStmt);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitWhileStmt(WhileStmtAST *WhileStmt) {
    if (getDerived().actBeforeVisitWhileStmt(WhileStmt))
        return;

    visitExpr(WhileStmt->getCond());
    visitStmt(WhileStmt->getBody());

    getDerived().actAfterVisitWhileStmt(WhileStmt);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitAssignmentStmt(
    AssignmentStmtAST *AssignmentStmt) {
    if (getDerived().actBeforeVisitAssignmentStmt(AssignmentStmt))
        return;

    visitExpr(AssignmentStmt->getLHS());
    visitExpr(AssignmentStmt->getRHS());

    getDerived().actAfterVisitAssignmentStmt(AssignmentStmt);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitFunction(FunctionAST *Function) {
    if (getDerived().actBeforeVisitFunction(Function))
        return;

    for (auto *VarDeclNode : Function->getParamDecls())
        visitVarDeclNode(VarDeclNode);
    visitLocalVarDeclStmt(Function->getLocalVarDecls());
    for (auto *Stmt : Function->getBody())
        visitStmt(Stmt);
    visitReturnStmt(Function->getReturn());

    getDerived().actAfterVisitFunction(Function);
}

template<typename Derived>
void RecursiveASTVisitor<Derived>::visitProgram(ProgramAST *Program) {
    if (getDerived().actBeforeVisitProgram(Program))
        return;

    for (auto *Function : Program->getFunctions())
        getDerived().visitFunction(Function);

    getDerived().actAfterVisitProgram(Program);
}

}  // namespace remniw
