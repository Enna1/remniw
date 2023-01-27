#pragma once

#include "codegen/asm/AsmRewriter.h"
#include "codegen/asm/RISCV/RISCVTargetInfo.h"

namespace remniw {

class RISCVAsmRewriter: public AsmRewriter {
private:
    int64_t StackSizeForCalleeSavedRegs {0};
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

    // The stack frame layout:
    //
    // | Incoming arguments      |
    // | passed via stack.       |
    // +-------------------------+ <- Old SP, New FP. High address
    // | saved register ra       |
    // | saved register fp       |
    // +- - - - - - - - - - - - -+
    // | space for other         |
    // | callee-saved registers  |
    // +- - - - - - - - - - - - -+
    // | local vars space        | <- LocalFrame
    // +- - - - - - - - - - - - -+
    // | space for spilled regs  | <- SpillFrame
    // +- - - - - - - - - - - - -+
    // | parameter area for      | <- CallFrame
    // | called functions        |
    // +-------------------------+ <- New SP. Low address

    void insertPrologue(AsmFunction *F,
                        llvm::SetVector<uint32_t> &UsedCalleeSavedRegs) override {
        AsmInstruction *InsertBefore = &F->front();

        TotalStackFrameSizeInBytes = F->MaxCallFrameSize /* space for call frame */ +
            (RISCV::RegisterSize * NumSpilledReg + RISCV::RegisterSize * MaxNumReversedStackSlotForReg) /* space for spill frame */ +
            F->LocalFrameSize /* space for local frame */;
        if (F->FuncName != "main") {
            StackSizeForCalleeSavedRegs = UsedCalleeSavedRegs.size() * RISCV::RegisterSize;
            TotalStackFrameSizeInBytes += StackSizeForCalleeSavedRegs;
        } else {
            StackSizeForCalleeSavedRegs = 0;
        }
        TotalStackFrameSizeInBytes += RISCV::RegisterSize /* space for saved register ra */ +
                                      RISCV::RegisterSize /* space for saved register fp */;

        // The stack alignment for RV32 and RV64 is 16, for RV32E is 4. Align to 16 for simplicity.
        if (TotalStackFrameSizeInBytes % 16)
            TotalStackFrameSizeInBytes += 16 - TotalStackFrameSizeInBytes % 16;

        // Reserve space on the stack
        assert(TotalStackFrameSizeInBytes >= 0);
        int64_t SPAdjustAmount = 0;
        int64_t TmpStackFrameSizeInBytes = TotalStackFrameSizeInBytes;
        if (TotalStackFrameSizeInBytes > 2047) {
            // SPAdjustAmount is choosed as (2048 - StackAlign), because 2048 will cause sp = sp + 2048 in epilogue split into
            // multi-instructions. The offset smaller than 2048 can fit in signle load/store instruction and we have to stick with the stack alignment.
            // 2048 is 16-byte alignment. The stack alignment for RV32 and RV64 is 16,
            // for RV32E is 4. So (2048 - StackAlign) will satisfy the stack alignment.
            // In this way, the offset of the callee saved register could fit in a single store.
            SPAdjustAmount = 2048 - 16 /* StackAlign */;
            TmpStackFrameSizeInBytes = SPAdjustAmount;
        }
        int64_t TmpOffsetFromStackPointer = TmpStackFrameSizeInBytes;
        auto *AI = AsmInstruction::create(RISCV::ADDI, InsertBefore);
        AI->addOperand(AsmOperand::createReg(RISCV::SP));
        AI->addOperand(AsmOperand::createReg(RISCV::SP));
        AI->addOperand(AsmOperand::createImm(-TmpOffsetFromStackPointer));

        // Save return address register on stack
        TmpOffsetFromStackPointer -= RISCV::RegisterSize;
        auto *SDRA = AsmInstruction::create(RISCV::SD, InsertBefore);
        SDRA->addOperand(AsmOperand::createReg(RISCV::RA));
        SDRA->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));

