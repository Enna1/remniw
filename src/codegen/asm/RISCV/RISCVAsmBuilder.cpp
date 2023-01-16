#include "codegen/asm/RISCV/RISCVAsmBuilder.h"
#include "codegen/asm/RISCV/RISCVTargetInfo.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "remniw-RISCVAsmBuilder"

namespace remniw {

AsmOperand::RegOp RISCVAsmBuilder::handleLOAD(llvm::Instruction *I,
                                              AsmOperand::MemOp Mem) {
    normalizeAsmMemoryOperand(Mem);
    uint32_t LDDstReg = Register::createVirtReg();
    auto DstReg = AsmOperand::createReg(LDDstReg);
    auto *I = createLDInst(
        /* destination register */ DstReg,
        /* source register and offset*/ AsmOperand::createMem(Mem.Disp, Mem.BaseReg));
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
    normalizeAsmMemoryOperand(Mem);
    auto *SI = createSDInst(
        /* source register */ Reg,
        /* memory (base register and offset) */ AsmOperand::createMem(MemDisp,
                                                                      MemBaseReg));
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                  AsmOperand::RegOp Reg2) {
    createSDInst(
        /* source register */ Reg1,
        /* memory (base register and offset) */ AsmOperand::createMem(0, Reg2.RegNo));
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                  AsmOperand::RegOp Reg) {
    uint32_t VirtReg = remniw::Register::createVirtReg();
    auto *LI = createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                            /* immediate */ Imm);
    auto *SI = createSDInst(
        /* source register */ AsmOperand::createReg(VirtReg),
        /* memory (base register and offset) */ AsmOperand::createMem(0, Reg.RegNo));
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                  AsmOperand::MemOp Mem) {
    normalizeAsmMemoryOperand(Mem);
    uint32_t VirtReg = remniw::Register::createVirtReg();
    auto *LI = createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                            /* immediate */ Imm);
    auto *SI = createSDInst(
        /* source register */ AsmOperand::createReg(VirtReg),
        /* memory (base register and offset) */ Mem);
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::MemOp Mem1,
                                  AsmOperand::MemOp Mem2, bool DestIsArgument) {
    normalizeAsmMemoryOperand(Mem1);
    normalizeAsmMemoryOperand(Mem2);
    if (DestIsArgument) {
        uint32_t LDDstReg = remniw::Register::createVirtReg();
        createLDInst(/* destination register */ AsmOperand::createReg(LDDstReg),
                     /* memory (base register and offset) */ Mem1);
        createSDInst(/* source register */ AsmOperand::createReg(LDDstReg),
                     /* memory (base register and offset) */ Mem2);
    } else {
        createADDIInst(/* destination register */ AsmOperand::createReg(Mem1.BaseReg),
                       /* source register 1 */ AsmOperand::createReg(Mem1.BaseReg),
                       /* Immediate data */ AsmOperand::createImm(Mem1.Disp));
        createSDInst(/* source register */ AsmOperand::createReg(Mem1.BaseReg),
                     /* memory (base register and offset) */ Mem2);
    }
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::LabelOp Label,
                                  AsmOperand::MemOp Mem) {
    normalizeAsmMemoryOperand(Mem);
    uint32_t LADstReg = Register::createVirtReg();
    createLAInst(/* destination register */ AsmOperand::createReg(LADstReg),
                 /* source SYMBOL */ Label);
    createSDInst(/* source register */ AsmOperand::createReg(LADstReg),
                 /* memory (base register and offset) */ Mem);
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
    if (Mem.IndexReg == Register::NoRegister) {
        return {Mem.Disp, Mem.BaseReg, Reg.RegNo, SizeInBytes};
    } else {
        normalizeAsmMemoryOperand(Mem);
        createADDIInst(/* destination register */ AsmOperand::createReg(Mem.BaseReg),
                       /* source register 1 */ AsmOperand::createReg(Mem.BaseReg),
                       /* Immediate */ AsmOperand::createImm(Mem.Disp));
        return {0, Mem.BaseReg, Reg.RegNo, SizeInBytes};
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
    auto *CI = llvm::cast<llvm::CmpInst>(I);
    CondRegsMap.insert(std::make_pair(ICI, {Reg1, Reg2}));
}

void RISCVAsmBuilder::handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                 AsmOperand::ImmOp Imm) {
    auto *CI = llvm::cast<llvm::CmpInst>(I);
    uint32_t LIDstReg = remniw::Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    CondRegsMap.insert(std::make_pair(CI, {Reg, LIDstReg}));
}

