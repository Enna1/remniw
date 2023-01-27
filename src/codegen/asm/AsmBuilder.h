#pragma once

#include "codegen/asm/AsmContext.h"
#include "codegen/asm/AsmFunction.h"
#include "codegen/asm/AsmInstruction.h"
#include "codegen/asm/AsmOperand.h"
#include "codegen/asm/BrgTreeBuilder.h"
#include "codegen/asm/LiveInterval.h"
#include "codegen/asm/Register.h"
#include "codegen/asm/TargetInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include <unordered_map>
#include <vector>

#define DEBUG_TYPE "remniw-AsmBuilder"

namespace remniw {

class AsmBuilder {
private:
    llvm::SmallVector<std::unique_ptr<AsmFunction>> AsmFunctions;
    llvm::SmallVector<uint32_t> CurrentCallInstIndexes;
    AsmFunction *CurrentFunction {nullptr};

public:
    virtual ~AsmBuilder() = default;

    void build(const llvm::SmallVectorImpl<BrgFunction *> &BrgFunctions) {
        for (auto *BrgFunc : BrgFunctions) {
            CurrentCallInstIndexes.clear();
            buildAsmFunction(BrgFunc);
        }
        LLVM_DEBUG({
            for (auto &F : AsmFunctions)
                F->print(llvm::outs());
        });
    }

    virtual const TargetInfo &getTargetInfo() const = 0;

    void buildAsmFunction(const BrgFunction *);

    llvm::SmallVector<std::unique_ptr<AsmFunction>> &getAsmFunctions() {
        return AsmFunctions;
    }
    llvm::SmallVector<uint32_t> &getCurrentCallInstIndexes() {
        return CurrentCallInstIndexes;
    };
    AsmFunction *getCurrentFunction() { return CurrentFunction; }

    virtual AsmOperand::MemOp handleALLOCA(int StackObjectIndex) = 0;

    virtual AsmOperand::RegOp handleLOAD(llvm::Instruction *I, AsmOperand::MemOp Mem) = 0;
    virtual AsmOperand::RegOp handleLOAD(llvm::Instruction *I, AsmOperand::RegOp Reg) = 0;

    virtual void handleRET(llvm::Instruction *I, AsmOperand::RegOp Reg) = 0;
    virtual void handleRET(llvm::Instruction *I, AsmOperand::ImmOp Imm) = 0;

