#pragma once

#include "frontend/RecursiveASTVisitor.h"
#include "frontend/Type.h"
#include "semantic/SymbolTable.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Casting.h"
#include <vector>
#include <set>

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

    // update type of ExprAST nodes with type analysis result
    void updateTypeForExprs();

    // visitor
    bool actBeforeVisitFunction(FunctionAST *Function) {
        CurrentFunction = Function;
        return false;
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

    void actAfterVisitNumberExpr(NumberExprAST *NumberExpr) {
        // Type constraint for BinaryExpr: // I: [[I]] = int
        Constraints.emplace_back(ASTNodeToType(NumberExpr), Type::getIntType(TypeCtx));

        // Add current NumberExpr to Exprs set
        Exprs.insert(NumberExpr);
    }

    void actAfterVisitBinaryExpr(BinaryExprAST *BinaryExpr) {
        // Type constraint for BinaryExpr:
        // E1 op E2: [[E1]] = [[E2]] = [[E1 op E2]] = int
        // E1 == E2: [[E1]] = [[E2]] ^ [[E1 == E2]] = int
        auto *IntTy = Type::getIntType(TypeCtx);
        Constraints.emplace_back(ASTNodeToType(BinaryExpr), IntTy);
        if (BinaryExpr->getOp() == BinaryExprAST::OpKind::Eq) {
            Constraints.emplace_back(ASTNodeToType(BinaryExpr->getLHS()),
                                     ASTNodeToType(BinaryExpr->getRHS()));
        } else {
            Constraints.emplace_back(ASTNodeToType(BinaryExpr->getLHS()), IntTy);
            Constraints.emplace_back(ASTNodeToType(BinaryExpr->getRHS()), IntTy);
        }

        // Add current BinaryExpr to Exprs set
        Exprs.insert(BinaryExpr);
    }

    void actAfterVisitInputExpr(InputExprAST *InputExpr) {
        // Type constraint for InputExpr: [[input]] = int
        Constraints.emplace_back(ASTNodeToType(InputExpr), Type::getIntType(TypeCtx));

        // Add current InputExpr to Exprs set
        Exprs.insert(InputExpr);
    }

    void actAfterVisitFunctionCallExpr(FunctionCallExprAST *FunctionCallExpr) {
        // Type constraint for FunctionCallExpr:
        // E(E1,...,En): [[E]] = ([[E1]],...,[[En]])->[[E(E1,...,En)]]
        std::vector<Type *> ArgTypes;
        for (auto *Arg : FunctionCallExpr->getArgs()) {
            ArgTypes.push_back(ASTNodeToType(Arg));
        }
        Constraints.emplace_back(
            ASTNodeToType(FunctionCallExpr->getCallee()),
            Type::getFunctionType(ArgTypes, ASTNodeToType(FunctionCallExpr)));

        // Add current FunctionCallExpr to Exprs set
        Exprs.insert(FunctionCallExpr);
    }

    void actAfterVisitRefExpr(RefExprAST *RefExpr) {
        // Type constraint for RefExprAST: &X: [[&X]] = &[[X]]
        Constraints.emplace_back(ASTNodeToType(RefExpr),
                                 ASTNodeToType(RefExpr->getVar())->getPointerTo());

        // Add current RefExpr to Exprs set
        Exprs.insert(RefExpr);
    }

    // TODO
    void actAfterVisitNullExpr(NullExprAST *NullExpr) {
        // Type constraint for NullExprAST: [[null]] = &α
        // Constraints.emplace_back(ASTNodeToType(&NullExpr),
        //                          std::make_shared<PointerType>(std::make_shared<AlphaType>(&NullExpr)));
    }

    void actAfterVisitDerefExpr(DerefExprAST *DerefExpr) {
        // Type constraint for DerefExpr: *E: [[E]] = &[[*E]]
        Constraints.emplace_back(ASTNodeToType(DerefExpr->getPtr()),
                                 ASTNodeToType(DerefExpr)->getPointerTo());

        // Add current DerefExpr to Exprs set
        Exprs.insert(DerefExpr);
    }

    void actAfterVisitArraySubscriptExpr(ArraySubscriptExprAST *ArraySubscriptExpr) {
        // Type constraint for ArraySubscriptExpr:
        // E[E1]: [[E1]] = int, [[E[E1]]] = [[E]]->getElementType()
        Constraints.emplace_back(ASTNodeToType(ArraySubscriptExpr->getSelector()),
                                 Type::getIntType(TypeCtx));
        // Note here, we decay arrayType to pointerType in type analysis
        Constraints.emplace_back(ASTNodeToType(ArraySubscriptExpr->getBase()),
                                 ASTNodeToType(ArraySubscriptExpr)->getPointerTo());

        // Add current ArraySubscriptExpr to Exprs set
        Exprs.insert(ArraySubscriptExpr);
    }

private:
    Type *ASTNodeToType(const ASTNode *Node) const;

    Type *getConcreteType(Type* Ty) const;

private:
    SymbolTable &SymTab;
    TypeContext &TypeCtx;
    std::unique_ptr<UnionFind> TheUnionFind;
    FunctionAST *CurrentFunction;
    std::vector<TypeConstraint> Constraints;
    std::set<ExprAST *> Exprs;
};

}  // namespace remniw
