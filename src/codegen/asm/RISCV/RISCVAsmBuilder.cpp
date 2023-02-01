#include "codegen/asm/RISCV/RISCVAsmBuilder.h"
#include "codegen/asm/RISCV/RISCVTargetInfo.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "remniw-RISCVAsmBuilder"

namespace remniw {

AsmOperand::MemOp RISCVAsmBuilder::handleALLOCA(uint32_t StackObjectIndex) {
    return AsmOperand::createStackObject(StackObjectIndex);
}

AsmOperand::RegOp RISCVAsmBuilder::handleLOAD(llvm::Instruction *I,
                                              AsmOperand::MemOp Mem) {
    uint32_t VirtReg = Register::createVirtReg();
    auto Reg = AsmOperand::createReg(VirtReg);
    createLDInst(
        /* destination register */ Reg,
        /* source register and offset*/ Mem);
    return Reg;
}

AsmOperand::RegOp RISCVAsmBuilder::handleLOAD(llvm::Instruction *I,
                                              AsmOperand::RegOp Reg) {
    uint32_t VirtReg = Register::createVirtReg();
    auto DstReg = AsmOperand::createReg(VirtReg);
    createLDInst(
        /* destination register */ DstReg,
        /* source register and offset */ AsmOperand::createMem(0, Reg.RegNo));
    return DstReg;
}

void RISCVAsmBuilder::handleRET(llvm::Instruction *I, AsmOperand::RegOp Reg) {
    createMVInst(/* destination register */ AsmOperand::createReg(RISCV::A0),
                 /* source register*/ Reg);
}

void RISCVAsmBuilder::handleRET(llvm::Instruction *I, AsmOperand::ImmOp Imm) {
    createLIInst(/* destination register */ AsmOperand::createReg(RISCV::A0),
                 /*immediate*/ Imm);
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                  AsmOperand::MemOp Mem) {
    createSDInst(/* source register */ Reg,
                 /* memory (base register and offset) */ Mem);
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                  AsmOperand::RegOp Reg2) {
    createSDInst(
        /* source register */ Reg1,
        /* memory (base register and offset) */ AsmOperand::createMem(0, Reg2.RegNo));
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                  AsmOperand::RegOp Reg) {
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* immediate */ Imm);
    createSDInst(
        /* source register */ AsmOperand::createReg(VirtReg),
        /* memory (base register and offset) */ AsmOperand::createMem(0, Reg.RegNo));
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                  AsmOperand::MemOp Mem) {
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* immediate */ Imm);
    createSDInst(
        /* source register */ AsmOperand::createReg(VirtReg),
        /* memory (base register and offset) */ Mem);
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::MemOp Mem1,
                                  AsmOperand::MemOp Mem2, bool DestIsArgument) {
    uint32_t VirtReg = Register::createVirtReg();
    storeMemoryAddressToReg(Mem1, AsmOperand::createReg(VirtReg));
    createSDInst(/* source register */ AsmOperand::createReg(VirtReg),
                 /* memory (base register and offset) */ Mem2);
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, AsmOperand::LabelOp Label,
                                  AsmOperand::MemOp Mem) {
    uint32_t VirtReg = Register::createVirtReg();
    createLAInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* source SYMBOL */ Label);
    createSDInst(/* source register */ AsmOperand::createReg(VirtReg),
                 /* memory (base register and offset) */ Mem);
}

void RISCVAsmBuilder::handleSTORE(llvm::Instruction *I, llvm::Argument *FuncArg,
                                  AsmOperand::MemOp Mem) {
    uint32_t VirtReg = Register::createVirtReg();
    unsigned ArgNo = FuncArg->getArgNo();
    if (ArgNo < RISCV::NumArgRegs) {
        createSDInst(/* source register */ AsmOperand::createReg(RISCV::ArgRegs[ArgNo]),
                     /* memory (base register and offset) */ Mem);
    } else {
        uint32_t FuncArgStackObjectIndex = ArgNo - RISCV::NumArgRegs;
        createLDInst(
            /* destination register */ AsmOperand::createReg(VirtReg),
            /* memory (base register and offset) */ AsmOperand::createStackObject(
                FuncArgStackObjectIndex));
        createSDInst(/* source register */ AsmOperand::createReg(VirtReg),
                     /* memory (base register and offset) */ Mem);
    }
}

