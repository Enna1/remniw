#pragma once

#include "AsmFunction.h"
#include "RegisterAllocator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Alignment.h"

namespace remniw {

class AsmRewriter {
private:
    llvm::SmallVector<AsmFunction *> AsmFunctions;
    AsmFunction *CurrentFunction;
    uint32_t NumSpilledReg;
    uint32_t NumReversedStackSlotForReg;
    uint32_t MaxNumReversedStackSlotForReg;

public:
    AsmRewriter(llvm::SmallVector<AsmFunction *> AsmFuncs): AsmFunctions(AsmFuncs) {
        // for (auto *AsmFunc : AsmFunctions) {
        //     if (AsmFunc->empty())
        //         continue;
        //     CurrentFunction = AsmFunc;
        //     NumSpilledReg = 0;
        //     NumReversedStackSlotForReg = 0;
        //     MaxNumReversedStackSlotForReg = 0;
        //     llvm::SmallVector<uint32_t, 8> UsedCalleeSavedRegisters;
        //     doRegAlloc(UsedCalleeSavedRegisters);
        //     insertPrologue(AsmFunc, UsedCalleeSavedRegisters);
        //     insertEpilogue(AsmFunc, UsedCalleeSavedRegisters);
        // }
    }

    llvm::SmallVector<AsmFunction *> &getAsmFunctions() { return AsmFunctions; }

private:
    // void doRegAlloc(llvm::SmallVectorImpl<uint32_t> &UsedCalleeSavedRegisters) {
    //     LinearScanRegisterAllocator LSRA(CurrentFunction->RegLiveRangesMap);
    //     LSRA.LinearScan();
    //     std::unordered_map<uint32_t, uint32_t> &VirtRegToAllocatedRegMap =
    //         LSRA.getVirtRegToAllocatedRegMap();
    //     NumSpilledReg = LSRA.getSpilledRegCount();

    //     for (auto &AsmInst : *CurrentFunction)
    //         rewriteAsmInstVirtRegToPhysReg(&AsmInst, VirtRegToAllocatedRegMap);

    //     for (auto &AsmInst : *CurrentFunction)
    //         rewriteAsmInstSpilledRegToStackSlot(&AsmInst, VirtRegToAllocatedRegMap);

    //     for (auto p : VirtRegToAllocatedRegMap) {
    //         if (Register::isCalleeSavedRegister(p.second))
    //             UsedCalleeSavedRegisters.push_back(p.second);
    //     }
    // }

    // void rewriteAsmInstSpilledRegToStackSlot(
    //     AsmInstruction *AsmInst,
    //     std::unordered_map<uint32_t, uint32_t> &VirtRegToAllocatedRegMap) {
    //     size_t NumOperands = AsmInst->getNumOperands();
    //     assert(NumOperands <= 2 && "The #operands of AsmInst must <= 2");
    //     NumReversedStackSlotForReg = 0;
    //     llvm::SmallVector<uint32_t, 4> UsedRegs;
    //     getUsedRegisters(AsmInst, UsedRegs);
    //     if (NumOperands == 1) {
    //         rewriteAsmOperandSpilledRegToStackSlot(AsmInst, 0, /*IsDst=*/true,
    //                                                VirtRegToAllocatedRegMap, UsedRegs);
    //     }
    //     if (NumOperands == 2) {
    //         rewriteAsmOperandSpilledRegToStackSlot(AsmInst, 0, /*IsDst=*/false,
    //                                                VirtRegToAllocatedRegMap, UsedRegs);
    //         rewriteAsmOperandSpilledRegToStackSlot(AsmInst, 1, /*IsDst=*/true,
    //                                                VirtRegToAllocatedRegMap, UsedRegs);
    //     }
    //     if (NumReversedStackSlotForReg > MaxNumReversedStackSlotForReg)
    //         MaxNumReversedStackSlotForReg = NumReversedStackSlotForReg;
    // }

