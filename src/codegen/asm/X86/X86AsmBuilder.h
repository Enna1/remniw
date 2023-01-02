#pragma once

#include "X86InstrInfo.h"
#include "X86RegisterInfo.h"
#include "codegen/asm/AsmBuilder.h"
#include "codegen/asm/AsmInstruction.h"
#include "codegen/asm/AsmOperand.h"

namespace remniw {

class X86AsmBuilder: public AsmBuilder {
private:
    X86RegisterInfo RI;
    X86InstrInfo II;

public:
    const TargetRegisterInfo &getTargetRegisterInfo() const override { return RI; }
    const TargetInstrInfo &getTargetInstrInfo() const override { return II; }

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

    void handleARG(unsigned ArgNo, AsmOperand::RegOp Reg) override;
    void handleARG(unsigned ArgNo, AsmOperand::ImmOp Imm) override;
    void handleARG(unsigned ArgNo, AsmOperand::MemOp Mem) override;
    void handleARG(unsigned ArgNo, AsmOperand::LabelOp Label) override;

    void handleLABEL(AsmOperand::LabelOp Label) override;

private:
    AsmInstruction *createMOVInst(AsmOperand Src, AsmOperand Dst);
    AsmInstruction *createLEAInst(AsmOperand Src, AsmOperand Dst);
    AsmInstruction *createCMPInst(AsmOperand Src, AsmOperand Dst);
    AsmInstruction *createJMPInst(unsigned JmpOpcode, AsmOperand Op);
    AsmInstruction *createADDInst(AsmOperand Src, AsmOperand Dst);
    AsmInstruction *createSUBInst(AsmOperand Src, AsmOperand Dst);
    AsmInstruction *createIMULInst(AsmOperand Src, AsmOperand Dst);
    AsmInstruction *createIDIVInst(AsmOperand Op);
    AsmInstruction *createCQTOInst();
    AsmInstruction *createCALLInst(AsmOperand Callee, bool DirectCall, unsigned NumArgs);
    AsmInstruction *createXORInst(AsmOperand Src, AsmOperand Dst);
    AsmInstruction *createLABELInst(AsmOperand LabelOp);
};

}  // namespace remniw
