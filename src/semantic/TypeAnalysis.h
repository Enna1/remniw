#pragma once

#include "RecursiveASTVisitor.h"
#include "SymbolTable.h"
#include "Type.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Casting.h"
#include <vector>

namespace remniw {

struct TypeConstraint {
    TypeConstraint(Type *LHS, Type *RHS): LHS(LHS), RHS(RHS) {}

    void print(llvm::raw_ostream &OS) const {
        LHS->print(OS);
        OS << " == ";
        RHS->print(OS);
        OS << "\n";
    }

    Type *LHS;
    Type *RHS;
};

class UnionFind {
public:
    void unionTypes(Type *Ty1, Type *Ty2) {
        auto Ty1r = find(Ty1);
        auto Ty2r = find(Ty2);
        if (Ty1r != Ty2r)
            Parent[Ty1r] = Ty2r;
    }

    void makeSet(Type *Ty) {
        if (Parent.find(Ty) == Parent.end())
            Parent[Ty] = Ty;
    }

    Type *find(Type *Ty) {
        makeSet(Ty);
        if (Parent[Ty] != Ty)
            Parent[Ty] = find(Parent[Ty]);
        return Parent[Ty];
    }

private:
    // Parents[x] = y means the parent of x is y
    llvm::DenseMap<Type *, Type *> Parent;
};

class TypeAnalysis: public RecursiveASTVisitor<TypeAnalysis> {
public:
    TypeAnalysis(SymbolTable &SymTab, TypeContext &TypeCtx):
        SymTab(SymTab), TypeCtx(TypeCtx),
        TheUnionFind(std::move(std::make_unique<UnionFind>())) {}

    bool unify(Type *Ty1, Type *Ty2);

    bool solve(ProgramAST *AST);

    std::vector<TypeConstraint> getConstraints() { return Constraints; }

    // visitor
    bool actBeforeVisitFunction(FunctionAST *Function) {
        CurrentFunction = Function;
        return false;
    }

    // I: [[I]] = int
    void actAfterVisitNumberExpr(NumberExprAST *NumberExpr) {
        Constraints.emplace_back(ASTNodeToType(NumberExpr), Type::getIntType(TypeCtx));
    }

    // E1 op E2: [[E1]] = [[E2]] = [[E1 op E2]] = int
    // E1 == E2: [[E1]] = [[E2]] ^ [[E1 == E2]] = int
    void actAfterVisitBinaryExpr(BinaryExprAST *BinaryExpr) {
        auto *IntTy = Type::getIntType(TypeCtx);
        Constraints.emplace_back(ASTNodeToType(BinaryExpr), IntTy);
        if (BinaryExpr->getOp() == BinaryExprAST::OpKind::Eq) {
            Constraints.emplace_back(ASTNodeToType(BinaryExpr->getLHS()),
                                     ASTNodeToType(BinaryExpr->getRHS()));
        } else {
            Constraints.emplace_back(ASTNodeToType(BinaryExpr->getLHS()), IntTy);
            Constraints.emplace_back(ASTNodeToType(BinaryExpr->getRHS()), IntTy);
        }
    }

    // input: [[input]] = int
    void actAfterVisitInputExpr(InputExprAST *InputExpr) {
        Constraints.emplace_back(ASTNodeToType(InputExpr), Type::getIntType(TypeCtx));
    }

    // X = E: [[X]] = [[E]]
    void actAfterVisitAssignmentStmt(AssignmentStmtAST *AssignmentStmt) {
        Constraints.emplace_back(ASTNodeToType(AssignmentStmt->getLHS()),
                                 ASTNodeToType(AssignmentStmt->getRHS()));
    }

    // output E: [[E]] = int
    void actAfterVisitOutputStmt(OutputStmtAST *OutputStmt) {
        Constraints.emplace_back(ASTNodeToType(OutputStmt->getExpr()),
                                 Type::getIntType(TypeCtx));
    }

    // if (E) S1 else S2: [[E]] = int
    void actAfterVisitIfStmt(IfStmtAST *IfStmt) {
        Constraints.emplace_back(ASTNodeToType(IfStmt->getCond()),
                                 Type::getIntType(TypeCtx));
    }

