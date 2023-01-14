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

AsmInstruction::AsmInstruction(unsigned Opcode, AsmFunction *InsertAtEnd):
    Opcode(Opcode), Parent(nullptr) {
    assert(InsertAtEnd && "Function to append to must not be NULL!");
    InsertAtEnd->getInstList().push_back(this);
}

AsmInstruction::AsmInstruction(unsigned Opcode, AsmInstruction *InsertBefore):
    Opcode(Opcode), Parent(nullptr) {
    assert(InsertBefore && "Instruction to insert before must not be NULL!");
    AsmFunction *F = InsertBefore->getParent();
    assert(F && "Instruction to insert before is not in a Function!");
    F->getInstList().insert(InsertBefore->getIterator(), this);
}

void AsmInstruction::removeFromParent() {
    getParent()->getInstList().remove(getIterator());
}

void AsmInstruction::deleteValue() {
    delete this;
}

void AsmInstruction::print(llvm::raw_ostream &OS) const {
    OS << "<AsmInstruction " << getOpcode();
    for (unsigned i = 0, e = getNumOperands(); i != e; ++i) {
        OS << " ";
        getOperand(i).print(OS);
    }
    OS << ">";
}

}  // namespace remniw