void RISCVAsmBuilder::handleICMP(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                 AsmOperand::RegOp Reg) {
    auto *CI = llvm::cast<llvm::CmpInst>(I);
    uint32_t LIDstReg = remniw::Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    CondRegsMap.insert(std::make_pair(CI, {LIDstReg, Reg}));
}

void RISCVAsmBuilder::handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label1,
                               AsmOperand::LabelOp Label2) {
    auto *BI = llvm::cast<llvm::BranchInst>(I);
    auto *CI = llvm::cast<llvm::CmpInst>(BI->getCondition());
    switch (CI->getPredicate()) {
    case llvm::CmpInst::Predicate::ICMP_EQ:
        createBEQInst(
            /* source register 1 */ AsmOperand::createReg(CondRegsMap[CI].first),
            /* source register 2 */ AsmOperand::createReg(CondRegsMap[CI].second),
            Label1);
        createBNEInst(
            /* source register 1 */ AsmOperand::createReg(CondRegsMap[CI].first),
            /* source register 2 */ AsmOperand::createReg(CondRegsMap[CI].second),
            Label2);
        break;
    case llvm::CmpInst::Predicate::ICMP_NE:
        createBNEInst(
            /* source register 1 */ AsmOperand::createReg(CondRegsMap[CI].first),
            /* source register 2 */ AsmOperand::createReg(CondRegsMap[CI].second),
            Label1);
        createBEQInst(
            /* source register 1 */ AsmOperand::createReg(CondRegsMap[CI].first),
            /* source register 2 */ AsmOperand::createReg(CondRegsMap[CI].second),
            Label2);
        break;
    case llvm::CmpInst::Predicate::ICMP_SGT:
        createBGTInst(
            /* source register 1 */ AsmOperand::createReg(CondRegsMap[CI].first),
            /* source register 2 */ AsmOperand::createReg(CondRegsMap[CI].second),
            Label1);
        createBLEInst(
            /* source register 1 */ AsmOperand::createReg(CondRegsMap[CI].first),
            /* source register 2 */ AsmOperand::createReg(CondRegsMap[CI].second),
            Label2);
        break;
    default: llvm_unreachable("Invalid CmpInst!\n");
    }

    // TODO: optimize
    llvm::BasicBlock *NextBB = BI->getParent()->getNextNode();
    if (NextBB == BI->getSuccessor(0)) {
    } else if (NextBB == BI->getSuccessor(1)) {
    } else {
    }
}

