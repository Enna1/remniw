#pragma once

#include "AsmSymbol.h"
#include "Register.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Value.h"
#include <cassert>
#include <cstdint>
#include <memory>

namespace remniw {

struct AsmOperand {
public:
    enum KindTy {
        AO_Register,
        AO_Memory,
        AO_Immediate,
        AO_Label,
        AO_StackObject
    } Kind;

    struct RegOp {
        uint32_t RegNo;
    };

    struct MemOp {
        int64_t Disp;
        uint32_t BaseReg;
        uint32_t IndexReg;
        uint32_t Scale;
    };

    struct ImmOp {
        int64_t Val;
    };

    struct LabelOp {
        AsmSymbol* Symbol;
    };

    struct StackObject {
        // Index < 0, incoming function argument on stack
        // Index >= 0, local stack frame object
        int Index;
        // The size of this object on the stack.
        uint64_t Size;
        // The offset of this object from the stack pointer on entry to the function.
        int64_t Offset;
        // If this stack object is originated from an Alloca instruction
        // this value saves the original IR allocation.
        // If this stack object is originated from a function argument this
        // value saves the origin IR incoming formal argument to the function.
        llvm::Value *V;
    };

    union {
        struct RegOp Reg;
        struct MemOp Mem;
        struct ImmOp Imm;
        struct LabelOp Lbl;
        struct StackObject StackObj;
    };

    AsmOperand(RegOp Reg): Kind(KindTy::AO_Register), Reg(Reg) {}
    AsmOperand(MemOp Mem): Kind(KindTy::AO_Memory), Mem(Mem) {}
    AsmOperand(ImmOp Imm): Kind(KindTy::AO_Immediate), Imm(Imm) {}
    AsmOperand(LabelOp Lbl): Kind(KindTy::AO_Label), Lbl(Lbl) {}
    AsmOperand(StackObject StackObj): Kind(KindTy::AO_StackObject), StackObj(StackObj) {}

    static AsmOperand::RegOp createReg(uint32_t RegNo) { return {RegNo}; }

    static AsmOperand::ImmOp createImm(int64_t Val) { return {Val}; }

    static AsmOperand::StackObject createStackObject(llvm::Value *V, int Index,
                                                     uint64_t Size,
                                                     int64_t Offset) {
        return {Index, Size, Offset, V};
    }

    static AsmOperand::MemOp createMem(int64_t Disp, uint32_t BaseReg,
                                       uint32_t IndexReg = Register::NoRegister,
                                       uint32_t Scale = 1) {
        return {Disp, BaseReg, IndexReg, Scale};
    }

    static AsmOperand::LabelOp createLabel(AsmSymbol* Symbol) { return {Symbol}; }

    static AsmOperand create(AsmOperand::RegOp Reg) { return {Reg}; }

    static AsmOperand create(AsmOperand::ImmOp Imm) { return {Imm}; }

    static AsmOperand create(AsmOperand::StackObject StackObj) { return {StackObj}; }

    static AsmOperand create(AsmOperand::MemOp Mem) { return {Mem}; }

    static AsmOperand create(AsmOperand::LabelOp Lbl) { return {Lbl}; }


    std::unique_ptr<AsmOperand> clone() { return std::make_unique<AsmOperand>(*this); }

    bool isReg() const { return Kind == AO_Register; }

    bool isVirtReg() const {
        return Kind == AO_Register && Register::isVirtualRegister(Reg.RegNo);
    }

    bool isPhysReg() const {
        return Kind == AO_Register && Register::isPhysicalRegister(Reg.RegNo);
    }

    bool isStackSlotReg() const {
        return Kind == AO_Register && Register::isStackSlot(Reg.RegNo);
    }

    bool isMem() const { return Kind == AO_Memory; }

    bool isImm() const { return Kind == AO_Immediate; }

    bool isLabel() const { return Kind == AO_Label; }

    bool isStackObject() const { return Kind == AO_StackObject; }

    uint32_t getReg() const {
        assert(Kind == AO_Register && "Not a Register AsmOperand");
        return Reg.RegNo;
    }

    uint32_t getMemDisp() const {
        assert(Kind == AO_Memory && "Not a Memory AsmOperand");
        return Mem.Disp;
    }

    uint32_t getMemBaseReg() const {
        assert(Kind == AO_Memory && "Not a Memory AsmOperand");
        return Mem.BaseReg;
    }

    uint32_t getMemIndexReg() const {
        assert(Kind == AO_Memory && "Not a Memory AsmOperand");
        return Mem.IndexReg;
    }

    uint32_t getMemScale() const {
        assert(Kind == AO_Memory && "Not a Memory AsmOperand");
        return Mem.Scale;
    }

    AsmSymbol* getLabel() const {
        assert(Kind == AO_Label && "Not a Label AsmOperand");
        return Lbl.Symbol;
    }

    void print(llvm::raw_ostream& OS) const {
        switch (Kind) {
        case AO_Register: OS << "REG: " << Reg.RegNo; break;
        case AO_Immediate: OS << "IMM: " << Imm.Val; break;
        case AO_Memory:
            OS << "MEM: ";
            if (Mem.Disp != 0)
                OS << Mem.Disp;
            OS << "(";
            if (Mem.BaseReg)
                OS << (Mem.BaseReg);
            if (Mem.IndexReg) {
                OS << ", " << (Mem.IndexReg);
                OS << ", " << Mem.Scale;
            }
            OS << ")";
            break;
        case AO_Label:
            OS << "LABEL: ";
            Lbl.Symbol->print(OS);
            break;
        case AO_StackObject:
            // TODO
            OS << "STACKOBJECT: ";
            break;
        default: llvm_unreachable("Invalid AsmOperand");
        }
    }
};

}  // namespace remniw
