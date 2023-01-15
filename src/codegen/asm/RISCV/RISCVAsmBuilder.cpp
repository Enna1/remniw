#include "codegen/asm/RISCV/RISCVAsmBuilder.h"
#include "codegen/asm/RISCV/RISCVTargetInfo.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "remniw-RISCVAsmBuilder"

namespace remniw {

AsmOperand::RegOp RISCVAsmBuilder::handleLOAD(llvm::Instruction *I,
                                              AsmOperand::MemOp Mem) {
    if (Mem.IndexReg != Register::NoRegister) {
        uint32_t LIDstReg = Register::createVirtReg();
        // MemScale
        auto *LI = createLIInst(
            /* destination register */ AsmOperand::createReg(LIDstReg),
            /* source immediate */ AsmOperand::createImm(Mem.Scale));

        // MemIndex * MemScale
        auto *MI = createMULInst(
            /* destination register */ AsmOperand::createReg(Mem.IndexReg),
            /* source register 1 */ AsmOperand::createReg(Mem.IndexReg),
            /* source register 2 */ AsmOperand::createReg(LIDstReg));

        // MemBase  MemIndex * MemScale
        auto *AI = createADDInst(
            /* destination register */ AsmOperand::createReg(Mem.BaseReg),
            /* source register 1 */ AsmOperand::createReg(Mem.BaseReg),
            /* source register 2 */ AsmOperand::createReg(Mem.IndexReg));
    }
    uint32_t LDDstReg = Register::createVirtReg();
    auto DstReg = AsmOperand::createReg(LDDstReg);
    auto *I = createLDInst(
        /* destination register */ DstReg,
        /* source register 1 and offset*/ AsmOperand::createMem(Mem.Disp, Mem.BaseReg));
    return DstReg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleLOAD(llvm::Instruction *I,
                                              AsmOperand::RegOp Reg) {
    uint32_t VirtReg = Register::createVirtReg();
    auto DstReg = AsmOperand::createReg(VirtReg);
    auto *I = RISCV64::LDInst::create(
        /* destination register */ DstReg,
        /* source register and offset */ AsmOperand::createMem(0, Reg.RegNo));
    return DstReg;
}

void RISCVAsmBuilder::handleRET(llvm::Instruction *I, AsmOperand::RegOp Reg) {
    createMVInst(/* destination register */ AsmOperand::createReg(RISCV::A0),
                 /* source register*/ Reg);
}

void RISCVAsmBuilder::handleRET(llvm::Instruction *I, AsmOperand::ImmOp Imm) {
    createLIInst(/* destination register */ AsmOperand::createReg(X86::RAX),
                 /*immediate*/ Imm);
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                  AsmOperand::MemOp Mem) {
    createSDInst(/* destination register */ Reg, Mem);
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                  AsmOperand::RegOp Reg2) {
    createMOVInst(Reg1, AsmOperand::createMem(0, Reg2.RegNo));
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                  AsmOperand::RegOp Reg) {
    createMOVInst(Imm, AsmOperand::createMem(0, Reg.RegNo));
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                  AsmOperand::MemOp Mem) {
    createMOVInst(Imm, Mem);
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::MemOp Mem1,
                                  AsmOperand::MemOp Mem2, bool DestIsArgument) {
    uint32_t VirtReg = Register::createVirtReg();
    if (DestIsArgument) {
        createMOVInst(Mem1, AsmOperand::createReg(VirtReg));
        createMOVInst(AsmOperand::createReg(VirtReg), Mem2);
    } else {
        createLEAInst(Mem1, AsmOperand::createReg(VirtReg));
        createMOVInst(AsmOperand::createReg(VirtReg), Mem2);
    }
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::LabelOp Label,
                                  AsmOperand::MemOp Mem) {
    uint32_t VirtReg = Register::createVirtReg();
    createLEAInst(Label, AsmOperand::createReg(VirtReg));
    createMOVInst(AsmOperand::createReg(VirtReg), Mem);
}

AsmOperand::MemOp RISCVAsmBuilder::handleGETELEMENTPTR(llvm::Instruction *I,
                                                       AsmOperand::MemOp Mem,
                                                       AsmOperand::ImmOp Imm) {
    auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
    uint32_t SizeInBytes =
        GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
            GEP->getResultElementType());
    return {Mem.Disp + SizeInBytes * Imm.Val, Mem.BaseReg, Mem.IndexReg, Mem.Scale};
}

AsmOperand::MemOp RISCVAsmBuilder::handleGETELEMENTPTR(llvm::Instruction *I,
                                                       AsmOperand::MemOp Mem,
                                                       AsmOperand::RegOp Reg) {
    auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
    uint32_t SizeInBytes =
        GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
            GEP->getResultElementType());
    if (Mem.BaseReg == Register::NoRegister) {
        return {Mem.Disp, Mem.BaseReg, Reg.RegNo, SizeInBytes};
    } else {
        uint32_t VirtReg = Register::createVirtReg();
        createLEAInst(Mem, AsmOperand::createReg(VirtReg));
        return {0, VirtReg, Reg.RegNo, SizeInBytes};
    }
}

