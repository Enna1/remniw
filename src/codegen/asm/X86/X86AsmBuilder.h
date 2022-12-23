#pragma once

#include "AsmBuilder.h"

class X86AsmBuilder: public AsmBuilder {
public:

    virtual uint32_t handleLOAD(AsmOperand::MemOp Mem) override {
        uint32_t VirtReg = remniw::Register::createVirtReg();
        createMov(Mem, remniw::AsmOperand::createReg(VirtReg));
        return VirtReg;
    }

    virtual uint32_t handleLOAD(AsmOperand::RegOp Reg) override {
        uint32_t VirtReg = remniw::Register::createVirtReg();
        Builder->createMov(remniw::AsmOperand::createMem(0, Reg),
                        remniw::AsmOperand::createReg(VirtReg));
        return VirtReg;
    }    

    virtual void handleRET(AsmOperand::RegOp Reg) override {
        createMov(Reg, remniw::AsmOperand::createReg(remniw::X86::RAX));
    }

    virtual void handleRET(AsmOperand::ImmOp Imm) override {
        createMov(Imm, remniw::AsmOperand::createReg(remniw::X86::RAX));
    }

};
