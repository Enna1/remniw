#pragma once

#include "AsmOperand.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/IR/SymbolTableListTraits.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdint>
#include <memory>

namespace remniw {
class AsmFunction;
class AsmInstruction;
};  // namespace remniw

namespace llvm {

template<>
struct ilist_traits<remniw::AsmInstruction> {
private:
    friend class remniw::AsmFunction;

    remniw::AsmFunction *Parent;  // Set by the owning AsmFunction.

    using instr_iterator = simple_ilist<remniw::AsmInstruction>::iterator;

public:
    void addNodeToList(remniw::AsmInstruction *I);
    void removeNodeFromList(remniw::AsmInstruction *I);
    void transferNodesFromList(ilist_traits &FromList, instr_iterator First,
                               instr_iterator Last);
    void deleteNode(remniw::AsmInstruction *I);
};

}  // namespace llvm

namespace remniw {

class AsmInstruction: public llvm::ilist_node_with_parent<AsmInstruction, AsmFunction> {
public:
    enum KindTy
    {
        Mov,
        Lea,
        Cmp,
        Jmp,
        Add,
        Sub,
        Imul,
        Idiv,
        Cqto,
        Call,
        Xor,
        Push,
        Pop,
        Ret,
        Label,
    };

    AsmInstruction(KindTy Kind, AsmFunction *InsertAtEnd);

    AsmInstruction(KindTy Kind, std::unique_ptr<AsmOperand> Dst,
                   AsmFunction *InsertAtEnd);

    AsmInstruction(KindTy Kind, std::unique_ptr<AsmOperand> Src,
                   std::unique_ptr<AsmOperand> Dst, AsmFunction *InsertAtEnd);

    AsmInstruction(KindTy Kind, AsmInstruction *InsertBefore);

    AsmInstruction(KindTy Kind, std::unique_ptr<AsmOperand> Dst,
                   AsmInstruction *InsertBefore);

    AsmInstruction(KindTy Kind, std::unique_ptr<AsmOperand> Src,
                   std::unique_ptr<AsmOperand> Dst, AsmInstruction *InsertBefore);

    inline const AsmFunction *getParent() const { return Parent; }

    inline AsmFunction *getParent() { return Parent; }

    void setParent(AsmFunction *P) { Parent = P; }

    void removeFromParent();

    size_t getNumOperands() { return Operands.size(); }

    AsmOperand *getOperand(unsigned i) const { return Operands[i].get(); }

    void setOperand(unsigned i, std::unique_ptr<AsmOperand> Op) {
        Operands[i] = std::move(Op);
    }

    KindTy getInstKind() const { return Kind; }

    void deleteValue();

    void print(llvm::raw_ostream &OS) const;

private:
    KindTy Kind;
    llvm::SmallVector<std::unique_ptr<AsmOperand>, 2> Operands;
    AsmFunction *Parent;
};

class AsmMovInst: public AsmInstruction {
private:
    AsmMovInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Mov, std::move(Src), std::move(Dst), InsertAtEnd) {
    }

    AsmMovInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Mov, std::move(Src), std::move(Dst),
                       InsertBefore) {}

public:
    static AsmMovInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst, AsmFunction *InsertAtEnd) {
        return new AsmMovInst(std::move(Src), std::move(Dst), InsertAtEnd);
    }

    static AsmMovInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst,
                              AsmInstruction *InsertBefore) {
        return new AsmMovInst(std::move(Src), std::move(Dst), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Mov;
    }
};

class AsmLeaInst: public AsmInstruction {
private:
    AsmLeaInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Lea, std::move(Src), std::move(Dst), InsertAtEnd) {
    }

    AsmLeaInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Lea, std::move(Src), std::move(Dst),
                       InsertBefore) {}

public:
    static AsmLeaInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst, AsmFunction *InsertAtEnd) {
        return new AsmLeaInst(std::move(Src), std::move(Dst), InsertAtEnd);
    }

    static AsmLeaInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst,
                              AsmInstruction *InsertBefore) {
        return new AsmLeaInst(std::move(Src), std::move(Dst), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Lea;
    }
};

class AsmCmpInst: public AsmInstruction {
private:
    AsmCmpInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Cmp, std::move(Src), std::move(Dst), InsertAtEnd) {
    }

    AsmCmpInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Cmp, std::move(Src), std::move(Dst),
                       InsertBefore) {}

public:
    static AsmCmpInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst, AsmFunction *InsertAtEnd) {
        return new AsmCmpInst(std::move(Src), std::move(Dst), InsertAtEnd);
    }

    static AsmCmpInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst,
                              AsmInstruction *InsertBefore) {
        return new AsmCmpInst(std::move(Src), std::move(Dst), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Cmp;
    }
};