AsmOperand::MemOp RISCVAsmBuilder::handleGETELEMENTPTR(llvm::Instruction *I,
                                                       AsmOperand::RegOp Reg,
                                                       AsmOperand::ImmOp Imm) {
    auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
    uint32_t SizeInBytes =
        GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
            GEP->getResultElementType());
    return {SizeInBytes * Imm.Val, Reg.RegNo};
}

AsmOperand::MemOp RISCVAsmBuilder::handleGETELEMENTPTR(llvm::Instruction *I,
                                                       AsmOperand::RegOp Reg1,
                                                       AsmOperand::RegOp Reg2) {
    auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
    uint32_t SizeInBytes =
        GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
            GEP->getResultElementType());
    return {0, Reg1.RegNo, Reg2.RegNo, SizeInBytes};
}

void RISCVAsmBuilder::handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                 AsmOperand::RegOp Reg2) {
    createCMPInst(Reg2, Reg1);
}

void RISCVAsmBuilder::handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                 AsmOperand::ImmOp Imm) {
    createCMPInst(Imm, Reg);
}

void RISCVAsmBuilder::handleICMP(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                 AsmOperand::RegOp Reg) {
    uint32_t VirtReg = Register::createVirtReg();
    createMOVInst(Imm, AsmOperand::createReg(VirtReg));
    createCMPInst(Reg, AsmOperand::createReg(VirtReg));
}

void RISCVAsmBuilder::handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label1,
                               AsmOperand::LabelOp Label2) {
    auto *BI = llvm::cast<llvm::BranchInst>(I);
    auto *CI = llvm::cast<llvm::CmpInst>(BI->getCondition());
    unsigned JmpTrueOpcode, JmpFalseOpcode;
    switch (CI->getPredicate()) {
    case llvm::CmpInst::Predicate::ICMP_EQ:
        JmpTrueOpcode = X86::JE;
        JmpFalseOpcode = X86::JNE;
        break;
    case llvm::CmpInst::Predicate::ICMP_NE:
        JmpTrueOpcode = X86::JNE;
        JmpFalseOpcode = X86::JE;
        break;
    case llvm::CmpInst::Predicate::ICMP_SGT:
        JmpTrueOpcode = X86::JG;
        JmpFalseOpcode = X86::JLE;
        break;
    default: llvm_unreachable("Invalid CmpInst!\n");
    }
    llvm::BasicBlock *NextBB = BI->getParent()->getNextNode();
    if (NextBB == BI->getSuccessor(0)) {
        createJMPInst(JmpFalseOpcode, Label2);
    } else if (NextBB == BI->getSuccessor(1)) {
        createJMPInst(JmpTrueOpcode, Label1);
    } else {
        createJMPInst(JmpTrueOpcode, Label1);
        createJMPInst(JmpFalseOpcode, Label2);
    }
}

