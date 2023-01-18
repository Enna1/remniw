#pragma once

#include "codegen/asm/AsmRewriter.h"
#include "codegen/asm/RISCV/RISCVTargetInfo.h"

namespace remniw {

class RISCVAsmRewriter: public AsmRewriter {
private:
    int64_t NeededStackSizeInBytes {0};
    int64_t TotalStackFrameSizeInBytes {0};

public:
    RISCVAsmRewriter(const TargetInfo &TI): AsmRewriter(TI) {}

private:
    void rewriteAsmInstVirtRegToPhysReg(
        AsmInstruction *I,
        const llvm::DenseMap<uint32_t, uint32_t> &VirtToAllocRegMap) override {
        for (unsigned i = 0; i < I->getNumOperands(); ++i) {
            rewriteAsmOperandVirtRegToPhysReg(I->getOperand(i), VirtToAllocRegMap);
        }
    }

    void rewriteAsmInstSpilledRegToStackSlot(
        AsmInstruction *I,
        const llvm::DenseMap<uint32_t, uint32_t> &VirtToAllocRegMap) override {
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
        auto *MI1 = AsmInstruction::create(RISCV::SD, I);
        MI1->addOperand(AsmOperand::createReg(AvailReg));
        MI1->addOperand(AsmOperand::createMem(ReservedStackSlotOffset, RISCV::FP));
        // Insert an instruction which moves the content of stack slot to AvailReg
        auto *MI2 = AsmInstruction::create(RISCV::LD, I);
        MI2->addOperand(AsmOperand::createReg(AvailReg));
        MI2->addOperand(AsmOperand::createMem(StackSlotOffset, RISCV::FP));
        // Replace the SpilledReg with AvailReg
        SpilledReg = AvailReg;
        // When current instruction has been executed, the content of AvailReg may be
        // updated. Insert instructions to store the content of AvailReg back to spilled
        // stack slot and restore the content of AvailReg.
        if (auto *NextInst = I->getNextNode()) /* Insert before next instruction */ {
            auto *MI1 = AsmInstruction::create(RISCV::SD, NextInst);
            MI1->addOperand(AsmOperand::createReg(AvailReg));
            MI1->addOperand(AsmOperand::createMem(StackSlotOffset, RISCV::FP));
            auto *MI2 = AsmInstruction::create(RISCV::LD, NextInst);
            MI2->addOperand(AsmOperand::createReg(AvailReg));
            MI2->addOperand(AsmOperand::createMem(ReservedStackSlotOffset, RISCV::FP));
        } else /* Insert at end of function */ {
            auto *MI1 = AsmInstruction::create(RISCV::SD, I->getParent());
            MI1->addOperand(AsmOperand::createReg(AvailReg));
            MI1->addOperand(AsmOperand::createMem(StackSlotOffset, RISCV::FP));
            auto *MI2 = AsmInstruction::create(RISCV::LD, I->getParent());
            MI2->addOperand(AsmOperand::createReg(AvailReg));
            MI2->addOperand(AsmOperand::createMem(ReservedStackSlotOffset, RISCV::FP));
        }
    }

    void insertPrologue(AsmFunction *F,
                        llvm::SmallVectorImpl<uint32_t> &UsedCalleeSavedRegs) override {
        AsmInstruction *InsertBefore = &F->front();

        NeededStackSizeInBytes =
            F->StackSizeInBytes + RISCV::RegisterSize * NumSpilledReg +
            RISCV::RegisterSize * MaxNumReversedStackSlotForReg;
        TotalStackFrameSizeInBytes = NeededStackSizeInBytes +
                                             RISCV::RegisterSize /*frame pointer*/ +
                                             RISCV::RegisterSize /*return address*/;
        if (F->FuncName != "main")
            TotalStackFrameSizeInBytes +=
                UsedCalleeSavedRegs.size() * RISCV::RegisterSize;
        // FIXME: x86-64 / AMD64 System V ABI requires 16-byte stack alignment, what about RISCV?
        if (TotalStackFrameSizeInBytes % 16)
            TotalStackFrameSizeInBytes += 16 - TotalStackFrameSizeInBytes % 16;

        // Reserve space on the stack
        auto *AI = AsmInstruction::create(RISCV::ADDI, InsertBefore);
        AI->addOperand(AsmOperand::createReg(RISCV::SP));
        AI->addOperand(AsmOperand::createReg(RISCV::SP));
        AI->addOperand(AsmOperand::createImm(-TotalStackFrameSizeInBytes));

        int64_t TmpOffsetFromStackPointer = NeededStackSizeInBytes;
        // Save return address register on stack
        auto *SDRA = AsmInstruction::create(RISCV::SD, InsertBefore);
        SDRA->addOperand(AsmOperand::createReg(RISCV::RA));
        SDRA->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));
        TmpOffsetFromStackPointer +=  RISCV::RegisterSize;