    // void rewriteAsmOperandSpilledRegToStackSlot(
    //     AsmInstruction *AsmInst, unsigned OpNo, bool IsDst,
    //     std::unordered_map<uint32_t, uint32_t> &VirtRegToAllocatedRegMap,
    //     llvm::SmallVectorImpl<uint32_t> &UsedRegs) {
    //     AsmOperand &AsmOp = AsmInst->getOperand(OpNo);
    //     if (AsmOp.isVirtReg()) {
    //         assert(VirtRegToAllocatedRegMap.count(AsmOp.Reg.RegNo));
    //         uint32_t AllocatedReg = VirtRegToAllocatedRegMap[AsmOp.Reg.RegNo];
    //         assert(Register::isStackSlot(AllocatedReg));
    //         int64_t StackSlotOffset = getStackSlotOffsetForSpilledReg(AllocatedReg);
    //         /*
    //         // FIXME: optimize generated spilled code
    //         if (OpNo == AsmInst->getNumOperands() - 1 &&
    //             AsmInst->getOperand(AsmInst->getNumOperands() - 1)->isPhysReg()) {
    //             AsmInst->setOperand(OpNo, AsmOperand::createMem(StackSlotOffset));
    //         } else
    //         */
    //         {
    //             uint32_t AvailReg = getAvailableRegister(AsmInst, UsedRegs);
    //             assert(AvailReg != Register::NoRegister);
    //             UsedRegs.push_back(AvailReg);
    //             int64_t ReservedStackSlotOffset = getReservedStackSlotOffsetForReg();
    //             AsmMovInst::create(AsmOperand::createReg(AvailReg),
    //                                AsmOperand::createMem(ReservedStackSlotOffset),
    //                                AsmInst);
    //             AsmMovInst::create(AsmOperand::createMem(StackSlotOffset),
    //                                AsmOperand::createReg(AvailReg), AsmInst);
    //             AsmInst->setOperand(OpNo, AsmOperand::createReg(AvailReg));
    //             if (auto *NextInst = AsmInst->getNextNode()) {
    //                 AsmMovInst::create(AsmOperand::createReg(AvailReg),
    //                                    AsmOperand::createMem(StackSlotOffset), NextInst);
    //                 AsmMovInst::create(AsmOperand::createMem(ReservedStackSlotOffset),
    //                                    AsmOperand::createReg(AvailReg), NextInst);
    //             } else {
    //                 AsmMovInst::create(AsmOperand::createReg(AvailReg),
    //                                    AsmOperand::createMem(StackSlotOffset),
    //                                    AsmInst->getParent());
    //                 AsmMovInst::create(AsmOperand::createMem(ReservedStackSlotOffset),
    //                                    AsmOperand::createReg(AvailReg),
    //                                    AsmInst->getParent());
    //             }
    //         }
    //     }

    //     if (AsmOp.isMem()) {
    //         if (Register::isVirtualRegister(AsmOp.Mem.BaseReg)) {
    //             assert(VirtRegToAllocatedRegMap.count(AsmOp.Mem.BaseReg));
    //             uint32_t AllocatedReg = VirtRegToAllocatedRegMap[AsmOp.Mem.BaseReg];
    //             assert(Register::isStackSlot(AllocatedReg));
    //             int64_t StackSlotOffset = getStackSlotOffsetForSpilledReg(AllocatedReg);
    //             uint32_t AvailReg = getAvailableRegister(AsmInst, UsedRegs);
    //             UsedRegs.push_back(AvailReg);
    //             int64_t ReservedStackSlotOffset = getReservedStackSlotOffsetForReg();
    //             AsmMovInst::create(AsmOperand::createReg(AvailReg),
    //                                AsmOperand::createMem(ReservedStackSlotOffset),
    //                                AsmInst);
    //             AsmMovInst::create(AsmOperand::createMem(StackSlotOffset),
    //                                AsmOperand::createReg(AvailReg), AsmInst);
    //             AsmOp.Mem.BaseReg = AvailReg;
    //             if (auto *NextInst = AsmInst->getNextNode()) {
    //                 AsmMovInst::create(AsmOperand::createReg(AvailReg),
    //                                    AsmOperand::createMem(StackSlotOffset), NextInst);
    //                 AsmMovInst::create(AsmOperand::createMem(ReservedStackSlotOffset),
    //                                    AsmOperand::createReg(AvailReg), NextInst);
    //             } else {
    //                 AsmMovInst::create(AsmOperand::createReg(AvailReg),
    //                                    AsmOperand::createMem(StackSlotOffset),
    //                                    AsmInst->getParent());
    //                 AsmMovInst::create(AsmOperand::createMem(ReservedStackSlotOffset),
    //                                    AsmOperand::createReg(AvailReg),
    //                                    AsmInst->getParent());
    //             }
    //         }