void RISCVAsmBuilder::handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label) {
    auto *BI = llvm::cast<llvm::BranchInst>(I);
    llvm::BasicBlock *NextBB = BI->getParent()->getNextNode();
    if (NextBB != BI->getSuccessor(0)) {
        createJMPInst(X86::JMP, Label);
    }
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                             AsmOperand::RegOp Reg2) {
    createADDInst(Reg1, Reg2);
    return Reg2;
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                             AsmOperand::ImmOp Imm) {
    createADDInst(Imm, Reg);
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                             AsmOperand::RegOp Reg) {
    createADDInst(Imm, Reg);
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                             AsmOperand::ImmOp Imm2) {
    llvm_unreachable("Unexpected handleADD(imm, imm)!\n");
    return {Register::NoRegister};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                             AsmOperand::RegOp Reg2) {
    createSUBInst(Reg2, Reg1);
    return Reg1;
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                             AsmOperand::ImmOp Imm) {
    createSUBInst(Imm, Reg);
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                             AsmOperand::RegOp Reg) {
    uint32_t VirtReg = Register::createVirtReg();
    createMOVInst(Imm, AsmOperand::createReg(VirtReg));
    createSUBInst(Reg, AsmOperand::createReg(VirtReg));
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                             AsmOperand::ImmOp Imm2) {
    llvm_unreachable("Unexpected handleSUB(imm, imm)!\n");
    return {Register::NoRegister};
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                             AsmOperand::RegOp Reg2) {
    createIMULInst(Reg1, Reg2);
    return Reg2;
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                             AsmOperand::ImmOp Imm) {
    createIMULInst(Imm, Reg);
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                             AsmOperand::RegOp Reg) {
    createIMULInst(Imm, Reg);
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                             AsmOperand::ImmOp Imm2) {
    llvm_unreachable("Unexpected handleMUL(imm, imm)!\n");
    return {Register::NoRegister};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I,
                                              AsmOperand::RegOp Reg1,
                                              AsmOperand::RegOp Reg2) {
    createMOVInst(Reg1, AsmOperand::createReg(X86::RAX));
    createCQTOInst();
    createIDIVInst(Reg2);
    return {X86::RAX};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                              AsmOperand::ImmOp Imm) {
    uint32_t VirtReg = Register::createVirtReg();
    createMOVInst(Reg, AsmOperand::createReg(X86::RAX));
    createCQTOInst();
    createMOVInst(Imm, AsmOperand::createReg(VirtReg));
    createIDIVInst(AsmOperand::createReg(VirtReg));
    return {X86::RAX};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                              AsmOperand::RegOp Reg) {
    createMOVInst(Imm, AsmOperand::createReg(X86::RAX));
    createCQTOInst();
    createIDIVInst(Reg);
    return {X86::RAX};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I,
                                              AsmOperand::ImmOp Imm1,
                                              AsmOperand::ImmOp Imm2) {
    llvm_unreachable("Unexpected handleSDIV(imm, imm)!\n");
    return {Register::NoRegister};
}