        // Save frame pointer on stack
        auto *SDFP = AsmInstruction::create(RISCV::SD, InsertBefore);
        SDFP->addOperand(AsmOperand::createReg(RISCV::FP));
        SDFP->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));
        TmpOffsetFromStackPointer +=  RISCV::RegisterSize;

        // Save callee-saved registers on stack, treat main function as special case
        if (F->FuncName != "main") {
            for (uint32_t Reg : UsedCalleeSavedRegs) {
                auto *I = AsmInstruction::create(RISCV::SD, InsertBefore);
                I->addOperand(AsmOperand::createReg(Reg));
                I->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));
                TmpOffsetFromStackPointer +=  RISCV::RegisterSize;
            }
        }

        // Update frame pointer
        auto *UpdateFP = AsmInstruction::create(RISCV::ADDI, InsertBefore);
        UpdateFP->addOperand(AsmOperand::createReg(RISCV::FP));
        UpdateFP->addOperand(AsmOperand::createReg(RISCV::SP));
        UpdateFP->addOperand(AsmOperand::createImm(NeededStackSizeInBytes));
    }

    void insertEpilogue(remniw::AsmFunction *F,
                        llvm::SmallVectorImpl<uint32_t> &UsedCalleeSavedRegs) override {
        int64_t TmpOffsetFromStackPointer = NeededStackSizeInBytes;
        // Restore return address register
        auto *RTRA = AsmInstruction::create(RISCV::LD, F);
        RTRA->addOperand(AsmOperand::createReg(RISCV::RA));
        RTRA->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));
        TmpOffsetFromStackPointer +=  RISCV::RegisterSize;

        // Restore frame pointer
        auto *RTFP = AsmInstruction::create(RISCV::LD, F);
        RTFP->addOperand(AsmOperand::createReg(RISCV::FP));
        RTFP->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));
        TmpOffsetFromStackPointer +=  RISCV::RegisterSize;

        // Restore callee-saved registers, treat main function as special case
        if (F->FuncName != "main") {
            for (uint32_t Reg : UsedCalleeSavedRegs) {
                auto *I = AsmInstruction::create(RISCV::LD, F);
                I->addOperand(AsmOperand::createReg(Reg));
                I->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));
                TmpOffsetFromStackPointer +=  RISCV::RegisterSize;
            }
        }

        // Restore stack pointer
        auto *UpdateSP = AsmInstruction::create(RISCV::ADDI, F);
        UpdateSP->addOperand(AsmOperand::createReg(RISCV::SP));
        UpdateSP->addOperand(AsmOperand::createReg(RISCV::SP));
        UpdateSP->addOperand(AsmOperand::createImm(TotalStackFrameSizeInBytes));

        // Return
        AsmInstruction::create(RISCV::RET, F);
    }

    void getUsedRegisters(AsmInstruction *I,
                          llvm::SmallVectorImpl<uint32_t> &UsedRegs) override {
        unsigned Opcode = I->getOpcode();
        if (Opcode == RISCV::CALL) {
            UsedRegs.push_back(RISCV::A0);
            // First operand of RISCV::CALL instruction is callee, second operand of
            // RISCV::CALL instruction is NumArgs.
            assert(I->getNumOperands() == 2 &&
                   "The #operands of RISCV::CALL AsmInstruction must be 2");
            for (unsigned i = 0; i < I->getOperand(1).Imm.Val && i < RISCV::NumArgRegs;
                 ++i)
                UsedRegs.push_back(RISCV::ArgRegs[i]);
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
        llvm::ArrayRef<uint32_t> FreeRegs = RISCV::FreeRegs;
        for (auto Reg : FreeRegs) {
            if (std::find(UsedRegs.begin(), UsedRegs.end(), Reg) != UsedRegs.end())
                continue;
            return Reg;
        }
        return Register::NoRegister;
    }
};

}  // namespace remniw
