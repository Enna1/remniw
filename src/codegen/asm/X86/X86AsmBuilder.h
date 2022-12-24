#pragma once

#include "AsmBuilder.h"
#include "AsmOperand.h"

namespace remniw {

class X86AsmBuilder: public AsmBuilder {
public:
    AsmOperand::RegOp handleLOAD(llvm::Instruction *I, AsmOperand::MemOp Mem) override {
        uint32_t VirtReg = Register::createVirtReg();
        auto DstReg = AsmOperand::createReg(VirtReg);
        createMov(Mem, DstReg);
        return DstReg;
    }

    AsmOperand::RegOp handleLOAD(llvm::Instruction *I, AsmOperand::RegOp Reg) override {
        uint32_t VirtReg = Register::createVirtReg();
        auto DstReg = AsmOperand::createReg(VirtReg);
        Builder->createMov(AsmOperand::createMem(0, Reg), DstReg);
        return DstReg;
    }

    void handleRET(llvm::Instruction *I, AsmOperand::RegOp Reg) override {
        createMov(Reg, AsmOperand::createReg(X86::RAX));
    }

    void handleRET(llvm::Instruction *I, AsmOperand::ImmOp Imm) override {
        createMov(Imm, AsmOperand::createReg(X86::RAX));
    }

    void handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg,
                     AsmOperand::MemOp Mem) override {
        createMov(Reg, Mem);
    }