    // while (E) S: [[E]] = int
    void actAfterVisitWhileStmt(WhileStmtAST *WhileStmt) {
        Constraints.emplace_back(ASTNodeToType(WhileStmt->getCond()),
                                 Type::getIntType(TypeCtx));
    }

    // main(X1,...,Xn){ ...return E; }: [[X1]] = ...[[Xn]] = [[E]] = int
    // X(X1,...,Xn){ ...return E; }: [[X]] = ([[X1]],...,[[Xn]])->[[E]]
    void actAfterVisitFunction(FunctionAST *Function) {
        std::vector<Type *> ParamTypes;
        ReturnStmtAST *Ret = Function->getReturn();
        if (Function->getFuncName() == "main") {
            for (auto *Param : Function->getParamDecls()) {
                ParamTypes.push_back(ASTNodeToType(Param));
                Constraints.emplace_back(ASTNodeToType(Param), Type::getIntType(TypeCtx));
            }
            Constraints.emplace_back(ASTNodeToType(Ret->getExpr()),
                                     Type::getIntType(TypeCtx));
        } else {
            for (auto *Param : Function->getParamDecls()) {
                ParamTypes.push_back(ASTNodeToType(Param));
            }
        }
        Constraints.emplace_back(
            ASTNodeToType(Function),
            Type::getFunctionType(ParamTypes, ASTNodeToType(Ret->getExpr())));
    }

    // E(E1,...,En): [[E]] = ([[E1]],...,[[En]])->[[E(E1,...,En)]]
    void actAfterVisitFunctionCallExpr(FunctionCallExprAST *FunctionCallExpr) {
        std::vector<Type *> ArgTypes;
        for (auto *Arg : FunctionCallExpr->getArgs()) {
            ArgTypes.push_back(ASTNodeToType(Arg));
        }
        Constraints.emplace_back(
            ASTNodeToType(FunctionCallExpr->getCallee()),
            Type::getFunctionType(ArgTypes, ASTNodeToType(FunctionCallExpr)));
    }

    // alloc E: [[alloc E]] = &[[E]]
    void actAfterVisitAllocExpr(AllocExprAST *AllocExpr) {
        Constraints.emplace_back(ASTNodeToType(AllocExpr),
                                 ASTNodeToType(AllocExpr->getInit())->getPointerTo());
    }

    // &X: [[&X]] = &[[X]]
    void actAfterVisitRefExpr(RefExprAST *RefExpr) {
        Constraints.emplace_back(ASTNodeToType(RefExpr),
                                 ASTNodeToType(RefExpr->getVar())->getPointerTo());
    }

    // FIXME
    // null: [[null]] = &α
    void actAfterVisitNullExpr(NullExprAST *NullExpr) {
        // Constraints.emplace_back(ASTNodeToType(&NullExpr),
        //                          std::make_shared<PointerType>(std::make_shared<AlphaType>(&NullExpr)));
    }

    // *E: [[E]] = &[[*E]]
    void actAfterVisitDerefExpr(DerefExprAST *DerefExpr) {
        Constraints.emplace_back(ASTNodeToType(DerefExpr->getPtr()),
                                 ASTNodeToType(DerefExpr)->getPointerTo());
    }

    // E[E1]: [[E1]] = int, [[E[E1]]] = [[E]]->getElementType()
    void actAfterVisitArraySubscriptExpr(ArraySubscriptExprAST *ArraySubscriptExpr) {
        Constraints.emplace_back(ASTNodeToType(ArraySubscriptExpr->getSelector()),
                                 Type::getIntType(TypeCtx));
        // Note here, we decay arrayType to pointerType in type analysis
        Constraints.emplace_back(ASTNodeToType(ArraySubscriptExpr->getBase()),
                                 ASTNodeToType(ArraySubscriptExpr)->getPointerTo());
    }

private:
    Type *ASTNodeToType(const ASTNode *Node) const;

private:
    SymbolTable &SymTab;
    TypeContext &TypeCtx;
    std::unique_ptr<UnionFind> TheUnionFind;
    FunctionAST *CurrentFunction;
    std::vector<TypeConstraint> Constraints;
};

}  // namespace remniw