AsmOperand::MemOp RISCVAsmBuilder::handleGETELEMENTPTR(llvm::Instruction *I,
                                                       AsmOperand::MemOp Mem,
                                                       AsmOperand::ImmOp Imm) {
    auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
    uint32_t SizeInBytes =
        GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
            GEP->getResultElementType());
    uint32_t VirtReg = Register::createVirtReg();
    storeMemoryAddressToReg(Mem, AsmOperand::createReg(VirtReg));
    return AsmOperand::createMem(SizeInBytes * Imm.Val, VirtReg);
}

AsmOperand::MemOp RISCVAsmBuilder::handleGETELEMENTPTR(llvm::Instruction *I,
                                                       AsmOperand::MemOp Mem,
                                                       AsmOperand::RegOp Reg) {
    auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
    uint32_t SizeInBytes =
        GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
            GEP->getResultElementType());
    uint32_t VirtReg1 = Register::createVirtReg();
    storeMemoryAddressToReg(Mem, AsmOperand::createReg(VirtReg1));
    uint32_t VirtReg2 = Register::createVirtReg();
    createLIInst(
        /* destination register */ AsmOperand::createReg(VirtReg2),
        /* source immediate */ AsmOperand::createImm(SizeInBytes));
    createMULInst(
        /* destination register */ AsmOperand::createReg(VirtReg2),
        /* source register 1 */ AsmOperand::createReg(VirtReg2),
        /* source register 2 */ AsmOperand::createReg(Reg.RegNo));
    createADDInst(/* destination register */ AsmOperand::createReg(VirtReg2),
                  /* source register 1 */ AsmOperand::createReg(VirtReg2),
                  /* source register 2 */ AsmOperand::createReg(VirtReg1));
    return AsmOperand::createMem(0, VirtReg2);
}

AsmOperand::MemOp RISCVAsmBuilder::handleGETELEMENTPTR(llvm::Instruction *I,
                                                       AsmOperand::RegOp Reg,
                                                       AsmOperand::ImmOp Imm) {
    auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
    uint32_t SizeInBytes =
        GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
            GEP->getResultElementType());
    return AsmOperand::createMem(SizeInBytes * Imm.Val, Reg.RegNo);
}

AsmOperand::MemOp RISCVAsmBuilder::handleGETELEMENTPTR(llvm::Instruction *I,
                                                       AsmOperand::RegOp Reg1,
                                                       AsmOperand::RegOp Reg2) {
    auto *GEP = llvm::cast<llvm::GetElementPtrInst>(I);
    uint32_t SizeInBytes =
        GEP->getFunction()->getParent()->getDataLayout().getTypeAllocSize(
            GEP->getResultElementType());
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(
        /* destination register */ AsmOperand::createReg(VirtReg),
        /* source immediate */ AsmOperand::createImm(SizeInBytes));
    createMULInst(
        /* destination register */ AsmOperand::createReg(VirtReg),
        /* source register 1 */ AsmOperand::createReg(VirtReg),
        /* source register 2 */ AsmOperand::createReg(Reg2.RegNo));
    createADDInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1 */ AsmOperand::createReg(VirtReg),
                  /* source register 2 */ AsmOperand::createReg(Reg1.RegNo));
    return AsmOperand::createMem(0, VirtReg);
}

void RISCVAsmBuilder::handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                 AsmOperand::RegOp Reg2) {
    auto *CI = llvm::cast<llvm::CmpInst>(I);
    CondRegsMap.insert({CI, {Reg1.RegNo, Reg2.RegNo}});
}

void RISCVAsmBuilder::handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                 AsmOperand::ImmOp Imm) {
    auto *CI = llvm::cast<llvm::CmpInst>(I);
    uint32_t LIDstReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    CondRegsMap.insert({CI, {Reg.RegNo, LIDstReg}});
}