AsmOperand::RegOp RISCVAsmBuilder::handleCALL(llvm::Instruction *I,
                                              AsmOperand::LabelOp Label) {
    CallArgOffsetFromStackPointer = 0;
    auto *CB = llvm::cast<llvm::CallBase>(I);
    std::string CalleeName = Label.Symbol->getName();
    if (CalleeName == "printf" || CalleeName == "scanf") {
        createXORInst(AsmOperand::createReg(X86::RAX), AsmOperand::createReg(X86::RAX));
    }
    uint32_t VirtReg = Register::createVirtReg();
    createCALLInst(Label, /*DirectCall*/ true, CB->arg_size());
    createMOVInst(AsmOperand::createReg(X86::RAX), AsmOperand::createReg(VirtReg));
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleCALL(llvm::Instruction *I,
                                              AsmOperand::RegOp Reg) {
    CallArgOffsetFromStackPointer = 0;
    auto *CB = llvm::cast<llvm::CallBase>(I);
    createCALLInst(Reg, /*DirectCall*/ false, CB->arg_size());
    uint32_t VirtReg = Register::createVirtReg();
    createMOVInst(AsmOperand::createReg(X86::RAX), AsmOperand::createReg(VirtReg));
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleCALL(llvm::Instruction *I,
                                              AsmOperand::MemOp Mem) {
    CallArgOffsetFromStackPointer = 0;
    auto *CB = llvm::cast<llvm::CallBase>(I);
    createCALLInst(Mem, /*DirectCall*/ false, CB->arg_size());
    uint32_t VirtReg = Register::createVirtReg();
    createMOVInst(AsmOperand::createReg(X86::RAX), AsmOperand::createReg(VirtReg));
    return {VirtReg};
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::RegOp Reg) {
    if (ArgNo < X86::NumArgRegs) {
        createMOVInst(Reg, AsmOperand::createReg(X86::ArgRegs[ArgNo]));
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        createMOVInst(Reg,
                      AsmOperand::createMem(CallArgOffsetFromStackPointer, X86::RSP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::ImmOp Imm) {
    if (ArgNo < X86::NumArgRegs) {
        createMOVInst(Imm, AsmOperand::createReg(X86::ArgRegs[ArgNo]));
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        createMOVInst(Imm,
                      AsmOperand::createMem(CallArgOffsetFromStackPointer, X86::RSP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::MemOp Mem) {
    if (ArgNo < X86::NumArgRegs) {
        createLEAInst(Mem, AsmOperand::createReg(X86::ArgRegs[ArgNo]));
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        uint32_t VirtReg = Register::createVirtReg();
        createMOVInst(Mem, AsmOperand::createReg(VirtReg));
        createMOVInst(AsmOperand::createReg(VirtReg),
                      AsmOperand::createMem(CallArgOffsetFromStackPointer, X86::RSP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::LabelOp Label) {
    if (ArgNo < X86::NumArgRegs) {
        createLEAInst(Label, AsmOperand::createReg(X86::ArgRegs[ArgNo]));
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        createLEAInst(Label,
                      AsmOperand::createMem(CallArgOffsetFromStackPointer, X86::RSP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleLABEL(AsmOperand::LabelOp Label) {
    createLABELInst(Label);
}

AsmInstruction *RISCVAsmBuilder::createMOVInst(AsmOperand Src, AsmOperand Dst) {
    updateAsmOperandLiveRanges(Src);
    updateAsmOperandLiveRanges(Dst);
    auto *I = AsmInstruction::create(X86::MOV, getCurrentFunction());
    I->addOperand(Src);
    I->addOperand(Dst);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createLEAInst(AsmOperand Src, AsmOperand Dst) {
    updateAsmOperandLiveRanges(Src);
    updateAsmOperandLiveRanges(Dst);
    auto *I = AsmInstruction::create(X86::LEA, getCurrentFunction());
    I->addOperand(Src);
    I->addOperand(Dst);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createCMPInst(AsmOperand Src, AsmOperand Dst) {
    updateAsmOperandLiveRanges(Src);
    updateAsmOperandLiveRanges(Dst);
    auto *I = AsmInstruction::create(X86::CMP, getCurrentFunction());
    I->addOperand(Src);
    I->addOperand(Dst);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createJMPInst(unsigned JmpOpcode, AsmOperand Op) {
    updateAsmOperandLiveRanges(Op);
    auto *I = AsmInstruction::create(JmpOpcode, getCurrentFunction());
    I->addOperand(Op);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createADDInst(AsmOperand Src, AsmOperand Dst) {
    updateAsmOperandLiveRanges(Src);
    updateAsmOperandLiveRanges(Dst);
    auto *I = AsmInstruction::create(X86::ADD, getCurrentFunction());
    I->addOperand(Src);
    I->addOperand(Dst);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createSUBInst(AsmOperand Src, AsmOperand Dst) {
    updateAsmOperandLiveRanges(Src);
    updateAsmOperandLiveRanges(Dst);
    auto *I = AsmInstruction::create(X86::SUB, getCurrentFunction());
    I->addOperand(Src);
    I->addOperand(Dst);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createIMULInst(AsmOperand Src, AsmOperand Dst) {
    updateAsmOperandLiveRanges(Src);
    updateAsmOperandLiveRanges(Dst);
    auto *I = AsmInstruction::create(X86::IMUL, getCurrentFunction());
    I->addOperand(Src);
    I->addOperand(Dst);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createIDIVInst(AsmOperand Op) {
    updateAsmOperandLiveRanges(Op);
    updateRegLiveRanges(X86::RAX);
    updateRegLiveRanges(X86::RDX);
    auto *I = AsmInstruction::create(X86::IDIV, getCurrentFunction());
    I->addOperand(Op);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createCQTOInst() {
    updateRegLiveRanges(X86::RAX);
    updateRegLiveRanges(X86::RDX);
    auto *I = AsmInstruction::create(X86::CQTO, getCurrentFunction());
    return I;
}

AsmInstruction *RISCVAsmBuilder::createCALLInst(AsmOperand Callee, bool DirectCall,
                                                unsigned NumArgs) {
    updateAsmOperandLiveRanges(Callee);
    auto *I = AsmInstruction::create(X86::CALL, getCurrentFunction());
    I->addOperand(Callee);
    I->addOperand(AsmOperand::createImm(NumArgs));
    getCurrentCallInstIndexes().push_back(getCurrentFunction()->size());
    return I;
}

AsmInstruction *RISCVAsmBuilder::createXORInst(AsmOperand Src, AsmOperand Dst) {
    updateAsmOperandLiveRanges(Src);
    updateAsmOperandLiveRanges(Dst);
    auto *I = AsmInstruction::create(X86::XOR, getCurrentFunction());
    I->addOperand(Src);
    I->addOperand(Dst);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createLABELInst(AsmOperand LabelOp) {
    auto *I = AsmInstruction::create(X86::LABEL, getCurrentFunction());
    I->addOperand(LabelOp);
    return I;
}

}  // namespace remniw