    //         if (Register::isVirtualRegister(AsmOp.Mem.IndexReg)) {
    //             assert(VirtRegToAllocatedRegMap.count(AsmOp->Mem.IndexReg));
    //             uint32_t AllocatedReg = VirtRegToAllocatedRegMap[AsmOp.Mem.IndexReg];
    //             assert(Register::isStackSlot(AllocatedReg));
    //             int64_t StackSlotOffset = getStackSlotOffsetForSpilledReg(AllocatedReg);
    //             uint32_t AvailReg = getAvailableRegister(AsmInst, UsedRegs);
    //             UsedRegs.push_back(AvailReg);
    //             AsmOperand::createReg(AvailReg);
    //             int64_t ReservedStackSlotOffset = getReservedStackSlotOffsetForReg();
    //             AsmMovInst::create(AsmOperand::createReg(AvailReg),
    //                                AsmOperand::createMem(ReservedStackSlotOffset),
    //                                AsmInst);
    //             AsmMovInst::create(AsmOperand::createMem(StackSlotOffset),
    //                                AsmOperand::createReg(AvailReg), AsmInst);
    //             AsmOp.Mem.IndexReg = AvailReg;
    //             if (auto *NextInst = AsmInst->getNextNode()) {
    //                 AsmMovInst::create(AsmOperand::createReg(AvailReg),
    //                                    AsmOperand::createMem(StackSlotOffset), NextInst);
    //                 AsmMovInst::create(AsmOperand::createMem(ReservedStackSlotOffset),
    //                                    AsmOperand::createReg(AvailReg), NextInst);
    //             } else {
    //                 AsmMovInst::create(AsmOperand::createReg(AvailReg),
    //                                    AsmOperand::createMem(StackSlotOffset),
    //                                    AsmInst->getParent());
    //                 AsmMovInst::create(AsmOperand::createMem(ReservedStackSlotOffset),
    //                                    AsmOperand::createReg(AvailReg),
    //                                    AsmInst->getParent());
    //             }
    //         }
    //     }
    // }

    // int64_t getStackSlotOffsetForSpilledReg(uint32_t RegNo) {
    //     assert(Register::isStackSlot(RegNo) && "Must be StackSlot");
    //     uint32_t StackSlotIndex = Register::stackSlot2Index(RegNo);
    //     return -(CurrentFunction->StackSizeInBytes + 8 * (StackSlotIndex + 1));
    // }

    // int64_t getReservedStackSlotOffsetForReg() {
    //     NumReversedStackSlotForReg++;
    //     int64_t Offset = -(CurrentFunction->StackSizeInBytes + 8 * NumSpilledReg +
    //                        8 * NumReversedStackSlotForReg);
    //     return Offset;
    // }

    // void insertPrologue(remniw::AsmFunction *AsmFunc,
    //                     llvm::SmallVectorImpl<uint32_t> &UsedCalleeSavedRegisters) {
    //     AsmInstruction *InsertBefore = &AsmFunc->front();
    //     if (AsmFunc->FuncName != "main") {
    //         for (uint32_t Reg : UsedCalleeSavedRegisters) {
    //             AsmPushInst::create(AsmOperand::createReg(Reg), InsertBefore);
    //         }
    //     }
    //     AsmPushInst::create(AsmOperand::createReg(Register::RBP), InsertBefore);
    //     AsmMovInst::create(AsmOperand::createReg(Register::RSP),
    //                        AsmOperand::createReg(Register::RBP), InsertBefore);
    //     int64_t NeededStackSizeInBytes = AsmFunc->StackSizeInBytes + 8 * NumSpilledReg +
    //                                      8 * MaxNumReversedStackSlotForReg;
    //     int64_t TotalStackFrameSizeInBytes =
    //         NeededStackSizeInBytes + 8 /*push $rbp*/ + 8 /*return address*/;
    //     if (AsmFunc->FuncName != "main")
    //         TotalStackFrameSizeInBytes += UsedCalleeSavedRegisters.size() * 8;
    //     // x86-64 / AMD64 System V ABI requires 16-byte stack alignment
    //     if (TotalStackFrameSizeInBytes % 16)
    //         NeededStackSizeInBytes += 16 - TotalStackFrameSizeInBytes % 16;
    //     AsmSubInst::create(AsmOperand::createImm(NeededStackSizeInBytes),
    //                        AsmOperand::createReg(Register::RSP), InsertBefore);
    // }

    // void insertEpilogue(remniw::AsmFunction *AsmFunc,
    //                     llvm::SmallVectorImpl<uint32_t> &UsedCalleeSavedRegisters) {
    //     AsmMovInst::create(AsmOperand::createReg(Register::RBP),
    //                        AsmOperand::createReg(Register::RSP), AsmFunc);
    //     AsmPopInst::create(AsmOperand::createReg(Register::RBP), AsmFunc);
    //     if (AsmFunc->FuncName != "main") {
    //         for (auto i = UsedCalleeSavedRegisters.rbegin(),
    //                   e = UsedCalleeSavedRegisters.rend();
    //              i != e; ++i) {
    //             AsmPopInst::create(AsmOperand::createReg(*i), AsmFunc);
    //         }
    //     }
    //     AsmRetInst::create(AsmFunc);
    // }