void RISCVAsmBuilder::handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label) {
    createJInst(Label);
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                             AsmOperand::RegOp Reg2) {
    createADDInst(/* destination register */ Reg1,
                  /* source register 1*/ Reg1,
                  /* source register 2 */ Reg2);
    return Reg1;
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                             AsmOperand::ImmOp Imm) {
    createADDIInst(/* destination register */ Reg,
                   /* source register 1 */ Reg,
                   /* Immediate */ Imm);
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                             AsmOperand::RegOp Reg) {
    createADDIInst(/* destination register */ Reg,
                   /* source register 1 */ Reg,
                   /* Immediate */ Imm);
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                             AsmOperand::ImmOp Imm2) {
    llvm_unreachable("Unexpected handleADD(imm, imm)!\n");
    return {Register::NoRegister};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                             AsmOperand::RegOp Reg2) {
    createSUBInst(/* destination register*/ Reg1,
                  /* source register 1*/ Reg1,
                  /* source register 2*/ Reg2);
    return Reg1;
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                             AsmOperand::ImmOp Imm) {
    uint32_t LIDstReg = remniw::Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    createSUBInst(/* destination register */ Reg,
                  /* source register 1 */ Reg,
                  /* source register 2 */ remniw::AsmOperand::createReg(LIDstReg));
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                             AsmOperand::RegOp Reg) {
    uint32_t LIDstReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    createSUBInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                  /* source register 1 */ AsmOperand::createReg(LIDstReg),
                  /* source register 2 */ Reg);
    return {LIDstReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                             AsmOperand::ImmOp Imm2) {
    llvm_unreachable("Unexpected handleSUB(imm, imm)!\n");
    return {Register::NoRegister};
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                             AsmOperand::RegOp Reg2) {
    createMULInst(/*destination register */ Reg1,
                  /*source register 1*/ Reg1,
                  /*source register 2*/ Reg2);
    return Reg1;
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                             AsmOperand::ImmOp Imm) {
    uint32_t LIDstReg = remniw::Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    createMULInst(/* destination register */ Reg,
                  /* source register 1 */ Reg,
                  /* source register 2 */ remniw::AsmOperand::createReg(LIDstReg));
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                             AsmOperand::RegOp Reg) {
    uint32_t LIDstReg = remniw::Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    createMULInst(/* destination register */ remniw::AsmOperand::createReg(LIDstReg),
                  /* source register 1 */ remniw::AsmOperand::createReg(LIDstReg),
                  /* source register 2 */ Reg;
    return {LIDstReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                             AsmOperand::ImmOp Imm2) {
    llvm_unreachable("Unexpected handleMUL(imm, imm)!\n");
    return {Register::NoRegister};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I,
                                              AsmOperand::RegOp Reg1,
                                              AsmOperand::RegOp Reg2) {
    createDIVInst(/* destination register */ Reg1,
                  /* source register 1*/ Reg1,
                  /* source register 2*/ Reg2);
    return Reg1;
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                              AsmOperand::ImmOp Imm) {
    uint32_t LIDstReg = remniw::Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    createDIVInst(/* destination register */ Reg,
                  /* source register 1 */ Reg,
                  /* source register 2 */ remniw::AsmOperand::createReg(LIDstReg));
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                              AsmOperand::RegOp Reg) {
    uint32_t LIDstReg = remniw::Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    createDIVInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                  /* source register 1 */ AsmOperand::createReg(LIDstReg),
                  /* source register 2 */ Reg);
    return {LIDstReg};
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
    if (argNo < RISCV::NumArgRegs) {
        createMVInst(
            /* source register */ Reg,
            /* destination register */ AsmOperand::createReg(RISCV::ArgRegs[argNo]));
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        createSDInst(/* source register */ Reg,
                     /* memory (base register and offset) */ AsmOperand::createMem(
                         CallArgOffsetFromStackPointer, RISCV::SP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::ImmOp Imm) {
    if (argNo < RISCV::NumArgRegs) {
        createLIInst(
            /* destination register */ AsmOperand::createReg(RISCV::ArgRegs[argNo])
            /* immediate */ Imm);
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        uint32_t VirtReg = remniw::Register::createVirtReg();
        createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                     /* immediate */ Imm);
        createSDInst(/* source register */ remniw::AsmOperand::createReg(VirtReg),
                     /* memory (base register and offset) */ AsmOperand::createMem(
                         CallArgOffsetFromStackPointer, RISCV::SP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::MemOp Mem) {
    normalizeAsmMemoryOperand(Mem);
    createADDIInst(/* destination register */ AsmOperand::createReg(Mem.BaseReg),
                   /* source register 1 */ AsmOperand::createReg(Mem.BaseReg),
                   /* Immediate data */ AsmOperand::createImm(Mem.Disp));
    if (argNo < RISCV::NumArgRegs) {
        createMVInst(
            /* destination register */ AsmOperand::createReg(RISCV::ArgRegs[argNo]),
            /* source register */ remniw::AsmOperand::createReg(Mem.BaseReg));
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        createSDInst(/* source register */ remniw::AsmOperand::createReg(Mem.BaseReg),
                     /* memory (base register and offset) */ AsmOperand::createMem(
                         CallArgOffsetFromStackPointer, RISCV::SP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::LabelOp Label) {
    uint32_t LADstReg = remniw::Register::createVirtReg();
    createLAInst(/* destination register */ AsmOperand::createReg(LADstReg),
                 /* source SYMBOL */ AsmOperand::createLabel(Label));
    if (argNo < RISCV::NumArgRegs) {
        createMVInst(
            /* destination register */ AsmOperand::createReg(RISCV::ArgRegs[argNo])
            /* source register */ remniw::AsmOperand::createReg(LADstReg));
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        createSDInst(/* source register */ remniw::AsmOperand::createReg(LADstReg),
                     /* memory (base register and offset) */ AsmOperand::createMem(
                         CallArgOffsetFromStackPointer, RISCV::SP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleLABEL(AsmOperand::LabelOp Label) {
    createLABELInst(Label);
}

void RISCVAsmBuilder::normalizeAsmMemoryOperand(AsmOperand &MemOp) {
    assert(MemOp.isMem());
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