#pragma once

#include "codegen/asm/AsmOperand.h"
#include "codegen/asm/Register.h"
#include "codegen/asm/TargetInfo.h"

namespace remniw {

namespace X86 {

enum {
    NoRegister = 0,
    RAX,  // temporary register. with variable arguments passes information about the
          // number of vector registers used. 1st return register. not preserved
          // across function calls
    RDI,  // used to pass 1st argument to functions. not preserved across function calls
    RSI,  // used to pass 2nd argument to functions. not preserved across function calls
    RDX,  // used to pass 3rd argument to functions. 2nd return register. not
          // preserved across function calls
    RCX,  // used to pass 4th argument to functions. not preserved across function calls
    R8,   // used to pass 5th argument to functions. not preserved across function calls
    R9,   // used to pass 6th argument to functions. not preserved across function calls
    R10,  // temporary register, used for passing a function’s static chain pointer.
          // not preserved across function calls
    R11,  // temporary register. not preserved across function calls
    RSP,  // stack pointer. preserved across function calls
    RBP,  // callee-saved register; optionally used as frame pointer. preserved
          // across function calls
    RBX,  // callee-saved register. preserved across function calls
    R12,  // callee-saved register. preserved across function calls
    R13,  // callee-saved register. preserved across function calls
    R14,  // callee-saved register. preserved across function calls
    R15,  // callee-saved register. optionally used as GOT base pointer. preserved
          // across function calls
    NUM_TARGET_REGS
};

static_assert(Register::FirstPhysicalReg <= X86::RAX &&
                  X86::NUM_TARGET_REGS <= Register::FirstStackSlot,
              "Target registers ranges check failed");

static constexpr unsigned RegisterSize = 8 /*bytes*/;

static constexpr uint32_t CallerSavedRegs[] = {X86::RAX, X86::RDI, X86::RSI,
                                               X86::RDX, X86::RCX, X86::R8,
                                               X86::R9,  X86::R10, X86::R11};

static constexpr uint32_t CalleeSavedRegs[] = {X86::RSP, X86::RBP, X86::RBX, X86::R12,
                                               X86::R13, X86::R14, X86::R15};

static constexpr uint32_t ArgRegs[] = {X86::RDI, X86::RSI, X86::RDX,
                                       X86::RCX, X86::R8,  X86::R9};

static constexpr unsigned NumCallerSavedRegs =
    sizeof(CallerSavedRegs) / sizeof(CallerSavedRegs[0]);

static constexpr unsigned NumCalleeSavedRegs =
    sizeof(CalleeSavedRegs) / sizeof(CalleeSavedRegs[0]);

static constexpr unsigned NumArgRegs = sizeof(ArgRegs) / sizeof(ArgRegs[0]);

enum {
    INSTRUCTION_LIST_BEGIN = 0,
    MOV,
    LEA,
    CMP,
    JMP,
    JE,
    JNE,
    JG,
    JLE,
    ADD,
    SUB,
    IMUL,
    IDIV,
    CQTO,
    CALL,
    XOR,
    PUSH,
    POP,
    RET,
    LABEL,
    INSTRUCTION_LIST_END
};

}  // namespace X86

class X86TargetInfo: public TargetInfo {
public:
    unsigned getRegisterSize() const { return X86::RegisterSize; }

    bool isCallerSavedRegister(uint32_t Reg) const override {
        return X86::RAX <= Reg && Reg <= X86::R11;
    }

    bool isCalleeSavedRegister(uint32_t Reg) const override {
        return X86::RSP <= Reg && Reg <= X86::R15;
    }

    bool isArgRegister(uint32_t Reg) const override {
        return X86::RDI <= Reg && Reg <= X86::R9;
    }

    unsigned getNumCallerSavedRegisters() const override {
        return X86::NumCallerSavedRegs;
    }

    unsigned getNumCalleeSavedRegisters() const override {
        return X86::NumCalleeSavedRegs;
    }

    unsigned getNumArgRegisters() const override { return X86::NumArgRegs; }

    llvm::ArrayRef<uint32_t> getCallerSavedRegisters() const override {
        return X86::CallerSavedRegs;
    }

    llvm::ArrayRef<uint32_t> getCalleeSavedRegisters() const override {
        return X86::CalleeSavedRegs;
    }

    llvm::ArrayRef<uint32_t> getArgRegisters() const override { return X86::ArgRegs; }

    virtual uint32_t getStackPointerRegister() const override { return X86::RSP; }

    virtual uint32_t getFramePointerRegister() const override { return X86::RBP; }

    void getFreeRegistersForRegisterAllocator(
        llvm::SmallVector<bool> &FreeRegisters) const override {
        FreeRegisters.resize(X86::NUM_TARGET_REGS /*number of registers + 1*/);
        FreeRegisters[X86::NoRegister /*0*/] = false;
        // Caller saved registers
        FreeRegisters[X86::RAX /*1*/] = true;
        FreeRegisters[X86::RDI /*2*/] = true;
        FreeRegisters[X86::RSI /*3*/] = true;
        FreeRegisters[X86::RDX /*4*/] = true;
        FreeRegisters[X86::RCX /*5*/] = true;
        FreeRegisters[X86::R8 /*6*/] = true;
        FreeRegisters[X86::R9 /*7*/] = true;
        FreeRegisters[X86::R10 /*8*/] = true;
        FreeRegisters[X86::R11 /*9*/] = true;
        // Callee saved registers
        FreeRegisters[X86::RSP /*10*/] = false;
        FreeRegisters[X86::RBP /*11*/] = false;
        FreeRegisters[X86::RBX /*12*/] = true;
        FreeRegisters[X86::R12 /*13*/] = true;
        FreeRegisters[X86::R13 /*14*/] = true;
        FreeRegisters[X86::R14 /*15*/] = true;
        FreeRegisters[X86::R15 /*16*/] = true;
    }
};

}  // namespace remniw
