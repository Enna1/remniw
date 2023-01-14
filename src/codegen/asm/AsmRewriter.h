#pragma once

#include "codegen/asm/AsmFunction.h"
#include "codegen/asm/RegisterAllocator.h"
#include "codegen/asm/TargetInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Alignment.h"

namespace remniw {

class AsmRewriter {
private:
    const TargetInfo &TI;
    LinearScanRegisterAllocator LSRA;
    AsmFunction *CurrentFunction;

protected:
    uint32_t NumSpilledReg;
    uint32_t NumReversedStackSlotForReg;
    uint32_t MaxNumReversedStackSlotForReg;

public:
    AsmRewriter(const TargetInfo &TI): TI(TI), LSRA(TI) {}
    virtual ~AsmRewriter() = default;

    void rewrite(llvm::SmallVector<std::unique_ptr<AsmFunction>> &AsmFunctions) {
        for (auto &F : AsmFunctions) {
            if (F->empty())
                continue;

            CurrentFunction = F.get();
            NumSpilledReg = 0;
            NumReversedStackSlotForReg = 0;
            MaxNumReversedStackSlotForReg = 0;

            // Register allocation, assign physical registers or spilled stack slots to
            // virtual registers.
            LSRA.doRegAlloc(CurrentFunction->RegLiveRangesMap);
            const llvm::DenseMap<uint32_t, uint32_t> &VirtToAllocRegMap =
                LSRA.getVirtRegToAllocatedRegMap();
            NumSpilledReg = LSRA.getSpilledRegCount();

            // Rewrite virtual registers to physical registers or spilled stack slots.
            // First, if any virtual regsiters are assigned to physical registers, rewrite
            // instructions.
            for (auto &AsmInst : *CurrentFunction)
                rewriteAsmInstVirtRegToPhysReg(&AsmInst, VirtToAllocRegMap);
            // At this point, if there is any virtual register still used in instructions,
            // the virtual register must be spilled to stack slot.
            for (auto &AsmInst : *CurrentFunction)
                rewriteAsmInstSpilledRegToStackSlot(&AsmInst, VirtToAllocRegMap);

            // Insert prologue and epilogue.
            llvm::SmallVector<uint32_t> UsedCalleeSavedRegs;
            for (auto p : VirtToAllocRegMap) {
                if (TI.isCalleeSavedRegister(p.second))
                    UsedCalleeSavedRegs.push_back(p.second);
            }
            insertPrologue(CurrentFunction, UsedCalleeSavedRegs);
            insertEpilogue(CurrentFunction, UsedCalleeSavedRegs);
        }
    }

    void rewriteAsmOperandVirtRegToPhysReg(
        AsmOperand &AsmOp, const llvm::DenseMap<uint32_t, uint32_t> &VirtToAllocRegMap) {
        if (AsmOp.isVirtReg()) {
            assert(VirtToAllocRegMap.count(AsmOp.Reg.RegNo));
            uint32_t AllocatedReg = VirtToAllocRegMap.lookup(AsmOp.Reg.RegNo);
            if (Register::isPhysicalRegister(AllocatedReg)) {
                AsmOp.Reg.RegNo = VirtToAllocRegMap.lookup(AsmOp.Reg.RegNo);
            }
        }
        if (AsmOp.isMem()) {
            if (Register::isVirtualRegister(AsmOp.Mem.BaseReg)) {
                assert(VirtToAllocRegMap.count(AsmOp.Mem.BaseReg));
                uint32_t AllocatedReg = VirtToAllocRegMap.lookup(AsmOp.Mem.BaseReg);
                if (Register::isPhysicalRegister(AllocatedReg)) {
                    AsmOp.Mem.BaseReg = VirtToAllocRegMap.lookup(AsmOp.Mem.BaseReg);
                }
            }
            if (Register::isVirtualRegister(AsmOp.Mem.IndexReg)) {
                assert(VirtToAllocRegMap.count(AsmOp.Mem.IndexReg));
                uint32_t AllocatedReg = VirtToAllocRegMap.lookup(AsmOp.Mem.IndexReg);
                if (Register::isPhysicalRegister(AllocatedReg)) {
                    AsmOp.Mem.IndexReg = VirtToAllocRegMap.lookup(AsmOp.Mem.IndexReg);
                }
            }
        }
    }

    int64_t getStackSlotOffsetForSpilledReg(uint32_t RegNo) {
        assert(Register::isStackSlot(RegNo) && "Must be StackSlot");
        uint32_t StackSlotIndex = Register::stackSlot2Index(RegNo);
        return -(CurrentFunction->StackSizeInBytes +
                 TI.getRegisterSize() * (StackSlotIndex + 1));
    }

    int64_t getReservedStackSlotOffsetForReg() {
        NumReversedStackSlotForReg++;
        int64_t Offset =
            -(CurrentFunction->StackSizeInBytes + TI.getRegisterSize() * NumSpilledReg +
              TI.getRegisterSize() * NumReversedStackSlotForReg);
        return Offset;
    }

private:
    virtual void rewriteAsmInstVirtRegToPhysReg(
        AsmInstruction *I,
        const llvm::DenseMap<uint32_t, uint32_t> &VirtToAllocRegMap) = 0;

    virtual void rewriteAsmInstSpilledRegToStackSlot(
        AsmInstruction *I,
        const llvm::DenseMap<uint32_t, uint32_t> &VirtToAllocRegMap) = 0;

    virtual void insertPrologue(AsmFunction *F,
                                llvm::SmallVectorImpl<uint32_t> &UsedCalleeSavedRegs) = 0;

    virtual void insertEpilogue(AsmFunction *F,
                                llvm::SmallVectorImpl<uint32_t> &UsedCalleeSavedRegs) = 0;

    virtual void getUsedRegisters(AsmInstruction *I,
                                  llvm::SmallVectorImpl<uint32_t> &UsedRegs) = 0;

    virtual uint32_t getAvailableRegister(AsmInstruction *I,
                                          llvm::SmallVectorImpl<uint32_t> &UsedRegs) = 0;
};

}  // namespace remniw