class AsmJmpInst: public AsmInstruction {
public:
    enum JmpKindTy
    {
        Jmp,
        Je,
        Jne,
        Jg,
        Jle
    };

private:
    AsmJmpInst(AsmJmpInst::JmpKindTy JmpKind, std::unique_ptr<AsmOperand> Op,
               AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Jmp, std::move(Op), InsertAtEnd),
        JmpKind(JmpKind) {}

    AsmJmpInst(AsmJmpInst::JmpKindTy JmpKind, std::unique_ptr<AsmOperand> Op,
               AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Jmp, std::move(Op), InsertBefore),
        JmpKind(JmpKind) {}

public:
    static AsmJmpInst *create(AsmJmpInst::JmpKindTy JmpKind,
                              std::unique_ptr<AsmOperand> Op, AsmFunction *InsertAtEnd) {
        return new AsmJmpInst(JmpKind, std::move(Op), InsertAtEnd);
    }

    static AsmJmpInst *create(AsmJmpInst::JmpKindTy JmpKind,
                              std::unique_ptr<AsmOperand> Op,
                              AsmInstruction *InsertBefore) {
        return new AsmJmpInst(JmpKind, std::move(Op), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Jmp;
    }

    AsmJmpInst::JmpKindTy getJmpKind() const { return JmpKind; }

private:
    AsmJmpInst::JmpKindTy JmpKind;
};

class AsmAddInst: public AsmInstruction {
private:
    AsmAddInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Add, std::move(Src), std::move(Dst), InsertAtEnd) {
    }

    AsmAddInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Add, std::move(Src), std::move(Dst),
                       InsertBefore) {}

public:
    static AsmAddInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst, AsmFunction *InsertAtEnd) {
        return new AsmAddInst(std::move(Src), std::move(Dst), InsertAtEnd);
    }

    static AsmAddInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst,
                              AsmInstruction *InsertBefore) {
        return new AsmAddInst(std::move(Src), std::move(Dst), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Add;
    }
};

class AsmSubInst: public AsmInstruction {
private:
    AsmSubInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Sub, std::move(Src), std::move(Dst), InsertAtEnd) {
    }

    AsmSubInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Sub, std::move(Src), std::move(Dst),
                       InsertBefore) {}

public:
    static AsmSubInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst, AsmFunction *InsertAtEnd) {
        return new AsmSubInst(std::move(Src), std::move(Dst), InsertAtEnd);
    }

    static AsmSubInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst,
                              AsmInstruction *InsertBefore) {
        return new AsmSubInst(std::move(Src), std::move(Dst), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Sub;
    }
};

class AsmImulInst: public AsmInstruction {
private:
    AsmImulInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
                AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Imul, std::move(Src), std::move(Dst),
                       InsertAtEnd) {}

    AsmImulInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
                AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Imul, std::move(Src), std::move(Dst),
                       InsertBefore) {}

public:
    static AsmImulInst *create(std::unique_ptr<AsmOperand> Src,
                               std::unique_ptr<AsmOperand> Dst,
                               AsmFunction *InsertAtEnd) {
        return new AsmImulInst(std::move(Src), std::move(Dst), InsertAtEnd);
    }

    static AsmImulInst *create(std::unique_ptr<AsmOperand> Src,
                               std::unique_ptr<AsmOperand> Dst,
                               AsmInstruction *InsertBefore) {
        return new AsmImulInst(std::move(Src), std::move(Dst), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Imul;
    }
};

class AsmIdivInst: public AsmInstruction {
private:
    AsmIdivInst(std::unique_ptr<AsmOperand> Op, AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Idiv, std::move(Op), InsertAtEnd) {}

    AsmIdivInst(std::unique_ptr<AsmOperand> Op, AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Idiv, std::move(Op), InsertBefore) {}

public:
    static AsmIdivInst *create(std::unique_ptr<AsmOperand> Op, AsmFunction *InsertAtEnd) {
        return new AsmIdivInst(std::move(Op), InsertAtEnd);
    }

    static AsmIdivInst *create(std::unique_ptr<AsmOperand> Op,
                               AsmInstruction *InsertBefore) {
        return new AsmIdivInst(std::move(Op), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Idiv;
    }
};

class AsmCqtoInst: public AsmInstruction {
private:
    AsmCqtoInst(AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Cqto, InsertAtEnd) {}

    AsmCqtoInst(AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Cqto, InsertBefore) {}

public:
    static AsmCqtoInst *create(AsmFunction *InsertAtEnd) {
        return new AsmCqtoInst(InsertAtEnd);
    }

    static AsmCqtoInst *create(AsmInstruction *InsertBefore) {
        return new AsmCqtoInst(InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Cqto;
    }
};

class AsmCallInst: public AsmInstruction {
private:
    bool DirectCall;
    unsigned NumArgs;

