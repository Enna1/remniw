#pragma once

#include "codegen/asm/AsmRewriter.h"
#include "codegen/asm/X86/X86TargetInfo.h"

namespace remniw {

class X86AsmRewriter: public AsmRewriter {
public:
    X86AsmRewriter(const TargetInfo &TI): AsmRewriter(TI) {}

private:
    void rewriteAsmInstVirtRegToPhysReg(
        AsmInstruction *I,
        const llvm::DenseMap<uint32_t, uint32_t> &VirtToAllocRegMap) override {
        switch (I->getOpcode()) {
        case X86::MOV:
        case X86::LEA:
        case X86::CMP:
        case X86::ADD:
        case X86::SUB:
        case X86::IMUL:
        case X86::XOR:
            rewriteAsmOperandVirtRegToPhysReg(I->getOperand(0), VirtToAllocRegMap);
            rewriteAsmOperandVirtRegToPhysReg(I->getOperand(1), VirtToAllocRegMap);
            break;
        case X86::IDIV:
        case X86::CALL:
            rewriteAsmOperandVirtRegToPhysReg(I->getOperand(0), VirtToAllocRegMap);
            break;
        case X86::JMP:
        case X86::JE:
        case X86::JNE:
        case X86::JG:
        case X86::JLE:
        case X86::CQTO:
        case X86::LABEL: break;
        default: llvm_unreachable("Invalid AsmInstruction");
        }
    }

    void rewriteAsmInstSpilledRegToStackSlot(
        AsmInstruction *I,
        const llvm::DenseMap<uint32_t, uint32_t> &VirtToAllocRegMap) override {
        size_t NumOperands = I->getNumOperands();
        assert(NumOperands <= 2 && "The #operands of X86AsmInst must <= 2");
        NumReversedStackSlotForReg = 0;
        llvm::SmallVector<uint32_t, 4> UsedRegs;
        getUsedRegisters(I, UsedRegs);
        for (unsigned i = 0; i < I->getNumOperands(); ++i) {
            AsmOperand &Op = I->getOperand(i);
            // If operand is virtual register, spill it to stack slot
            if (Op.isVirtReg()) {
                rewriteAsmOperandSpilledRegToStackSlot(Op.Reg.RegNo, I, VirtToAllocRegMap,
                                                       UsedRegs);
            }
            // If operand is memory, if memory base register or memory index register is
            // virtual register, spill it to stack slot
            if (Op.isMem()) {
                if (Register::isVirtualRegister(Op.Mem.BaseReg)) {
                    rewriteAsmOperandSpilledRegToStackSlot(Op.Mem.BaseReg, I,
                                                           VirtToAllocRegMap, UsedRegs);
                }
                if (Register::isVirtualRegister(Op.Mem.IndexReg)) {
                    rewriteAsmOperandSpilledRegToStackSlot(Op.Mem.IndexReg, I,
                                                           VirtToAllocRegMap, UsedRegs);
                }
            }
        }

        if (NumReversedStackSlotForReg > MaxNumReversedStackSlotForReg)
            MaxNumReversedStackSlotForReg = NumReversedStackSlotForReg;
    }

    // TODO: optimize generated spilled code
    void rewriteAsmOperandSpilledRegToStackSlot(
        uint32_t &SpilledReg, AsmInstruction *I,
        const llvm::DenseMap<uint32_t, uint32_t> &VirtToAllocRegMap,
        llvm::SmallVectorImpl<uint32_t> &UsedRegs) {
        assert(VirtToAllocRegMap.count(SpilledReg));
        uint32_t AllocatedReg = VirtToAllocRegMap.lookup(SpilledReg);
        assert(Register::isStackSlot(AllocatedReg));

        // Select an available register, use it later. There muse be available register as
        // one instruction cannot use all registers.
        uint32_t AvailReg = getAvailableRegister(I, UsedRegs);
        assert(AvailReg != Register::NoRegister);
        UsedRegs.push_back(AvailReg);

        // Spilling
        int64_t ReservedStackSlotOffset = getReservedStackSlotOffsetForReg();
        int64_t StackSlotOffset = getStackSlotOffsetForSpilledReg(AllocatedReg);
        // Insert an instruction which stores the original content of AvailReg to stack
        auto *MI1 = AsmInstruction::create(X86::MOV, I);
        MI1->addOperand(AsmOperand::createReg(AvailReg));
        MI1->addOperand(AsmOperand::createMem(ReservedStackSlotOffset, X86::RBP));
        // Insert an instruction which moves the content of stack slot to AvailReg
        auto *MI2 = AsmInstruction::create(X86::MOV, I);
        MI2->addOperand(AsmOperand::createMem(StackSlotOffset, X86::RBP));
        MI2->addOperand(AsmOperand::createReg(AvailReg));
        // Replace the SpilledReg with AvailReg
        SpilledReg = AvailReg;
        // When current instruction has been executed, the content of AvailReg may be
        // updated. Insert instructions to store the content of AvailReg back to spilled
        // stack slot and restore the content of AvailReg.
        if (auto *NextInst = I->getNextNode()) /* Insert before next instruction */ {
            auto *MI1 = AsmInstruction::create(X86::MOV, NextInst);
            MI1->addOperand(AsmOperand::createReg(AvailReg));
            MI1->addOperand(AsmOperand::createMem(StackSlotOffset, X86::RBP));
            auto *MI2 = AsmInstruction::create(X86::MOV, NextInst);
            MI2->addOperand(AsmOperand::createMem(ReservedStackSlotOffset, X86::RBP));
            MI2->addOperand(AsmOperand::createReg(AvailReg));
        } else /* Insert at end of function */ {
            auto *MI1 = AsmInstruction::create(X86::MOV, I->getParent());
            MI1->addOperand(AsmOperand::createReg(AvailReg));
            MI1->addOperand(AsmOperand::createMem(StackSlotOffset, X86::RBP));
            auto *MI2 = AsmInstruction::create(X86::MOV, I->getParent());
            MI2->addOperand(AsmOperand::createMem(ReservedStackSlotOffset, X86::RBP));
            MI2->addOperand(AsmOperand::createReg(AvailReg));
        }
    }

