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
    static AsmInstruction *create(unsigned Opcode, AsmFunction *InsertAtEnd) {
        return new AsmInstruction(Opcode, InsertAtEnd);
    }

    static AsmInstruction *create(unsigned Opcode, AsmInstruction *InsertBefore) {
        return new AsmInstruction(Opcode, InsertBefore);
    }

    inline const AsmFunction *getParent() const { return Parent; }

    inline AsmFunction *getParent() { return Parent; }

    void setParent(AsmFunction *P) { Parent = P; }

    void removeFromParent();

    unsigned getNumOperands() const { return Operands.size(); }

    void setOperand(unsigned i, AsmOperand Op) { Operands[i] = std::move(Op); }

    const AsmOperand &getOperand(unsigned i) const { return Operands[i]; }
    AsmOperand &getOperand(unsigned i) { return Operands[i]; }

    void addOperand(const AsmOperand Op) { Operands.push_back(Op); }

    void setOpcode(unsigned Op) { Opcode = Op; }
    unsigned getOpcode() const { return Opcode; }

    void deleteValue();

    void print(llvm::raw_ostream &OS) const;

private:
    AsmInstruction(unsigned Opcode, AsmFunction *InsertAtEnd);
    AsmInstruction(unsigned Opcode, AsmInstruction *InsertBefore);

    unsigned Opcode;
    llvm::SmallVector<AsmOperand, 4> Operands;
    AsmFunction *Parent;
};

}  // namespace remniw