    // void rewriteAsmOperandVirtRegToPhysReg(
    //     AsmOperand *AsmOp,
    //     std::unordered_map<uint32_t, uint32_t> &VirtRegToAllocatedRegMap) {
    //     if (AsmOp->isVirtReg()) {
    //         assert(VirtRegToAllocatedRegMap.count(AsmOp->Reg.RegNo));
    //         uint32_t AllocatedReg = VirtRegToAllocatedRegMap[AsmOp->Reg.RegNo];
    //         if (Register::isPhysicalRegister(AllocatedReg)) {
    //             AsmOp->Reg.RegNo = VirtRegToAllocatedRegMap[AsmOp->Reg.RegNo];
    //         }
    //     }
    //     if (AsmOp->isMem()) {
    //         if (Register::isVirtualRegister(AsmOp->Mem.BaseReg)) {
    //             assert(VirtRegToAllocatedRegMap.count(AsmOp->Mem.BaseReg));
    //             uint32_t AllocatedReg = VirtRegToAllocatedRegMap[AsmOp->Mem.BaseReg];
    //             if (Register::isPhysicalRegister(AllocatedReg)) {
    //                 AsmOp->Mem.BaseReg = VirtRegToAllocatedRegMap[AsmOp->Mem.BaseReg];
    //             }
    //         }
    //         if (Register::isVirtualRegister(AsmOp->Mem.IndexReg)) {
    //             assert(VirtRegToAllocatedRegMap.count(AsmOp->Mem.IndexReg));
    //             uint32_t AllocatedReg = VirtRegToAllocatedRegMap[AsmOp->Mem.IndexReg];
    //             if (Register::isPhysicalRegister(AllocatedReg)) {
    //                 AsmOp->Mem.IndexReg = VirtRegToAllocatedRegMap[AsmOp->Mem.IndexReg];
    //             }
    //         }
    //     }
    // }

    // void rewriteAsmInstVirtRegToPhysReg(
    //     AsmInstruction *AsmInst,
    //     std::unordered_map<uint32_t, uint32_t> &VirtRegToAllocatedRegMap) {
    //     switch (AsmInst->getInstKind()) {
    //     case AsmInstruction::Mov:
    //     case AsmInstruction::Lea:
    //     case AsmInstruction::Cmp:
    //     case AsmInstruction::Add:
    //     case AsmInstruction::Sub:
    //     case AsmInstruction::Imul:
    //     case AsmInstruction::Xor:
    //         rewriteAsmOperandVirtRegToPhysReg(AsmInst->getOperand(0),
    //                                           VirtRegToAllocatedRegMap);
    //         rewriteAsmOperandVirtRegToPhysReg(AsmInst->getOperand(1),
    //                                           VirtRegToAllocatedRegMap);
    //         break;
    //     case AsmInstruction::Jmp:
    //     case AsmInstruction::Idiv:
    //     case AsmInstruction::Call:
    //         rewriteAsmOperandVirtRegToPhysReg(AsmInst->getOperand(0),
    //                                           VirtRegToAllocatedRegMap);
    //         break;
    //     case AsmInstruction::Cqto:
    //     case AsmInstruction::Label: break;
    //     default: llvm_unreachable("Invalid AsmInstruction");
    //     }
    // }

    // void getUsedRegisters(AsmInstruction *AsmInst,
    //                       llvm::SmallVectorImpl<uint32_t> &UsedRegs) {
    //     auto InstKind = AsmInst->getInstKind();
    //     if (InstKind == AsmInstruction::Idiv || InstKind == AsmInstruction::Cqto) {
    //         UsedRegs.push_back(Register::RAX);
    //         UsedRegs.push_back(Register::RDX);
    //     }
    //     if (InstKind == AsmInstruction::Call) {
    //         auto *CI = llvm::cast<AsmCallInst>(AsmInst);
    //         UsedRegs.push_back(Register::RAX);
    //         for (unsigned i = 0; i < CI->getNumArgs(); ++i)
    //             UsedRegs.push_back(Register::ArgRegs[i]);
    //     }
    //     for (unsigned i = 0; i < AsmInst->getNumOperands(); ++i) {
    //         auto &AsmOp = AsmInst->getOperand(i);
    //         if (AsmOp.isPhysReg()) {
    //             UsedRegs.push_back(AsmOp.Reg.RegNo);
    //         }
    //         if (AsmOp.isMem()) {
    //             if (Register::isPhysicalRegister(AsmOp.Mem.BaseReg))
    //                 UsedRegs.push_back(AsmOp.Mem.BaseReg);
    //             if (Register::isPhysicalRegister(AsmOp.Mem.IndexReg))
    //                 UsedRegs.push_back(AsmOp.Mem.IndexReg);
    //         }
    //     }
    // }

    // uint32_t getAvailableRegister(AsmInstruction *AsmInst,
    //                               llvm::SmallVectorImpl<uint32_t> &UsedRegs) {
    //     for (uint32_t Reg = Register::RAX; Reg <= Register::R15; ++Reg) {
    //         if (std::find(UsedRegs.begin(), UsedRegs.end(), Reg) != UsedRegs.end())
    //             continue;
    //         return Reg;
    //     }
    //     return Register::NoRegister;
    // }
};

}  // namespace remniw