    AsmCallInst(std::unique_ptr<AsmOperand> Callee, bool DirectCall, unsigned NumArgs,
                AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Call, std::move(Callee), InsertAtEnd),
        DirectCall(DirectCall), NumArgs(NumArgs) {}

    AsmCallInst(std::unique_ptr<AsmOperand> Callee, bool DirectCall, unsigned NumArgs,
                AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Call, std::move(Callee), InsertBefore),
        DirectCall(DirectCall), NumArgs(NumArgs) {}

public:
    static AsmCallInst *create(std::unique_ptr<AsmOperand> Callee, bool DirectCall,
                               unsigned NumArgs, AsmFunction *InsertAtEnd) {
        return new AsmCallInst(std::move(Callee), DirectCall, NumArgs, InsertAtEnd);
    }

    static AsmCallInst *create(std::unique_ptr<AsmOperand> Callee, bool DirectCall,
                               unsigned NumArgs, AsmInstruction *InsertBefore) {
        return new AsmCallInst(std::move(Callee), DirectCall, NumArgs, InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Call;
    }

    bool isDirectCall() const { return DirectCall; }

    unsigned getNumArgs() const { return NumArgs; }
};

class AsmXorInst: public AsmInstruction {
private:
    AsmXorInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Xor, std::move(Src), std::move(Dst), InsertAtEnd) {
    }

    AsmXorInst(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst,
               AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Xor, std::move(Src), std::move(Dst),
                       InsertBefore) {}

public:
    static AsmXorInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst, AsmFunction *InsertAtEnd) {
        return new AsmXorInst(std::move(Src), std::move(Dst), InsertAtEnd);
    }

    static AsmXorInst *create(std::unique_ptr<AsmOperand> Src,
                              std::unique_ptr<AsmOperand> Dst,
                              AsmInstruction *InsertBefore) {
        return new AsmXorInst(std::move(Src), std::move(Dst), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Xor;
    }
};

class AsmPushInst: public AsmInstruction {
private:
    AsmPushInst(std::unique_ptr<AsmOperand> Op, AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Push, std::move(Op), InsertAtEnd) {}

    AsmPushInst(std::unique_ptr<AsmOperand> Op, AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Push, std::move(Op), InsertBefore) {}

public:
    static AsmPushInst *create(std::unique_ptr<AsmOperand> Op, AsmFunction *InsertAtEnd) {
        return new AsmPushInst(std::move(Op), InsertAtEnd);
    }

    static AsmPushInst *create(std::unique_ptr<AsmOperand> Op,
                               AsmInstruction *InsertBefore) {
        return new AsmPushInst(std::move(Op), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Push;
    }
};

class AsmPopInst: public AsmInstruction {
private:
    AsmPopInst(std::unique_ptr<AsmOperand> Op, AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Pop, std::move(Op), InsertAtEnd) {}

    AsmPopInst(std::unique_ptr<AsmOperand> Op, AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Pop, std::move(Op), InsertBefore) {}

public:
    static AsmPopInst *create(std::unique_ptr<AsmOperand> Op, AsmFunction *InsertAtEnd) {
        return new AsmPopInst(std::move(Op), InsertAtEnd);
    }

    static AsmPopInst *create(std::unique_ptr<AsmOperand> Op,
                              AsmInstruction *InsertBefore) {
        return new AsmPopInst(std::move(Op), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Pop;
    }
};

class AsmRetInst: public AsmInstruction {
private:
    AsmRetInst(AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Ret, InsertAtEnd) {}

    AsmRetInst(AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Ret, InsertBefore) {}

public:
    static AsmRetInst *create(AsmFunction *InsertAtEnd) {
        return new AsmRetInst(InsertAtEnd);
    }

    static AsmRetInst *create(AsmInstruction *InsertBefore) {
        return new AsmRetInst(InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Ret;
    }
};

class AsmLabelInst: public AsmInstruction {
private:
    AsmLabelInst(std::unique_ptr<AsmOperand> LabelOp, AsmFunction *InsertAtEnd):
        AsmInstruction(AsmInstruction::Label, std::move(LabelOp), InsertAtEnd) {}

    AsmLabelInst(std::unique_ptr<AsmOperand> LabelOp, AsmInstruction *InsertBefore):
        AsmInstruction(AsmInstruction::Label, std::move(LabelOp), InsertBefore) {}

public:
    static AsmLabelInst *create(std::unique_ptr<AsmOperand> LabelOp,
                                AsmFunction *InsertAtEnd) {
        return new AsmLabelInst(std::move(LabelOp), InsertAtEnd);
    }

    static AsmLabelInst *create(std::unique_ptr<AsmOperand> LabelOp,
                                AsmInstruction *InsertBefore) {
        return new AsmLabelInst(std::move(LabelOp), InsertBefore);
    }

    static bool classof(const AsmInstruction *I) {
        return I->getInstKind() == AsmInstruction::Label;
    }
};

}  // namespace remniw
