#pragma once

#include "codegen/asm/Register.h"

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

static constexpr uint32_t CallerSavedRegs[] = {X86::RAX, X86::RDI, X86::RSI,
                                               X86::RDX, X86::RCX, X86::R8,
                                               X86::R9,  X86::R10, X86::R11};

static constexpr uint32_t CalleeSavedRegs[] = {X86::RSP, X86::RBP, X86::RBX, X86::R12,
                                               X86::R13, X86::R14, X86::R15};

static constexpr uint32_t ArgRegs[] = {X86::RDI, X86::RSI, X86::RDX,
                                       X86::RCX, X86::R8,  X86::R9};

}  // namespace X86

class X86RegisterInfo: public RegisterInfo {
public:
    bool isCallerSavedRegister(uint32_t Reg) override {
        return X86::RAX <= Reg && Reg <= X86::R11;
    }
    bool isCalleeSavedRegister(uint32_t Reg) {
        return X86::RSP <= Reg && Reg <= X86::R15;
    }
    bool isArgRegister(uint32_t Reg) { return X86::RDI <= Reg && Reg <= X86::R9; }

    void
    getFreeRegistersForRegisterAllocator(llvm::SmallVector<bool, 32> &FreeRegisters) {
        FreeRegisters.resize(16 /*number of registers*/ + 1);
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

    std::string convertRegisterToString(uint32_t Reg) {
        switch (Reg) {
        case X86::RAX: return "%rax";
        case X86::RBX: return "%rbx";
        case X86::RCX: return "%rcx";
        case X86::RDX: return "%rdx";
        case X86::RSP: return "%rsp";
        case X86::RBP: return "%rbp";
        case X86::RDI: return "%rdi";
        case X86::RSI: return "%rsi";
        case X86::R8: return "%r8";
        case X86::R9: return "%r9";
        case X86::R10: return "%r10";
        case X86::R11: return "%r11";
        case X86::R12: return "%r12";
        case X86::R13: return "%r13";
        case X86::R14: return "%r14";
        case X86::R15: return "%r15";
        };
        if (Register::isVirtualRegister(Reg))
            return std::string("%virtreg") + std::to_string(Register::virtReg2Index(Reg));
        if (Register::isStackSlot(Reg))
            return std::string("%stackslot") +
                   std::to_string(Register::stackSlot2Index(Reg));
        llvm_unreachable("Invalid Register\n");
    }
};

}  // namespace remniw
