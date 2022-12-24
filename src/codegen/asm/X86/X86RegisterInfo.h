#pragma once

#include "Register.h"

namespace remniw {

namespace X86 {

enum {
    NoRegister = 0,
    RAX = 1,  // temporary register. with variable arguments passes information about the
              // number of vector registers used. 1st return register. not preserved
              // across function calls
    RDI =
        2,  // used to pass 1st argument to functions. not preserved across function calls
    RSI =
        3,  // used to pass 2nd argument to functions. not preserved across function calls
    RDX = 4,  // used to pass 3rd argument to functions. 2nd return register. not
              // preserved across function calls
    RCX =
        5,  // used to pass 4th argument to functions. not preserved across function calls
    R8 =
        6,  // used to pass 5th argument to functions. not preserved across function calls
    R9 =
        7,  // used to pass 6th argument to functions. not preserved across function calls
    R10 = 8,  // temporary register, used for passing a function’s static chain pointer.
              // not preserved across function calls
    R11 = 9,  // temporary register. not preserved across function calls
    RSP = 10,  // stack pointer. preserved across function calls
    RBP = 11,  // callee-saved register; optionally used as frame pointer. preserved
               // across function calls
    RBX = 12,  // callee-saved register. preserved across function calls
    R12 = 13,  // callee-saved register. preserved across function calls
    R13 = 14,  // callee-saved register. preserved across function calls
    R14 = 15,  // callee-saved register. preserved across function calls
    R15 = 16,  // callee-saved register. optionally used as GOT base pointer. preserved
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

class X86RegisterInfo {
public:
    bool isCallerSavedRegister(uint32_t Reg) {
        return X86::RAX <= Reg && Reg <= X86::R11;
    }
    bool isCalleeSavedRegister(uint32_t Reg) {
        return X86::RSP <= Reg && Reg <= X86::R15;
    }
    bool isArgRegister(uint32_t Reg) { return X86::RDI <= Reg && Reg <= X86::R9; }

    std::string convertRegisterToString(uint32_t Reg) {
        switch (Reg) {
        case Register::RAX: return "%rax";
        case Register::RBX: return "%rbx";
        case Register::RCX: return "%rcx";
        case Register::RDX: return "%rdx";
        case Register::RSP: return "%rsp";
        case Register::RBP: return "%rbp";
        case Register::RDI: return "%rdi";
        case Register::RSI: return "%rsi";
        case Register::R8: return "%r8";
        case Register::R9: return "%r9";
        case Register::R10: return "%r10";
        case Register::R11: return "%r11";
        case Register::R12: return "%r12";
        case Register::R13: return "%r13";
        case Register::R14: return "%r14";
        case Register::R15: return "%r15";
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