    void insertPrologue(AsmFunction *F,
                        llvm::SmallVectorImpl<uint32_t> &UsedCalleeSavedRegs) override {
        AsmInstruction *InsertBefore = &F->front();

        // Specially handle main function
        if (F->FuncName != "main") {
            for (uint32_t Reg : UsedCalleeSavedRegs) {
                auto *I = AsmInstruction::create(X86::PUSH, InsertBefore);
                I->addOperand(AsmOperand::createReg(Reg));
            }
        }

        // Push RBP(frame pointer) on stack
        auto *PI = AsmInstruction::create(X86::PUSH, InsertBefore);
        PI->addOperand(AsmOperand::createReg(X86::RBP));

        // Mov RSP(stack pointer) to RBP(frame pointer)
        auto *MI = AsmInstruction::create(X86::MOV, InsertBefore);
        MI->addOperand(AsmOperand::createReg(X86::RSP));
        MI->addOperand(AsmOperand::createReg(X86::RBP));

        // Reserve space on the stack
        int64_t NeededStackSizeInBytes =
            F->StackSizeInBytes + X86::RegisterSize * NumSpilledReg +
            X86::RegisterSize * MaxNumReversedStackSlotForReg;
        int64_t TotalStackFrameSizeInBytes = NeededStackSizeInBytes +
                                             X86::RegisterSize /*push $rbp*/ +
                                             X86::RegisterSize /*return address*/;
        if (F->FuncName != "main")
            TotalStackFrameSizeInBytes += UsedCalleeSavedRegs.size() * X86::RegisterSize;
        // x86-64 / AMD64 System V ABI requires 16-byte stack alignment
        if (TotalStackFrameSizeInBytes % 16)
            NeededStackSizeInBytes += 16 - TotalStackFrameSizeInBytes % 16;
        auto *SI = AsmInstruction::create(X86::SUB, InsertBefore);
        SI->addOperand(AsmOperand::createImm(NeededStackSizeInBytes));
        SI->addOperand(AsmOperand::createReg(X86::RSP));
    }

    void insertEpilogue(remniw::AsmFunction *F,
                        llvm::SmallVectorImpl<uint32_t> &UsedCalleeSavedRegs) override {
        // Mov RBP(frame pointer) to RSP(stack pointer)
        auto *MI = AsmInstruction::create(X86::MOV, F);
        MI->addOperand(AsmOperand::createReg(X86::RBP));
        MI->addOperand(AsmOperand::createReg(X86::RSP));

        // Pop RBP(frame pointer) on stack
        auto *PI = AsmInstruction::create(X86::POP, F);
        PI->addOperand(AsmOperand::createReg(X86::RBP));

        // Specially handle main function
        if (F->FuncName != "main") {
            for (auto i = UsedCalleeSavedRegs.rbegin(), e = UsedCalleeSavedRegs.rend();
                 i != e; ++i) {
                auto *PI = AsmInstruction::create(X86::POP, F);
                PI->addOperand(AsmOperand::createReg(*i));
            }
        }

        // Return
        AsmInstruction::create(X86::RET, F);
    }

    void getUsedRegisters(AsmInstruction *I,
                          llvm::SmallVectorImpl<uint32_t> &UsedRegs) override {
        unsigned Opcode = I->getOpcode();
        if (Opcode == X86::IDIV || Opcode == X86::CQTO) {
            UsedRegs.push_back(X86::RAX);
            UsedRegs.push_back(X86::RDX);
        }
        if (Opcode == X86::CALL) {
            UsedRegs.push_back(X86::RAX);
            // First operand of X86::CALL instruction is callee, second operand of
            // X86::CALL instruction is NumArgs.
            assert(I->getNumOperands() == 2 &&
                   "The #operands of X86::CALL instruction must be 2");
            for (unsigned i = 0; i < I->getOperand(1).Imm.Val && i < X86::NumArgRegs; ++i)
                UsedRegs.push_back(X86::ArgRegs[i]);
        }
        for (unsigned i = 0; i < I->getNumOperands(); ++i) {
            auto &AsmOp = I->getOperand(i);
            if (AsmOp.isPhysReg()) {
                UsedRegs.push_back(AsmOp.Reg.RegNo);
            }
            if (AsmOp.isMem()) {
                if (Register::isPhysicalRegister(AsmOp.Mem.BaseReg))
                    UsedRegs.push_back(AsmOp.Mem.BaseReg);
                if (Register::isPhysicalRegister(AsmOp.Mem.IndexReg))
                    UsedRegs.push_back(AsmOp.Mem.IndexReg);
            }
        }
    }

    uint32_t getAvailableRegister(AsmInstruction *I,
                                  llvm::SmallVectorImpl<uint32_t> &UsedRegs) override {
        for (uint32_t Reg = X86::RAX; Reg <= X86::R15; ++Reg) {
            if (std::find(UsedRegs.begin(), UsedRegs.end(), Reg) != UsedRegs.end())
                continue;
            return Reg;
        }
        return Register::NoRegister;
    }
};

}  // namespace remniw