void RISCVAsmBuilder::handleICMP(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                 AsmOperand::RegOp Reg) {
    auto *CI = llvm::cast<llvm::CmpInst>(I);
    uint32_t LIDstReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(LIDstReg),
                 /* immediate */ Imm);
    CondRegsMap.insert({CI, {LIDstReg, Reg.RegNo}});
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
    uint32_t VirtReg = Register::createVirtReg();
    createADDInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1 */ Reg1,
                  /* source register 2 */ Reg2);
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                             AsmOperand::ImmOp Imm) {
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* immediate */ Imm);
    createADDInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1 */ Reg,
                  /* source register 2 */ AsmOperand::createReg(VirtReg));
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                             AsmOperand::RegOp Reg) {
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* immediate */ Imm);
    createADDInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1 */ Reg,
                  /* source register 2 */ AsmOperand::createReg(VirtReg));
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                             AsmOperand::ImmOp Imm2) {
    llvm_unreachable("Unexpected handleADD(imm, imm)!\n");
    return {Register::NoRegister};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                             AsmOperand::RegOp Reg2) {
    uint32_t VirtReg = Register::createVirtReg();
    createSUBInst(/* destination register*/ AsmOperand::createReg(VirtReg),
                  /* source register 1*/ Reg1,
                  /* source register 2*/ Reg2);
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                             AsmOperand::ImmOp Imm) {
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* immediate */ Imm);
    createSUBInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1 */ Reg,
                  /* source register 2 */ AsmOperand::createReg(VirtReg));
    return {VirtReg};
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
    uint32_t VirtReg = Register::createVirtReg();
    createMULInst(/*destination register */ AsmOperand::createReg(VirtReg),
                  /*source register 1*/ Reg1,
                  /*source register 2*/ Reg2);
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                             AsmOperand::ImmOp Imm) {
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* immediate */ Imm);
    createMULInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1 */ Reg,
                  /* source register 2 */ AsmOperand::createReg(VirtReg));
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                             AsmOperand::RegOp Reg) {
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* immediate */ Imm);
    createMULInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1 */ AsmOperand::createReg(VirtReg),
                  /* source register 2 */ Reg);
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                             AsmOperand::ImmOp Imm2) {
    llvm_unreachable("Unexpected handleMUL(imm, imm)!\n");
    return {Register::NoRegister};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I,
                                              AsmOperand::RegOp Reg1,
                                              AsmOperand::RegOp Reg2) {
    uint32_t VirtReg = Register::createVirtReg();
    createDIVInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1*/ Reg1,
                  /* source register 2*/ Reg2);
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                              AsmOperand::ImmOp Imm) {
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* immediate */ Imm);
    createDIVInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1 */ Reg,
                  /* source register 2 */ AsmOperand::createReg(VirtReg));
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleSDIV(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                              AsmOperand::RegOp Reg) {
    uint32_t VirtReg = Register::createVirtReg();
    createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* immediate */ Imm);
    createDIVInst(/* destination register */ AsmOperand::createReg(VirtReg),
                  /* source register 1 */ AsmOperand::createReg(VirtReg),
                  /* source register 2 */ Reg);
    return {VirtReg};
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
    createCALLInst(Label, /*DirectCall*/ true, CB->arg_size());
    uint32_t VirtReg = Register::createVirtReg();
    createMVInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* source register */ AsmOperand::createReg(RISCV::A0));
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleCALL(llvm::Instruction *I,
                                              AsmOperand::RegOp Reg) {
    CallArgOffsetFromStackPointer = 0;
    auto *CB = llvm::cast<llvm::CallBase>(I);
    createCALLInst(Reg, /*DirectCall*/ false, CB->arg_size());
    uint32_t VirtReg = Register::createVirtReg();
    createMVInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* source register */ AsmOperand::createReg(RISCV::A0));
    return {VirtReg};
}