        // Save frame pointer on stack
        TmpOffsetFromStackPointer -= RISCV::RegisterSize;
        auto *SDFP = AsmInstruction::create(RISCV::SD, InsertBefore);
        SDFP->addOperand(AsmOperand::createReg(RISCV::FP));
        SDFP->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));

        // Save callee-saved registers on stack, treat main function as special case
        if (F->FuncName != "main") {
            for (uint32_t Reg : UsedCalleeSavedRegs) {
                TmpOffsetFromStackPointer -= RISCV::RegisterSize;
                auto *I = AsmInstruction::create(RISCV::SD, InsertBefore);
                I->addOperand(AsmOperand::createReg(Reg));
                I->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));
            }
        }

        // Generate new frame pointer
        auto *UpdateFP = AsmInstruction::create(RISCV::ADDI, InsertBefore);
        UpdateFP->addOperand(AsmOperand::createReg(RISCV::FP));
        UpdateFP->addOperand(AsmOperand::createReg(RISCV::SP));
        UpdateFP->addOperand(AsmOperand::createImm(TmpStackFrameSizeInBytes));

        // Emit the second SP adjustment
        if (SPAdjustAmount) {
            // it is safe to use t0 in prologue
            auto *LI = AsmInstruction::create(RISCV::LI, InsertBefore);
            LI->addOperand(AsmOperand::createReg(RISCV::T0));
            LI->addOperand(AsmOperand::createImm(TotalStackFrameSizeInBytes - SPAdjustAmount));
            auto *SI = AsmInstruction::create(RISCV::SUB, InsertBefore);
            SI->addOperand(AsmOperand::createReg(RISCV::SP));
            SI->addOperand(AsmOperand::createReg(RISCV::SP));
            SI->addOperand(AsmOperand::createReg(RISCV::T0));
        }
    }

    void insertEpilogue(remniw::AsmFunction *F,
                        llvm::SetVector<uint32_t> &UsedCalleeSavedRegs) override {
        int64_t SPAdjustAmount = 0;
        int64_t TmpStackFrameSizeInBytes = TotalStackFrameSizeInBytes;
        if (TotalStackFrameSizeInBytes > 2047) {
            SPAdjustAmount = 2048 - 16 /* StackAlign */;
            TmpStackFrameSizeInBytes = SPAdjustAmount;
        }
        int64_t TmpOffsetFromStackPointer = TmpStackFrameSizeInBytes;

        if (SPAdjustAmount) {
            // it is safe to use t0 in epilogue
            auto *LI = AsmInstruction::create(RISCV::LI, F);
            LI->addOperand(AsmOperand::createReg(RISCV::T0));
            LI->addOperand(AsmOperand::createImm(TotalStackFrameSizeInBytes - SPAdjustAmount));
            auto *SI = AsmInstruction::create(RISCV::ADD, F);
            SI->addOperand(AsmOperand::createReg(RISCV::SP));
            SI->addOperand(AsmOperand::createReg(RISCV::SP));
            SI->addOperand(AsmOperand::createReg(RISCV::T0));
        }

        // Restore return address register
        TmpOffsetFromStackPointer -=  RISCV::RegisterSize;
        auto *RTRA = AsmInstruction::create(RISCV::LD, F);
        RTRA->addOperand(AsmOperand::createReg(RISCV::RA));
        RTRA->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));

        // Restore frame pointer
        TmpOffsetFromStackPointer -=  RISCV::RegisterSize;
        auto *RTFP = AsmInstruction::create(RISCV::LD, F);
        RTFP->addOperand(AsmOperand::createReg(RISCV::FP));
        RTFP->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));

        // Restore callee-saved registers, treat main function as special case
        if (F->FuncName != "main") {
            for (uint32_t Reg : UsedCalleeSavedRegs) {
                TmpOffsetFromStackPointer -=  RISCV::RegisterSize;
                auto *I = AsmInstruction::create(RISCV::LD, F);
                I->addOperand(AsmOperand::createReg(Reg));
                I->addOperand(AsmOperand::createMem(TmpOffsetFromStackPointer, RISCV::SP));
            }
        }

        // Restore stack pointer
        auto *UpdateSP = AsmInstruction::create(RISCV::ADDI, F);
        UpdateSP->addOperand(AsmOperand::createReg(RISCV::SP));
        UpdateSP->addOperand(AsmOperand::createReg(RISCV::SP));
        UpdateSP->addOperand(AsmOperand::createImm(TmpStackFrameSizeInBytes));

        // Return
        AsmInstruction::create(RISCV::RET, F);
    }

    void adjustStackFrame(AsmFunction *AsmFn) override {
        int64_t IncommingArgOffsetFromFP = 0;
        llvm::SmallVector<int64_t> FuncArgOffets;
        llvm::Function *F = AsmFn->F;
        for (unsigned i = RISCV::NumArgRegs, e = F->arg_size(); i < e; ++i) {
            llvm::Argument *Arg = F->getArg(i);
            llvm::Type *Ty = F->getArg(i)->getType();
            uint64_t SizeInBytes = F->getParent()->getDataLayout().getTypeAllocSize(Ty);
            FuncArgOffets.push_back(IncommingArgOffsetFromFP);
            IncommingArgOffsetFromFP += SizeInBytes;
        }

        int64_t LocalFrameObjectOffsetFromFP = -(RISCV::RegisterSize * 2 + StackSizeForCalleeSavedRegs);
        for (auto &StackObj: AsmFn->StackObjects) {
            if (auto *Arg = llvm::dyn_cast_or_null<llvm::Argument>(StackObj.V)) {
                unsigned ArgNo = Arg->getArgNo();
                assert(ArgNo >= RISCV::NumArgRegs);
                StackObj.Offset = FuncArgOffets[ArgNo-RISCV::NumArgRegs];
            }
            if (auto *Alloca = llvm::dyn_cast_or_null<llvm::AllocaInst>(StackObj.V)) {
                StackObj.Offset = LocalFrameObjectOffsetFromFP;
                LocalFrameObjectOffsetFromFP -= StackObj.Size;
            }
        }

        for (auto &p : AsmFn->UsedStackObjectsMap) {
            auto *&I = p.first;
            assert(I->getOpcode() == RISCV::GET_STACKOBJECT_ADDRESS_USER_INST);
            I->setOpcode(RISCV::ADDI);
            I->addOperand(AsmOperand::createReg(RISCV::FP));
            I->addOperand(AsmOperand::createImm(p.second->Offset));
        }

        // for (auto &I : *AsmFn) {
        //     for (unsigned i = 0; i < I.getNumOperands(); ++i) {
        //         AsmOperand &Op = I.getOperand(i);
        //         if (!Op.isMem())
        //             continue;
        //         // Access memory in LocalFrame, SpillFrame, CallFrame
        //         if (Op.Mem.Disp < 0 && Op.Mem.BaseReg == RISCV::FP) {
        //             Op.Mem.Disp -= RISCV::RegisterSize * 2 + StackSizeForCalleeSavedRegs;
        //         }
        //         // Access Incoming arguments passed via stack
        //         if (auto *Arg = llvm::dyn_cast_or_null<llvm::Argument>(Op.Mem.V)) {
        //             unsigned ArgNo = Arg->getArgNo();
        //             assert(ArgNo >= RISCV::NumArgRegs);
        //             if (Op.Mem.BaseReg == Register::NoRegister)
        //                 Op.Mem.BaseReg = RISCV::FP;
        //             Op.Mem.Disp += FuncArgOffets[ArgNo-RISCV::NumArgRegs];
        //         }
        //     }
        // }

        // FIXME: As RISCV integer operand must be in the range [-2048, 2047],
        // This Hack uses register ra to avoid offset of memory operands out-of-range.
        for (auto &I : *AsmFn) {
            for (unsigned i = 0; i < I.getNumOperands(); ++i) {
                AsmOperand &Op = I.getOperand(i);
                if (!Op.isMem())
                    continue;
                if (Op.Mem.Disp < -2048 || Op.Mem.Disp > 2047) {
                    // li ra, Mem.Disp
                    auto *LI = AsmInstruction::create(RISCV::LI, &I);
                    LI->addOperand(AsmOperand::createReg(RISCV::RA));
                    LI->addOperand(AsmOperand::createImm(Op.Mem.Disp));
                    // add ra, ra, Mem.BaseReg
                    auto *AI = AsmInstruction::create(RISCV::ADD, &I);
                    AI->addOperand(AsmOperand::createReg(RISCV::RA));
                    AI->addOperand(AsmOperand::createReg(RISCV::RA));
                    AI->addOperand(AsmOperand::createReg(Op.Mem.BaseReg));
                    // Disp(BaseReg) -> 0(ra)
                    Op.Mem.Disp = 0;
                    Op.Mem.BaseReg = RISCV::RA;
                }
            }
        }
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
