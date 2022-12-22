#pragma once

#include "AsmContext.h"
#include "AsmFunction.h"
#include "AsmInstruction.h"
#include "BrgTreeBuilder.h"
#include "LiveInterval.h"
#include "Register.h"
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

#define DEBUG_TYPE "remniw-asmbuilder"

namespace remniw {

class AsmBuilder {
private:
    AsmContext &AsmCtx;
    llvm::SmallVector<AsmFunction *> AsmFunctions;
    llvm::SmallVector<uint32_t> CurrentCallInstIndexs;
    AsmFunction *CurrentFunction;

public:
    AsmBuilder(AsmContext &AsmCtx,
               const llvm::SmallVectorImpl<BrgFunction *> &BrgFunctions):
        AsmCtx(AsmCtx),
        CurrentFunction(nullptr) {
        for (auto *BrgFunc : BrgFunctions) {
            CurrentCallInstIndexs.clear();
            buildAsmFunction(BrgFunc);
        }
    }

    ~AsmBuilder() {
        for (auto *F : AsmFunctions)
            delete F;
    }

    void buildAsmFunction(const BrgFunction *);

    llvm::SmallVector<AsmFunction *> &getAsmFunctions() { return AsmFunctions; }

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

            if (std::find_if(CurrentCallInstIndexs.begin(), CurrentCallInstIndexs.end(),
                             [&](uint32_t Index) {
                                 return StartPoint <= Index && Index < EndPoint;
                             }) != CurrentCallInstIndexs.end())
                CurrentRegLiveRangesMap[Reg].Ranges.back().UsedAcrossCall = true;

            for (const auto &Range : CurrentRegLiveRangesMap[Reg].Ranges) {
                LLVM_DEBUG({
                    llvm::outs() << "VirtualRegister " << Reg << " LiveRange: ";
                    Range.print(llvm::outs());
                    llvm::outs() << "\n";
                });
            }
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

            if (std::find_if(CurrentCallInstIndexs.begin(), CurrentCallInstIndexs.end(),
                             [&](uint32_t Index) {
                                 return StartPoint <= Index && Index < EndPoint;
                             }) != CurrentCallInstIndexs.end())
                CurrentRegLiveRangesMap[Reg].Ranges.back().UsedAcrossCall = true;

            for (const auto &Range : CurrentRegLiveRangesMap[Reg].Ranges) {
                LLVM_DEBUG({
                    llvm::outs() << "PhysicalRegiste " << Reg << " LiveRange: ";
                    Range.print(llvm::outs());
                    llvm::outs() << "\n";
                });
            }
        }
    }

    void updateAsmOperandLiveRanges(const AsmOperand &Op) {
        LLVM_DEBUG({ llvm::outs() << "updateAsmOperandLiveRanges " << &Op << "\n"; });
        if (Op.isReg()) {
            updateRegLiveRanges(Op.getReg());
        }
        if (Op.isMem()) {
            uint32_t MemBaseReg = Op.getMemBaseReg();
            if (MemBaseReg != Register::RBP) {
                updateRegLiveRanges(MemBaseReg);
            }
            uint32_t MemIndexReg = Op.getMemIndexReg();
            if (MemIndexReg != Register::NoRegister) {
                updateRegLiveRanges(MemIndexReg);
            }
        }
    }
    
    void createMov(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst) {
        updateAsmOperandLiveRanges(*Src);
        updateAsmOperandLiveRanges(*Dst);
        auto *I = AsmMovInst::create(std::move(Src), std::move(Dst), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createLea(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst) {
        updateAsmOperandLiveRanges(*Src);
        updateAsmOperandLiveRanges(*Dst);
        auto *I = AsmLeaInst::create(std::move(Src), std::move(Dst), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createCmp(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst) {
        updateAsmOperandLiveRanges(*Src);
        updateAsmOperandLiveRanges(*Dst);
        auto *I = AsmCmpInst::create(std::move(Src), std::move(Dst), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createJmp(AsmJmpInst::JmpKindTy JmpKind, std::unique_ptr<AsmOperand> Op) {
        updateAsmOperandLiveRanges(*Op);
        auto *I = AsmJmpInst::create(JmpKind, std::move(Op), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createAdd(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst) {
        updateAsmOperandLiveRanges(*Src);
        updateAsmOperandLiveRanges(*Dst);
        auto *I = AsmAddInst::create(std::move(Src), std::move(Dst), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createSub(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst) {
        updateAsmOperandLiveRanges(*Src);
        updateAsmOperandLiveRanges(*Dst);
        auto *I = AsmSubInst::create(std::move(Src), std::move(Dst), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createImul(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst) {
        updateAsmOperandLiveRanges(*Src);
        updateAsmOperandLiveRanges(*Dst);
        auto *I = AsmImulInst::create(std::move(Src), std::move(Dst), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createIdiv(std::unique_ptr<AsmOperand> Op) {
        updateAsmOperandLiveRanges(*Op);
        updateRegLiveRanges(Register::RAX);
        updateRegLiveRanges(Register::RDX);
        auto *I = AsmIdivInst::create(std::move(Op), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createCqto() {
        updateRegLiveRanges(Register::RAX);
        updateRegLiveRanges(Register::RDX);
        auto *I = AsmCqtoInst::create(CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createCall(std::unique_ptr<AsmOperand> Callee, bool DirectCall, unsigned NumArgs) {
        updateAsmOperandLiveRanges(*Callee);
        auto *I = AsmCallInst::create(std::move(Callee), DirectCall, NumArgs, CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
        CurrentCallInstIndexs.push_back(CurrentFunction->size());
    }

    void createXor(std::unique_ptr<AsmOperand> Src, std::unique_ptr<AsmOperand> Dst) {
        updateAsmOperandLiveRanges(*Src);
        updateAsmOperandLiveRanges(*Dst);
        auto *I = AsmXorInst::create(std::move(Src), std::move(Dst), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }

    void createLabel(std::unique_ptr<AsmOperand> LabelOp) {
        auto *I = AsmLabelInst::create(std::move(LabelOp), CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
    }
};

using AsmBuilderPtr = AsmBuilder *;

}  // namespace remniw

#undef DEBUG_TYPE