    virtual void handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg,
                             AsmOperand::MemOp Mem) = 0;
    virtual void handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                             AsmOperand::RegOp Reg2) = 0;
    virtual void handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                             AsmOperand::RegOp Reg) = 0;
    virtual void handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                             AsmOperand::MemOp Mem) = 0;
    virtual void handleSTORE(llvm::Instruction *I, AsmOperand::MemOp Mem1,
                             AsmOperand::MemOp Mem2, bool DestIsArgument) = 0;
    virtual void handleSTORE(llvm::Instruction *I, AsmOperand::LabelOp Label,
                             AsmOperand::MemOp Mem) = 0;
    virtual void handleSTORE(llvm::Instruction *I, llvm::Argument *FuncArg,
                             AsmOperand::MemOp Mem) = 0;

    virtual AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I,
                                                  AsmOperand::MemOp Mem,
                                                  AsmOperand::ImmOp Imm) = 0;
    virtual AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I,
                                                  AsmOperand::MemOp Mem,
                                                  AsmOperand::RegOp Reg) = 0;
    virtual AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I,
                                                  AsmOperand::RegOp Reg,
                                                  AsmOperand::ImmOp Imm) = 0;
    virtual AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I,
                                                  AsmOperand::RegOp Reg1,
                                                  AsmOperand::RegOp Reg2) = 0;

    virtual void handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                            AsmOperand::RegOp Reg2) = 0;
    virtual void handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg,
                            AsmOperand::ImmOp Imm) = 0;
    virtual void handleICMP(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                            AsmOperand::RegOp Reg) = 0;

    virtual void handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label1,
                          AsmOperand::LabelOp Label2) = 0;
    virtual void handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label) = 0;

    virtual AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                        AsmOperand::RegOp Reg2) = 0;
    virtual AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                        AsmOperand::ImmOp Imm) = 0;
    virtual AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                        AsmOperand::RegOp Reg) = 0;
    virtual AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                        AsmOperand::ImmOp Imm2) = 0;

    virtual AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                        AsmOperand::RegOp Reg2) = 0;
    virtual AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                        AsmOperand::ImmOp Imm) = 0;
    virtual AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                        AsmOperand::RegOp Reg) = 0;
    virtual AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                        AsmOperand::ImmOp Imm2) = 0;

    virtual AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                        AsmOperand::RegOp Reg2) = 0;
    virtual AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                        AsmOperand::ImmOp Imm) = 0;
    virtual AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                        AsmOperand::RegOp Reg) = 0;
    virtual AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                        AsmOperand::ImmOp Imm2) = 0;

    virtual AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                         AsmOperand::RegOp Reg2) = 0;
    virtual AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                         AsmOperand::ImmOp Imm) = 0;
    virtual AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                         AsmOperand::RegOp Reg) = 0;
    virtual AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                         AsmOperand::ImmOp Imm2) = 0;

    virtual AsmOperand::RegOp handleCALL(llvm::Instruction *I,
                                         AsmOperand::LabelOp Label) = 0;
    virtual AsmOperand::RegOp handleCALL(llvm::Instruction *I, AsmOperand::RegOp Reg) = 0;
    virtual AsmOperand::RegOp handleCALL(llvm::Instruction *I, AsmOperand::MemOp Mem) = 0;

    virtual void handleARG(llvm::Instruction *CI, unsigned ArgNo,
                           AsmOperand::RegOp Reg) = 0;
    virtual void handleARG(llvm::Instruction *CI, unsigned ArgNo,
                           AsmOperand::ImmOp Imm) = 0;
    virtual void handleARG(llvm::Instruction *CI, unsigned ArgNo,
                           AsmOperand::MemOp Mem) = 0;
    virtual void handleARG(llvm::Instruction *CI, unsigned ArgNo,
                           AsmOperand::LabelOp Label) = 0;

    virtual void handleLABEL(AsmOperand::LabelOp Label) = 0;

    virtual AsmOperand::MemOp getStackObjectAddress(int StackObjectIndex) = 0;

    void updateRegLiveRanges(uint32_t Reg) {
        uint32_t StartPoint = static_cast<uint32_t>(CurrentFunction->size());
        uint32_t EndPoint = StartPoint + 1;
        bool UsedAcrossCall = false;
        std::unordered_map<uint32_t, remniw::LiveRanges> &CurrentRegLiveRangesMap =
            CurrentFunction->getRegLiveRangesMap();
        // For virtual registers, we simply do not consider lifetime holes.
        // The lifetime interval of virtual register is the segment of the program that
        // starts where the virtual register is first live in the static linear order
        // of the code and ends where it is last live.
        // So when we use `struct LiveRanges` to represent the lifetime of virtual
        // registers, the `struct LiveRanges` only consists of one `struct LiveRange`.
        if (Register::isVirtualRegister(Reg)) {
            if (CurrentRegLiveRangesMap.count(Reg)) {
                CurrentRegLiveRangesMap[Reg].Ranges.back().EndPoint = EndPoint;
                StartPoint = CurrentRegLiveRangesMap[Reg].Ranges.back().StartPoint;
            } else {
                CurrentRegLiveRangesMap[Reg].Ranges.push_back(
                    {StartPoint, EndPoint, false});
            }

            if (std::find_if(CurrentCallInstIndexes.begin(), CurrentCallInstIndexes.end(),
                             [&](uint32_t Index) {
                                 return StartPoint <= Index && Index < EndPoint;
                             }) != CurrentCallInstIndexes.end())
                CurrentRegLiveRangesMap[Reg].Ranges.back().UsedAcrossCall = true;

            LLVM_DEBUG({
                for (const auto &Range : CurrentRegLiveRangesMap[Reg].Ranges) {
                    llvm::outs() << "VirtualRegister: " << Reg << ", LiveRange: ";
                    Range.print(llvm::outs());
                    llvm::outs() << "\n";
                }
            });
        }
        // For physical register, we consider lifetime holes.
        // For the segment of the program that starts where the physical register
        // is first live in the static linear order of the code and ends where
        // it is last live, there are one or more holes during which physical register
        // is not used.
        // So when we use `struct LiveRanges` to represent the lifetime of physical
        // registers, the `struct LiveRanges` usually consists of multiple
        // `struct LiveRange`s.
        else if (Register::isPhysicalRegister(Reg)) {
            if (!CurrentRegLiveRangesMap[Reg].Ranges.empty()) {
                auto &LastActiveRange = CurrentRegLiveRangesMap[Reg].Ranges.back();
                if (LastActiveRange.StartPoint == StartPoint &&
                    LastActiveRange.EndPoint == EndPoint) {
                    // Do nothing
                } else if (LastActiveRange.EndPoint == StartPoint) {
                    LastActiveRange.EndPoint = EndPoint;
                    StartPoint = LastActiveRange.StartPoint;
                } else {
                    CurrentRegLiveRangesMap[Reg].Ranges.push_back(
                        {StartPoint, EndPoint, false});
                }
            } else {
                CurrentRegLiveRangesMap[Reg].Ranges.push_back(
                    {StartPoint, EndPoint, false});
            }

            if (std::find_if(CurrentCallInstIndexes.begin(), CurrentCallInstIndexes.end(),
                             [&](uint32_t Index) {
                                 return StartPoint <= Index && Index < EndPoint;
                             }) != CurrentCallInstIndexes.end())
                CurrentRegLiveRangesMap[Reg].Ranges.back().UsedAcrossCall = true;
            LLVM_DEBUG({
                for (const auto &Range : CurrentRegLiveRangesMap[Reg].Ranges) {
                    llvm::outs() << "PhysicalRegister: " << Reg << ", LiveRange: ";
                    Range.print(llvm::outs());
                    llvm::outs() << "\n";
                }
            });
        }
    }

    void updateAsmOperandLiveRanges(const AsmOperand &Op) {
        if (Op.isReg()) {
            updateRegLiveRanges(Op.getReg());
        }
        if (Op.isMem()) {
            uint32_t MemBaseReg = Op.getMemBaseReg();
            if (MemBaseReg != Register::NoRegister) {
                updateRegLiveRanges(MemBaseReg);
            }
            uint32_t MemIndexReg = Op.getMemIndexReg();
            if (MemIndexReg != Register::NoRegister) {
                updateRegLiveRanges(MemIndexReg);
            }
        }
    }

    void updateAsmOperandLiveRanges(const AsmOperand::RegOp &Reg) {
        updateRegLiveRanges(Reg.RegNo);
    }

    void updateAsmOperandLiveRanges(const AsmOperand::MemOp &Mem) {
        updateRegLiveRanges(Mem.BaseReg);
        if (Mem.IndexReg != Register::NoRegister)
            updateRegLiveRanges(Mem.IndexReg);
    }
};

using AsmBuilderPtr = AsmBuilder *;
using LLVMInstructionPtr = llvm::Instruction *;
}  // namespace remniw

#undef DEBUG_TYPE
