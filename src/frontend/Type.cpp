#include "Type.h"
#include "llvm/Support/Casting.h"

namespace remniw {

void Type::print(llvm::raw_ostream &OS) const {
    switch (getTypeKind()) {
    case Type::TK_VARTYPE: {
        OS << "[[VarType@" << llvm::cast<VarType>(this)->getASTNode() << "]]";
        return;
    }
    case Type::TK_INTTYPE: {
        OS << "IntType";
        return;
    }
    case Type::TK_POINTERTYPE: {
        llvm::cast<PointerType>(this)->getPointeeType()->print(OS << "*");
        return;
    }
    case Type::TK_ARRAYTYPE: {
        auto *ArrayTy = llvm::cast<ArrayType>(this);
        OS << "[" << ArrayTy->getNumElements() << "]";
        ArrayTy->getElementType()->print(OS);
        return;
    }
    case Type::TK_FUNCTIONTYPE: {
        auto *FTy = llvm::cast<FunctionType>(this);
        OS << "(";
        for (auto *ParamTy : FTy->getParamTypes()) {
            ParamTy->print(OS);
            OS << ",";
        }
        OS << ")->";
        FTy->getReturnType()->print(OS);
        return;
    }
    }
    llvm_unreachable("Invalid TypeKind");
}

IntType *Type::getIntType(TypeContext &C) {
    return &C.IntTy;
}

PointerType *Type::getIntPtrType(TypeContext &C) {
    return getIntType(C)->getPointerTo();
}

ArrayType *Type::getArrayType(Type *ElementType, uint64_t NumElements) {
    return ArrayType::get(ElementType, NumElements);
}

FunctionType *Type::getFunctionType(llvm::ArrayRef<Type *> Params, Type *ReturnType) {
    return FunctionType::get(Params, ReturnType);
}

VarType *Type::getVarType(const ASTNode *DeclNode, TypeContext &C) {
    return VarType::get(DeclNode, C);
}

PointerType *Type::getPointerTo() {
    return PointerType::get(const_cast<Type *>(this));
}

VarType *VarType::get(const ASTNode *DeclNode, TypeContext &C) {
    const VarTypeKeyInfo::KeyTy Key(DeclNode);
    VarType *VarTy;
    auto Insertion = C.VarTypes.insert_as(nullptr, Key);
    if (Insertion.second) {
        VarTy = new VarType(DeclNode, C);
        *Insertion.first = VarTy;
    } else {
        VarTy = *Insertion.first;
    }
    return VarTy;
}

IntType *IntType::get(TypeContext &C) {
    return Type::getIntType(C);
}

PointerType *PointerType::get(Type *EltTy) {
    TypeContext &C = EltTy->getContext();

    if (C.PointerTypes.count(EltTy))
        return C.PointerTypes[EltTy];

    PointerType *Entry = new PointerType(EltTy);
    C.PointerTypes[EltTy] = Entry;
    return Entry;
}

ArrayType *ArrayType::get(Type *EltTy, uint64_t NumEl) {
    TypeContext &C = EltTy->getContext();

    ArrayType *&Entry = C.ArrayTypes[std::make_pair(EltTy, NumEl)];

    if (!Entry)
        Entry = new ArrayType(EltTy, NumEl);
    return Entry;
}

FunctionType *FunctionType::get(llvm::ArrayRef<Type *> Params, Type *ReturnType) {
    TypeContext &C = ReturnType->getContext();
    const FunctionTypeKeyInfo::KeyTy Key(Params, ReturnType);
    FunctionType *FT;
    auto Insertion = C.FunctionTypes.insert_as(nullptr, Key);
    if (Insertion.second) {
        FT = new FunctionType(Params, ReturnType);
        *Insertion.first = FT;
    } else {
        FT = *Insertion.first;
    }
    return FT;
}

}  // namespace remniw

namespace llvm {

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const remniw::Type &Ty) {
    Ty.print(OS);
    return OS;
}

}  // namespace llvm
