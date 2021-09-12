#include "AsmInstruction.h"
#include "AsmFunction.h"
#include "llvm/Support/Casting.h"

namespace llvm {

void ilist_traits<remniw::AsmInstruction>::addNodeToList(remniw::AsmInstruction *I) {
    assert(!I->getParent() && "instruction already in a basic block");
    I->setParent(Parent);
}

void ilist_traits<remniw::AsmInstruction>::removeNodeFromList(remniw::AsmInstruction *I) {
    assert(I->getParent() && "instruction not in a basic block");
    I->setParent(nullptr);
}

void ilist_traits<remniw::AsmInstruction>::transferNodesFromList(ilist_traits &FromList,
                                                                 instr_iterator First,
                                                                 instr_iterator Last) {
    // If it's within the same Function, there's nothing to do.
    if (this == &FromList)
        return;

    assert(Parent != FromList.Parent && "Two lists have the same parent?");

    // If splicing between two blocks within the same function, just update the
    // parent pointers.
    for (; First != Last; ++First)
        First->setParent(Parent);
}

void ilist_traits<remniw::AsmInstruction>::deleteNode(remniw::AsmInstruction *I) {
    I->deleteValue();
}

};  // namespace llvm

namespace remniw {

AsmInstruction::AsmInstruction(KindTy Kind, AsmFunction *InsertAtEnd):
    Kind(Kind), Parent(nullptr) {
    assert(InsertAtEnd && "Function to append to must not be NULL!");
    InsertAtEnd->getInstList().push_back(this);
}

AsmInstruction::AsmInstruction(KindTy Kind, std::unique_ptr<AsmOperand> Dst,
                               AsmFunction *InsertAtEnd):
    Kind(Kind), Parent(nullptr) {
    assert(InsertAtEnd && "Function to append to must not be NULL!");
    Operands.push_back(std::move(Dst));
    InsertAtEnd->getInstList().push_back(this);
}

AsmInstruction::AsmInstruction(KindTy Kind, std::unique_ptr<AsmOperand> Src,
                               std::unique_ptr<AsmOperand> Dst, AsmFunction *InsertAtEnd):
    Kind(Kind), Parent(nullptr) {
    assert(InsertAtEnd && "Function to append to must not be NULL!");
    Operands.push_back(std::move(Src));
    Operands.push_back(std::move(Dst));
    InsertAtEnd->getInstList().push_back(this);
}

AsmInstruction::AsmInstruction(KindTy Kind, AsmInstruction *InsertBefore):
    Kind(Kind), Parent(nullptr) {
    assert(InsertBefore && "Instruction to insert before must not be NULL!");
    AsmFunction *F = InsertBefore->getParent();
    assert(F && "Instruction to insert before is not in a Function!");
    F->getInstList().insert(InsertBefore->getIterator(), this);
}

AsmInstruction::AsmInstruction(KindTy Kind, std::unique_ptr<AsmOperand> Dst,
                               AsmInstruction *InsertBefore):
    Kind(Kind),
    Parent(nullptr) {
    assert(InsertBefore && "Instruction to insert before must not be NULL!");
    Operands.push_back(std::move(Dst));
    AsmFunction *F = InsertBefore->getParent();
    assert(F && "Instruction to insert before is not in a Function!");
    F->getInstList().insert(InsertBefore->getIterator(), this);
}

AsmInstruction::AsmInstruction(KindTy Kind, std::unique_ptr<AsmOperand> Src,
                               std::unique_ptr<AsmOperand> Dst,
                               AsmInstruction *InsertBefore):
    Kind(Kind),
    Parent(nullptr) {
    assert(InsertBefore && "Instruction to insert before must not be NULL!");
    Operands.push_back(std::move(Src));
    Operands.push_back(std::move(Dst));
    AsmFunction *F = InsertBefore->getParent();
    assert(F && "Instruction to insert before is not in a Function!");
    F->getInstList().insert(InsertBefore->getIterator(), this);
}

void AsmInstruction::removeFromParent() {
    getParent()->getInstList().remove(getIterator());
}

void AsmInstruction::deleteValue() {
    switch (getInstKind()) {
    case AsmInstruction::Mov: delete static_cast<AsmMovInst *>(this); break;
    case AsmInstruction::Lea: delete static_cast<AsmLeaInst *>(this); break;
    case AsmInstruction::Cmp: delete static_cast<AsmCmpInst *>(this); break;
    case AsmInstruction::Jmp: delete static_cast<AsmJmpInst *>(this); break;
    case AsmInstruction::Add: delete static_cast<AsmAddInst *>(this); break;
    case AsmInstruction::Sub: delete static_cast<AsmSubInst *>(this); break;
    case AsmInstruction::Imul: delete static_cast<AsmImulInst *>(this); break;
    case AsmInstruction::Idiv: delete static_cast<AsmIdivInst *>(this); break;
    case AsmInstruction::Cqto: delete static_cast<AsmCqtoInst *>(this); break;
    case AsmInstruction::Call: delete static_cast<AsmCallInst *>(this); break;
    case AsmInstruction::Xor: delete static_cast<AsmXorInst *>(this); break;
    case AsmInstruction::Push: delete static_cast<AsmPushInst *>(this); break;
    case AsmInstruction::Pop: delete static_cast<AsmPopInst *>(this); break;
    case AsmInstruction::Ret: delete static_cast<AsmRetInst *>(this); break;
    case AsmInstruction::Label: delete static_cast<AsmLabelInst *>(this); break;
    default: llvm_unreachable("Attempting to delete unknown kind AsmInstruction");
    }
}

void AsmInstruction::print(llvm::raw_ostream &OS) const {
    switch (getInstKind()) {
    case AsmInstruction::Mov: {
        auto *Inst = llvm::cast<AsmMovInst>(this);
        OS << "\tmovq\t";
        Inst->getOperand(0)->print(OS);
        OS << ", ";
        Inst->getOperand(1)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Lea: {
        auto *Inst = llvm::cast<AsmLeaInst>(this);
        OS << "\tleaq\t";
        Inst->getOperand(0)->print(OS);
        if (Inst->getOperand(0)->isLabel() &&
            (Inst->getOperand(0)->getLabel()->isFunction() ||
             Inst->getOperand(0)->getLabel()->isGlobalVariable())) {
            OS << "(%rip)";  // rip relative addressing
        }
        OS << ", ";
        Inst->getOperand(1)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Cmp: {
        auto *Inst = llvm::cast<AsmCmpInst>(this);
        OS << "\tcmpq\t";
        Inst->getOperand(0)->print(OS);
        OS << ", ";
        Inst->getOperand(1)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Jmp: {
        auto *Inst = llvm::cast<AsmJmpInst>(this);
        switch (Inst->getJmpKind()) {
        case AsmJmpInst::Jmp: OS << "\tjmp\t"; break;
        case AsmJmpInst::Je: OS << "\tje\t"; break;
        case AsmJmpInst::Jne: OS << "\tjne\t"; break;
        case AsmJmpInst::Jg: OS << "\tjg\t"; break;
        case AsmJmpInst::Jle: OS << "\tjle\t"; break;
        default: llvm_unreachable("Invalid AsmJmpInst!");
        }
        Inst->getOperand(0)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Add: {
        auto *Inst = llvm::cast<AsmAddInst>(this);
        OS << "\taddq\t";
        Inst->getOperand(0)->print(OS);
        OS << ", ";
        Inst->getOperand(1)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Sub: {
        auto *Inst = llvm::cast<AsmSubInst>(this);
        OS << "\tsubq\t";
        Inst->getOperand(0)->print(OS);
        OS << ", ";
        Inst->getOperand(1)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Imul: {
        auto *Inst = llvm::cast<AsmImulInst>(this);
        OS << "\timulq\t";
        Inst->getOperand(0)->print(OS);
        OS << ", ";
        Inst->getOperand(1)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Idiv: {
        auto *Inst = llvm::cast<AsmIdivInst>(this);
        OS << "\tidivq\t";
        Inst->getOperand(0)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Cqto: {
        OS << "\tcqto\n";
        break;
    }
    case AsmInstruction::Call: {
        auto *Inst = llvm::cast<AsmCallInst>(this);
        OS << "\tcallq\t";
        if (!Inst->isDirectCall())
            OS << "*";
        Inst->getOperand(0)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Xor: {
        auto *Inst = llvm::cast<AsmXorInst>(this);
        OS << "\txorq\t";
        Inst->getOperand(0)->print(OS);
        OS << ", ";
        Inst->getOperand(1)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Push: {
        auto *Inst = llvm::cast<AsmPushInst>(this);
        OS << "\tpushq\t";
        Inst->getOperand(0)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Pop: {
        auto *Inst = llvm::cast<AsmPopInst>(this);
        OS << "\tpopq\t";
        Inst->getOperand(0)->print(OS);
        OS << "\n";
        break;
    }
    case AsmInstruction::Ret: {
        OS << "\tretq\n";
        break;
    }
    case AsmInstruction::Label: {
        auto *Inst = llvm::cast<AsmLabelInst>(this);
        Inst->getOperand(0)->print(OS);
        OS << ":\n";
        break;
    }
    default: llvm_unreachable("Invalid AsmInstruction");
    }
}

}  // namespace remniw
