#include "semantic/TypeAnalysis.h"
#include "frontend/AST.h"
#include "frontend/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"

namespace remniw {

Type *TypeAnalysis::ASTNodeToType(const ASTNode *Node) const {
    if (auto *DeclRefExpr = llvm::dyn_cast<DeclRefExprAST>(Node)) {
        return DeclRefExpr->getDecl()->getType();
    }

    if (auto *VariableDecl = llvm::dyn_cast<VarDeclAST>(Node)) {
        return VariableDecl->getType();
    }

    if (auto *Function = llvm::dyn_cast<FunctionDeclAST>(Node)) {
        return Function->getType();
    }

    return Type::getVarType(Node, TypeCtx);
}

bool TypeAnalysis::unify(Type *Ty1, Type *Ty2) {
    Type *Ty1r = TheUnionFind->find(Ty1);
    Type *Ty2r = TheUnionFind->find(Ty2);
    if (Ty1r != Ty2r) {
        if (llvm::isa<VarType>(Ty1r) && llvm::isa<VarType>(Ty2r)) {
            TheUnionFind->unionTypes(Ty1r, Ty2r);
        } else if (llvm::isa<VarType>(Ty1r) && !llvm::isa<VarType>(Ty2r)) {
            TheUnionFind->unionTypes(Ty1r, Ty2r);
        } else if (!llvm::isa<VarType>(Ty1r) && llvm::isa<VarType>(Ty2r)) {
            TheUnionFind->unionTypes(Ty2r, Ty1r);
        } else if (!llvm::isa<VarType>(Ty1r) && !llvm::isa<VarType>(Ty2r)) {
            /* Check if Ty1r and Ty2r are same type constructor */
            if (llvm::isa<IntType>(Ty1r) && llvm::isa<IntType>(Ty2r)) {
                TheUnionFind->unionTypes(Ty1r, Ty2r);
            } else if (llvm::isa<PointerType>(Ty1r) && llvm::isa<PointerType>(Ty2r)) {
                TheUnionFind->unionTypes(Ty1r, Ty2r);
                auto *PointerTy1 = llvm::dyn_cast<PointerType>(Ty1r);
                auto *PointerTy2 = llvm::dyn_cast<PointerType>(Ty2r);
                unify(PointerTy1->getPointeeType(), PointerTy2->getPointeeType());
            } else if (llvm::isa<ArrayType>(Ty1r) && llvm::isa<ArrayType>(Ty2r)) {
                TheUnionFind->unionTypes(Ty1r, Ty2r);
                auto *ArrayTy1 = llvm::dyn_cast<ArrayType>(Ty1r);
                auto *ArrayTy2 = llvm::dyn_cast<ArrayType>(Ty2r);
                if (ArrayTy1->getNumElements() != ArrayTy2->getNumElements())
                    return false;
                unify(ArrayTy1->getElementType(), ArrayTy2->getElementType());
            } else if (llvm::isa<PointerType>(Ty1r) && llvm::isa<ArrayType>(Ty2r)) {
                TheUnionFind->unionTypes(Ty1r, Ty2r);
                auto *PointerTy = llvm::dyn_cast<PointerType>(Ty1r);
                auto *ArrayTy = llvm::dyn_cast<ArrayType>(Ty2r);
                unify(PointerTy->getPointeeType(), ArrayTy->getElementType());
            } else if (llvm::isa<ArrayType>(Ty1r) && llvm::isa<PointerType>(Ty2r)) {
                TheUnionFind->unionTypes(Ty1r, Ty2r);
                auto *ArrayTy = llvm::dyn_cast<ArrayType>(Ty1r);
                auto *PointerTy = llvm::dyn_cast<PointerType>(Ty2r);
                unify(ArrayTy->getElementType(), PointerTy->getPointeeType());
            } else if (llvm::isa<FunctionType>(Ty1r) && llvm::isa<FunctionType>(Ty2r)) {
                auto *FunctionTy1 = llvm::dyn_cast<FunctionType>(Ty1r);
                auto *FunctionTy2 = llvm::dyn_cast<FunctionType>(Ty2r);
                auto ParamsTy1 = FunctionTy1->getParamTypes();
                auto ParamsTy2 = FunctionTy2->getParamTypes();
                if (ParamsTy1.size() != ParamsTy2.size())
                    return false;
                TheUnionFind->unionTypes(Ty1r, Ty2r);
                for (int i = 0; i < ParamsTy1.size(); ++i) {
                    unify(ParamsTy1[i], ParamsTy2[i]);
                }
                auto RetTy1 = FunctionTy1->getReturnType();
                auto RetTy2 = FunctionTy2->getReturnType();
                unify(RetTy1, RetTy2);
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

bool TypeAnalysis::solve(ProgramAST *AST) {
    visitProgram(AST);

    for (TypeConstraint &Constraint : Constraints) {
        if (!unify(Constraint.LHS, Constraint.RHS)) {
            llvm::errs() << "TypeAnalysis failed!\n";
            Constraint.print(llvm::errs());
            return false;
        }
    }

    return true;
}

}  // namespace remniw
