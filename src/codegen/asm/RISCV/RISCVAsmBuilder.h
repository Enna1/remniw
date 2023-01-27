#pragma once

#include "codegen/asm/AsmBuilder.h"
#include "codegen/asm/AsmInstruction.h"
#include "codegen/asm/AsmOperand.h"
#include "codegen/asm/RISCV/RISCVTargetInfo.h"

namespace remniw {

class RISCVAsmBuilder: public AsmBuilder {
private:
    RISCVTargetInfo TI;
    int64_t CallArgOffsetFromStackPointer {0};
    llvm::DenseMap<llvm::CmpInst *, std::pair<uint32_t, uint32_t>> CondRegsMap;

public:
    const TargetInfo &getTargetInfo() const override { return TI; }

    AsmOperand::RegOp handleLOAD(llvm::Instruction *I, AsmOperand::MemOp Mem) override;
    AsmOperand::RegOp handleLOAD(llvm::Instruction *I, AsmOperand::RegOp Reg) override;

    void handleRET(llvm::Instruction *I, AsmOperand::RegOp Reg) override;
    void handleRET(llvm::Instruction *I, AsmOperand::ImmOp Imm) override;

    void handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg,
                     AsmOperand::MemOp Mem) override;
    void handleSTORE(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                     AsmOperand::RegOp Reg2) override;
    void handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                     AsmOperand::RegOp Reg) override;
    void handleSTORE(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                     AsmOperand::MemOp Mem) override;
    void handleSTORE(llvm::Instruction *I, AsmOperand::MemOp Mem1, AsmOperand::MemOp Mem2,
                     bool DestIsArgument) override;
    void handleSTORE(llvm::Instruction *I, AsmOperand::LabelOp Label,
                     AsmOperand::MemOp Mem) override;
    void handleSTORE(llvm::Instruction *I, llvm::Argument *FuncArg,
                     AsmOperand::MemOp Mem) override;

    AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I, AsmOperand::MemOp Mem,
                                          AsmOperand::ImmOp Imm);
    AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I, AsmOperand::MemOp Mem,
                                          AsmOperand::RegOp Reg);
    AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                          AsmOperand::ImmOp Imm) override;
    AsmOperand::MemOp handleGETELEMENTPTR(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                          AsmOperand::RegOp Reg2) override;

    void handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                    AsmOperand::RegOp Reg2) override;
    void handleICMP(llvm::Instruction *I, AsmOperand::RegOp Reg,
                    AsmOperand::ImmOp Imm) override;
    void handleICMP(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                    AsmOperand::RegOp Reg) override;

    void handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label1,
                  AsmOperand::LabelOp Label2) override;
    void handleBR(llvm::Instruction *I, AsmOperand::LabelOp Label) override;

    AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                AsmOperand::RegOp Reg2) override;
    AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                AsmOperand::ImmOp Imm) override;
    AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                AsmOperand::RegOp Reg) override;
    AsmOperand::RegOp handleADD(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                AsmOperand::ImmOp Imm2) override;

    AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                AsmOperand::RegOp Reg2) override;
    AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                AsmOperand::ImmOp Imm) override;
    AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                AsmOperand::RegOp Reg) override;
    AsmOperand::RegOp handleSUB(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                AsmOperand::ImmOp Imm2) override;

    AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                AsmOperand::RegOp Reg2) override;
    AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                AsmOperand::ImmOp Imm) override;
    AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                AsmOperand::RegOp Reg) override;
    AsmOperand::RegOp handleMUL(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                AsmOperand::ImmOp Imm2) override;

    AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::RegOp Reg1,
                                 AsmOperand::RegOp Reg2) override;
    AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::RegOp Reg,
                                 AsmOperand::ImmOp Imm) override;
    AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::ImmOp Imm,
                                 AsmOperand::RegOp Reg) override;
    AsmOperand::RegOp handleSDIV(llvm::Instruction *I, AsmOperand::ImmOp Imm1,
                                 AsmOperand::ImmOp Imm2) override;

    AsmOperand::RegOp handleCALL(llvm::Instruction *I,
                                 AsmOperand::LabelOp Label) override;
    AsmOperand::RegOp handleCALL(llvm::Instruction *I, AsmOperand::RegOp Reg) override;
    AsmOperand::RegOp handleCALL(llvm::Instruction *I, AsmOperand::MemOp Mem) override;

    void handleARG(llvm::Instruction *CI, unsigned ArgNo, AsmOperand::RegOp Reg) override;
    void handleARG(llvm::Instruction *CI, unsigned ArgNo, AsmOperand::ImmOp Imm) override;
    void handleARG(llvm::Instruction *CI, unsigned ArgNo, AsmOperand::MemOp Mem) override;
    void handleARG(llvm::Instruction *CI, unsigned ArgNo,
                   AsmOperand::LabelOp Label) override;

    void handleLABEL(AsmOperand::LabelOp Label) override;

private:
    void normalizeAsmMemoryOperand(AsmOperand::MemOp &Op);

    AsmInstruction *createLDInst(AsmOperand DstReg, AsmOperand SrcMem);
    AsmInstruction *createSDInst(AsmOperand SrcReg, AsmOperand DstMem);
    AsmInstruction *createMVInst(AsmOperand DstReg, AsmOperand SrcReg);
    AsmInstruction *createLIInst(AsmOperand DstReg, AsmOperand Imm);
    AsmInstruction *createLAInst(AsmOperand DstReg, AsmOperand Label);
    AsmInstruction *createBEQInst(AsmOperand Reg1, AsmOperand Reg2, AsmOperand Label);
    AsmInstruction *createBNEInst(AsmOperand Reg1, AsmOperand Reg2, AsmOperand Label);
    AsmInstruction *createBGTInst(AsmOperand Reg1, AsmOperand Reg2, AsmOperand Label);
    AsmInstruction *createBLEInst(AsmOperand Reg1, AsmOperand Reg2, AsmOperand Label);
    AsmInstruction *createADDInst(AsmOperand DstReg, AsmOperand SrcReg1,
                                  AsmOperand SrcReg2);
    AsmInstruction *createADDIInst(AsmOperand DstReg, AsmOperand SrcReg,
                                   AsmOperand SrcImm);
    AsmInstruction *createSUBInst(AsmOperand DstReg, AsmOperand SrcReg1,
                                  AsmOperand SrcReg2);
    AsmInstruction *createMULInst(AsmOperand DstReg, AsmOperand SrcReg1,
                                  AsmOperand SrcReg2);
    AsmInstruction *createDIVInst(AsmOperand DstReg, AsmOperand SrcReg1,
                                  AsmOperand SrcReg2);
    AsmInstruction *createCALLInst(AsmOperand Callee, bool DirectCall, unsigned NumArgs);
    AsmInstruction *createLABELInst(AsmOperand LabelOp);
    AsmInstruction *createJInst(AsmOperand LabelOp);
};

}  // namespace remniw