    void handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                     AsmOperand::RegOp Reg2) override {
        createMov(Reg1, AsmOperand::createMem(0, Reg2));
    }

    void handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                     AsmOperand::RegOp Reg) override {
        createMov(Imm, AsmOperand::createMem(0, Reg));
    }

    void handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                     AsmOperand::MemOp Mem) override {
        createMov(Imm, Mem);
    }

    void handleSTORE(llvm::Instruction *I, AsmOperand::MemOp Mem1, AsmOperand::MemOp Mem2,
                     bool DestIsArgument) override {
        uint32_t VirtReg = Register::createVirtReg();
        if (DestIsArgument) {
            createMov(Mem1, AsmOperand::createReg(VirtReg));
            createMov(AsmOperand::createReg(VirtReg), Mem2);
        } else {
            createLea(Mem1, AsmOperand::createReg(VirtReg));
            createMov(AsmOperand::createReg(VirtReg), Mem2);
        }
    }

    void handleSTORE(llvm::Instruction *I, AsmOperand::LabelOp Label,
                     AsmOperand::MemOp Mem) override {
        uint32_t VirtReg = Register::createVirtReg();
        createLea(Label, AsmOperand::createReg(VirtReg));
        createMov(AsmOperand::createReg(VirtReg), Mem);
    }

    AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I, AsmOperand::MemOp Mem,
                                          AsmOperand::ImmOp Imm) override {
        auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
        uint32_t SizeInBytes =
            GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
                GEP->getResultElementType());
        return {$2->getMemDisp() + SizeInBytes * Imm.Val, Mem.BaseReg, Mem.IndexReg,
                Mem.Scale};
    }

    AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I, AsmOperand::MemOp Mem,
                                          AsmOperand::RegOp Reg) override {
        auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
        uint32_t SizeInBytes =
            GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
                GEP->getResultElementType());
        if (Mem.BaseReg == Register::NoRegister) {
            return {Mem.Disp, Mem.BaseReg, Reg.RegNo, SizeInBytes};
        } else {
            uint32_t VirtReg = Register::createVirtReg();
            createLea(Mem, AsmOperand::createReg(VirtReg));
            return {0, VirtReg, Reg.RegNo, SizeInBytes};
        }
    }

    AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                          AsmOperand::ImmOp Imm) override {
        auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
        uint32_t SizeInBytes =
            GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
                GEP->getResultElementType());
        return {SizeInBytes * Imm.Val, Reg.RegNo};
    }

    AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                          AsmOperand::RegOp Reg2) override {
        auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
        uint32_t SizeInBytes =
            GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
                GEP->getResultElementType());
        return {0, Reg1.RegNo, Reg2.RegNo, SizeInBytes};
    }

    void handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                    AsmOperand::RegOp Reg2) override {
        createCmp(Reg2 Reg1);
    }

    void handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg,
                    AsmOperand::ImmOp Imm) override {
        reateCmp(Imm, Reg);
    }

    void handleICMP(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                    AsmOperand::RegOp Reg) override {
        uint32_t VirtReg = Register::createVirtReg();
        createMov(Imm, AsmOperand::createReg(VirtReg));
        createCmp(Reg, AsmOperand::createReg(VirtReg));
    }

    void handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label1,
                  AsmOperand::LabelOp Label2) override {
        auto *BI = llvm::cast<llvm::BranchInst>(I);
        auto *CI = llvm::cast<llvm::CmpInst>(BI->getCondition());
        AsmJmpInst::JmpKindTy JmpTrue, JmpFalse;
        switch (CI->getPredicate()) {
        case llvm::CmpInst::Predicate::ICMP_EQ:
            JmpTrue = AsmJmpInst::JmpKindTy::Je;
            JmpFalse = AsmJmpInst::JmpKindTy::Jne;
            break;
        case llvm::CmpInst::Predicate::ICMP_NE:
            JmpTrue = AsmJmpInst::JmpKindTy::Jne;
            JmpFalse = AsmJmpInst::JmpKindTy::Je;
            break;
        case llvm::CmpInst::Predicate::ICMP_SGT:
            JmpTrue = AsmJmpInst::JmpKindTy::Jg;
            JmpFalse = AsmJmpInst::JmpKindTy::Jle;
            break;
        default: llvm_unreachable("Invalid CmpInst!\n");
        }
        llvm::BasicBlock *NextBB = BI->getParent()->getNextNode();
        if (NextBB == BI->getSuccessor(0)) {
            createJmp(JmpFalse, Label2);
        } else if (NextBB == BI->getSuccessor(1)) {
            createJmp(JmpTrue, Label1);
        } else {
            createJmp(JmpTrue, Label1);
            createJmp(JmpFalse, Label2);
        }
    }

    void handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label) override {
        auto *BI = llvm::cast<llvm::BranchInst>(I);
        llvm::BasicBlock *NextBB = BI->getParent()->getNextNode();
        if (NextBB != BI->getSuccessor(0)) {
            createJmp(AsmJmpInst::JmpKindTy::Jmp, Label);
        }
    }

    AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                AsmOperand::RegOp Reg2) override {
        createAdd(Reg1, Reg2);
        return Reg2;
    }

    AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                AsmOperand::ImmOp Imm) override {
        createAdd(Imm, Reg);
        return Reg;
    }

    AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                AsmOperand::RegOp Reg) override {
        createAdd(Imm, Reg);
        return Reg;
    }

    AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                AsmOperand::ImmOp Imm2) override {
        llvm_unreachable("Unexpected handleADD(imm, imm)!\n");
        return {Regsiter::NoRegister};
    }

    AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                AsmOperand::RegOp Reg2) override {
        createSub(Reg2, Reg1);
        return Reg1;
    }

    AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                AsmOperand::ImmOp Imm) override {
        createSub(Imm, Reg);
        return Reg;
    }

    AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                AsmOperand::RegOp Reg) override {
        uint32_t VirtReg = Register::createVirtReg();
        createMov(Imm, AsmOperand::createReg(VirtReg));
        createSub(Reg, AsmOperand::createReg(VirtReg));
        return {VirtReg};
    }

    AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                AsmOperand::ImmOp Imm2) override {
        llvm_unreachable("Unexpected handleSUB(imm, imm)!\n");
        return {Regsiter::NoRegister};
    }

    AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                AsmOperand::RegOp Reg2) override {
        createImul(Reg1, Reg2);
        return Reg2;
    }

    AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                AsmOperand::ImmOp Imm) override {
        createImul(Imm, Reg);
        return Reg;
    }

    AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                AsmOperand::RegOp Reg) override {
        createImul(Imm, Reg);
        return Reg;
    }

    AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                AsmOperand::ImmOp Imm) override {
        llvm_unreachable("Unexpected handleMUL(imm, imm)!\n");
        return {Regsiter::NoRegister};
    }

    AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                 AsmOperand::RegOp Reg2) override {
        createMov(Reg1, AsmOperand::createReg(X86::RAX));
        createCqto();
        createIdiv(Reg2);
        return {X86::RAX};
    }

    AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                 AsmOperand::ImmOp Imm) override {
        uint32_t VirtReg = Register::createVirtReg();
        createMov(Reg, AsmOperand::createReg(X86::RAX));
        createCqto();
        createMov(Imm, AsmOperand::createReg(VirtReg));
        createIdiv(AsmOperand::createReg(VirtReg));
        return {X86::RAX};
    }

    AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                 AsmOperand::RegOp Reg) override {
        createMov(Imm, AsmOperand::createReg(X86::RAX));
        createCqto();
        createIdiv(Reg);
        return {Register::RAX};
    }

    AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                 AsmOperand::ImmOp Imm2) override {
        llvm_unreachable("Unexpected handleSDIV(imm, imm)!\n");
        return {Regsiter::NoRegister};
    }

    AsmOperand::RegOp handleCALL(llvm::Instruction *I,
                                 AsmOperand::LabelOp Label) override {
        auto *CB = llvm::cast<llvm::CallBase>(I);
        std::string CalleeName = Label.Symbol->getName();
        if (CalleeName == "printf" || CalleeName == "scanf") {
            createXor(AsmOperand::createReg(X86::RAX), AsmOperand::createReg(X86::RAX));
        }
        uint32_t VirtReg = Register::createVirtReg();
        createCall(Label, /*DirectCall*/ true, CB->arg_size());
        createMov(AsmOperand::createReg(X86::RAX), AsmOperand::createReg(VirtReg));
        return {VirtReg};
    }

    AsmOperand::RegOp handleCALL(llvm::Instruction *I, AsmOperand::RegOp Reg) override {
        auto *CB = llvm::cast<llvm::CallBase>(I);
        createCall(Reg, /*DirectCall*/ false, CB->arg_size());
        uint32_t VirtReg = Register::createVirtReg();
        createMov(AsmOperand::createReg(X86::RAX), AsmOperand::createReg(VirtReg));
        return {VirtReg};
    }

    AsmOperand::RegOp handleCALL(llvm::Instruction *I, AsmOperand::MemOp Mem) override {
        auto *CB = llvm::cast<llvm::CallBase>(I);
        createCall(Mem, /*DirectCall*/ false, CB->arg_size());
        uint32_t VirtReg = Register::createVirtReg();
        createMov(AsmOperand::createReg(X86::RAX), AsmOperand::createReg(VirtReg));
        return {VirtReg};
    }

    void handleARG(unsigned ArgNo, AsmOperand::RegOp Reg) override {
        if (ArgNo < 6) {
            createMov(Reg, AsmOperand::createReg(X86::ArgRegs[ArgNo]));
        } else {
            createMov(Reg, AsmOperand::createMem(8 * (ArgNo - 6), X86::RSP));
        }
    }

    void handleARG(unsigned ArgNo, AsmOperand::ImmOp Imm) override {
        if (ArgNo < 6) {
            createMov(Imm, AsmOperand::createReg(X86::ArgRegs[ArgNo]));
        } else {
            createMov(Imm, AsmOperand::createMem(8 * (ArgNo - 6), X86::RSP));
        }
    }

    void handleARG(unsigned ArgNo, AsmOperand::MemOp Mem) override {
        if (ArgNo < 6) {
            createLea(Mem, AsmOperand::createReg(X86::ArgRegs[ArgNo]));
        } else {
            uint32_t VirtReg = Register::createVirtReg();
            createMov(Mem, AsmOperand::createReg(VirtReg));
            createMov(AsmOperand::createReg(VirtReg),
                      AsmOperand::createMem(8 * (ArgNo - 6), X86::RSP));
        }
    }

    void handleARG(unsigned ArgNo, AsmOperand::LabelOp Label) override {
        if (ArgNo < 6) {
            createLea(Label, AsmOperand::createReg(X86::ArgRegs[ArgNo]));
        } else {
            createLea(Label, AsmOperand::createMem(8 * (ArgNo - 6), X86::RSP));
        }
    }

private:
    AsmMovInst *createMov(AsmOperand::MemOp Src, AsmOperand::RegOp Dst) {
        updateAsmOperandLiveRanges(Src);
        updateAsmOperandLiveRanges(Dst);
        auto *I = AsmMovInst::create(AsmOperand::create(Src), AsmOperand::create(Dst),
                                     CurrentFunction);
        LLVM_DEBUG({
            llvm::outs() << CurrentFunction->size();
            I->print(llvm::outs());
        });
        return I;
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

    void createCall(std::unique_ptr<AsmOperand> Callee, bool DirectCall,
                    unsigned NumArgs) {
        updateAsmOperandLiveRanges(*Callee);
        auto *I =
            AsmCallInst::create(std::move(Callee), DirectCall, NumArgs, CurrentFunction);
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

}  // namespace remniw