AsmOperand::RegOp RISCVAsmBuilder::handleCALL(llvm::Instruction *I,
                                              AsmOperand::MemOp Mem) {
    CallArgOffsetFromStackPointer = 0;
    auto *CB = llvm::cast<llvm::CallBase>(I);
    createCALLInst(Mem, /*DirectCall*/ false, CB->arg_size());
    uint32_t VirtReg = Register::createVirtReg();
    createMVInst(/* destination register */ AsmOperand::createReg(VirtReg),
                 /* source register */ AsmOperand::createReg(RISCV::A0));
    return {VirtReg};
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::RegOp Reg) {
    if (ArgNo < RISCV::NumArgRegs) {
        createMVInst(
            /* destination register */ AsmOperand::createReg(RISCV::ArgRegs[ArgNo]),
            /* source register */ Reg);
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
    if (ArgNo < RISCV::NumArgRegs) {
        createLIInst(
            /* destination register */ AsmOperand::createReg(RISCV::ArgRegs[ArgNo]),
            /* immediate */ Imm);
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        uint32_t VirtReg = Register::createVirtReg();
        createLIInst(/* destination register */ AsmOperand::createReg(VirtReg),
                     /* immediate */ Imm);
        createSDInst(/* source register */ AsmOperand::createReg(VirtReg),
                     /* memory (base register and offset) */ AsmOperand::createMem(
                         CallArgOffsetFromStackPointer, RISCV::SP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::MemOp Mem) {
    uint32_t VirtReg = Register::createVirtReg();
    storeMemoryAddressToReg(Mem, AsmOperand::createReg(VirtReg));
    if (ArgNo < RISCV::NumArgRegs) {
        createMVInst(
            /* destination register */ AsmOperand::createReg(RISCV::ArgRegs[ArgNo]),
            /* source register */ AsmOperand::createReg(VirtReg));
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        createSDInst(/* source register */ AsmOperand::createReg(VirtReg),
                     /* memory (base register and offset) */ AsmOperand::createMem(
                         CallArgOffsetFromStackPointer, RISCV::SP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleARG(llvm::Instruction *CI, unsigned ArgNo,
                                AsmOperand::LabelOp Label) {
    uint32_t LADstReg = Register::createVirtReg();
    createLAInst(/* destination register */ AsmOperand::createReg(LADstReg),
                 /* source SYMBOL */ Label);
    if (ArgNo < RISCV::NumArgRegs) {
        createMVInst(
            /* destination register */ AsmOperand::createReg(RISCV::ArgRegs[ArgNo]),
            /* source register */ AsmOperand::createReg(LADstReg));
    } else {
        auto *CB = llvm::cast<llvm::CallBase>(CI);
        llvm::Type *Ty = CB->getArgOperand(ArgNo)->getType();
        uint64_t SizeInBytes = CB->getModule()->getDataLayout().getTypeAllocSize(Ty);
        createSDInst(/* source register */ AsmOperand::createReg(LADstReg),
                     /* memory (base register and offset) */ AsmOperand::createMem(
                         CallArgOffsetFromStackPointer, RISCV::SP));
        CallArgOffsetFromStackPointer += SizeInBytes;
    }
}

void RISCVAsmBuilder::handleLABEL(AsmOperand::LabelOp Label) {
    createLABELInst(Label);
}

void RISCVAsmBuilder::storeMemoryAddressToReg(AsmOperand::MemOp SrcMem,
                                              AsmOperand::RegOp DstReg) {
    if (SrcMem.isStackObject()) {
        auto *StackObj = &getCurrentFunction()->StackObjects[SrcMem.StackObjectIndex];
        auto *I = createGetStackObjectAddressUserInst(DstReg, SrcMem);
    } else {
        createADDIInst(/* destination register */ DstReg,
                       /* source register 1 */ AsmOperand::createReg(SrcMem.BaseReg),
                       /* Immediate data */ AsmOperand::createImm(SrcMem.Disp));
    }
}

AsmInstruction *RISCVAsmBuilder::createLDInst(AsmOperand DstReg, AsmOperand SrcMem) {
    assert(DstReg.isReg() && SrcMem.isMem());
    updateAsmOperandLiveRanges(DstReg);
    updateAsmOperandLiveRanges(SrcMem);
    auto *I = AsmInstruction::create(RISCV::LD, getCurrentFunction());
    I->addOperand(DstReg);
    I->addOperand(SrcMem);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createSDInst(AsmOperand SrcReg, AsmOperand DstMem) {
    assert(SrcReg.isReg() && DstMem.isMem());
    updateAsmOperandLiveRanges(SrcReg);
    updateAsmOperandLiveRanges(DstMem);
    auto *I = AsmInstruction::create(RISCV::SD, getCurrentFunction());
    I->addOperand(SrcReg);
    I->addOperand(DstMem);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createMVInst(AsmOperand DstReg, AsmOperand SrcReg) {
    assert(DstReg.isReg() && SrcReg.isReg());
    updateAsmOperandLiveRanges(DstReg);
    updateAsmOperandLiveRanges(SrcReg);
    auto *I = AsmInstruction::create(RISCV::MV, getCurrentFunction());
    I->addOperand(DstReg);
    I->addOperand(SrcReg);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createLIInst(AsmOperand DstReg, AsmOperand Imm) {
    assert(DstReg.isReg() && Imm.isImm());
    updateAsmOperandLiveRanges(DstReg);
    auto *I = AsmInstruction::create(RISCV::LI, getCurrentFunction());
    I->addOperand(DstReg);
    I->addOperand(Imm);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createLAInst(AsmOperand DstReg, AsmOperand Label) {
    assert(DstReg.isReg() && Label.isLabel());
    updateAsmOperandLiveRanges(DstReg);
    auto *I = AsmInstruction::create(RISCV::LA, getCurrentFunction());
    I->addOperand(DstReg);
    I->addOperand(Label);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createBEQInst(AsmOperand Reg1, AsmOperand Reg2,
                                               AsmOperand Label) {
    assert(Reg1.isReg() && Reg2.isReg() && Label.isLabel());
    updateAsmOperandLiveRanges(Reg1);
    updateAsmOperandLiveRanges(Reg2);
    auto *I = AsmInstruction::create(RISCV::BEQ, getCurrentFunction());
    I->addOperand(Reg1);
    I->addOperand(Reg2);
    I->addOperand(Label);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createBNEInst(AsmOperand Reg1, AsmOperand Reg2,
                                               AsmOperand Label) {
    assert(Reg1.isReg() && Reg2.isReg() && Label.isLabel());
    updateAsmOperandLiveRanges(Reg1);
    updateAsmOperandLiveRanges(Reg2);
    auto *I = AsmInstruction::create(RISCV::BNE, getCurrentFunction());
    I->addOperand(Reg1);
    I->addOperand(Reg2);
    I->addOperand(Label);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createBGTInst(AsmOperand Reg1, AsmOperand Reg2,
                                               AsmOperand Label) {
    assert(Reg1.isReg() && Reg2.isReg() && Label.isLabel());
    updateAsmOperandLiveRanges(Reg1);
    updateAsmOperandLiveRanges(Reg2);
    auto *I = AsmInstruction::create(RISCV::BGT, getCurrentFunction());
    I->addOperand(Reg1);
    I->addOperand(Reg2);
    I->addOperand(Label);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createBLEInst(AsmOperand Reg1, AsmOperand Reg2,
                                               AsmOperand Label) {
    assert(Reg1.isReg() && Reg2.isReg() && Label.isLabel());
    updateAsmOperandLiveRanges(Reg1);
    updateAsmOperandLiveRanges(Reg2);
    auto *I = AsmInstruction::create(RISCV::BLE, getCurrentFunction());
    I->addOperand(Reg1);
    I->addOperand(Reg2);
    I->addOperand(Label);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createADDInst(AsmOperand DstReg, AsmOperand SrcReg1,
                                               AsmOperand SrcReg2) {
    assert(DstReg.isReg() && SrcReg1.isReg() && SrcReg2.isReg());
    updateAsmOperandLiveRanges(DstReg);
    updateAsmOperandLiveRanges(SrcReg1);
    updateAsmOperandLiveRanges(SrcReg2);
    auto *I = AsmInstruction::create(RISCV::ADD, getCurrentFunction());
    I->addOperand(DstReg);
    I->addOperand(SrcReg1);
    I->addOperand(SrcReg2);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createADDIInst(AsmOperand DstReg, AsmOperand SrcReg,
                                                AsmOperand SrcImm) {
    assert(DstReg.isReg() && SrcReg.isReg() && SrcImm.isMem());
    updateAsmOperandLiveRanges(DstReg);
    updateAsmOperandLiveRanges(SrcReg);
    if (SrcImm.Imm.Val >= -2048 && SrcImm.Imm.Val <= 2047) {
        auto *I = AsmInstruction::create(RISCV::ADDI, getCurrentFunction());
        I->addOperand(DstReg);
        I->addOperand(SrcReg);
        I->addOperand(SrcImm);
        return I;
    } else {
        uint32_t VirtReg = Register::createVirtReg();
        createLIInst(AsmOperand::createReg(VirtReg), SrcImm);
        auto *I = createADDInst(DstReg, SrcReg, AsmOperand::createReg(VirtReg));
        return I;
    }
}

AsmInstruction *RISCVAsmBuilder::createSUBInst(AsmOperand DstReg, AsmOperand SrcReg1,
                                               AsmOperand SrcReg2) {
    assert(DstReg.isReg() && SrcReg1.isReg() && SrcReg2.isReg());
    updateAsmOperandLiveRanges(DstReg);
    updateAsmOperandLiveRanges(SrcReg1);
    updateAsmOperandLiveRanges(SrcReg2);
    auto *I = AsmInstruction::create(RISCV::SUB, getCurrentFunction());
    I->addOperand(DstReg);
    I->addOperand(SrcReg1);
    I->addOperand(SrcReg2);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createMULInst(AsmOperand DstReg, AsmOperand SrcReg1,
                                               AsmOperand SrcReg2) {
    assert(DstReg.isReg() && SrcReg1.isReg() && SrcReg2.isReg());
    updateAsmOperandLiveRanges(DstReg);
    updateAsmOperandLiveRanges(SrcReg1);
    updateAsmOperandLiveRanges(SrcReg2);
    auto *I = AsmInstruction::create(RISCV::MUL, getCurrentFunction());
    I->addOperand(DstReg);
    I->addOperand(SrcReg1);
    I->addOperand(SrcReg2);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createDIVInst(AsmOperand DstReg, AsmOperand SrcReg1,
                                               AsmOperand SrcReg2) {
    assert(DstReg.isReg() && SrcReg1.isReg() && SrcReg2.isReg());
    updateAsmOperandLiveRanges(DstReg);
    updateAsmOperandLiveRanges(SrcReg1);
    updateAsmOperandLiveRanges(SrcReg2);
    auto *I = AsmInstruction::create(RISCV::DIV, getCurrentFunction());
    I->addOperand(DstReg);
    I->addOperand(SrcReg1);
    I->addOperand(SrcReg2);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createCALLInst(AsmOperand Callee, bool DirectCall,
                                                unsigned NumArgs) {
    updateAsmOperandLiveRanges(Callee);
    AsmInstruction *I;
    if (DirectCall) {
        I = AsmInstruction::create(RISCV::CALL, getCurrentFunction());
    } else {
        I = AsmInstruction::create(RISCV::JALR, getCurrentFunction());
    }
    I->addOperand(Callee);
    I->addOperand(AsmOperand::createImm(NumArgs));
    getCurrentCallInstIndexes().push_back(getCurrentFunction()->size());
    return I;
}

AsmInstruction *RISCVAsmBuilder::createLABELInst(AsmOperand LabelOp) {
    assert(LabelOp.isLabel());
    auto *I = AsmInstruction::create(RISCV::LABEL, getCurrentFunction());
    I->addOperand(LabelOp);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createJInst(AsmOperand LabelOp) {
    assert(LabelOp.isLabel());
    auto *I = AsmInstruction::create(RISCV::J, getCurrentFunction());
    I->addOperand(LabelOp);
    return I;
}

AsmInstruction *RISCVAsmBuilder::createGetStackObjectAddressUserInst(AsmOperand DstReg,
                                                                     AsmOperand SrcMem) {
    assert(DstReg.isReg() && SrcMem.isStackObject());
    updateAsmOperandLiveRanges(DstReg);
    updateAsmOperandLiveRanges(SrcMem);
    auto *I = AsmInstruction::create(RISCV::GET_STACKOBJECT_ADDRESS_USER_INST,
                                     getCurrentFunction());
    I->addOperand(DstReg);
    I->addOperand(SrcMem);
    return I;
}

}  // namespace remniw