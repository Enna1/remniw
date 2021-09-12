#pragma once

#include "llvm/Support/ErrorHandling.h"
#include <cassert>

namespace remniw {

/*
- %rax:
    temporary register; with variable arguments passes information about the
        number of vector registers used; 1st return register
    not preserved across function calls

- %rbx: callee-saved register. preserved across function calls

- %rcx: used to pass 4th integer argument to functions.
        not preserved across function calls

- %rdx: used to pass 3rd argument to functions; 2nd return register.
        not preserved across function calls

- %rsp: stack pointer, preserved across function calls

- %rbp: callee-saved register; optionally used as frame pointer
        preserved across function calls

- %rsi: used to pass 2nd argument to functions
        not preserved across function calls

- %rdi: used to pass 1st argument to functions
        not preserved across function calls

- %r8: used to pass 5th argument to functions
       not preserved across function calls

- %r9: used to pass 6th argument to functions
       not preserved across function calls

- %r10: temporary register, used for passing a function’s static chain pointer
        not preserved across function calls

- %r11: temporary register
        not preserved across function calls

- %r12-r14: callee-saved registers
            preserved across function calls

- %r15: callee-saved register; optionally used as GOT base pointer
        preserved across function calls
*/

class Register {
private:
    uint32_t Reg;

public:
    enum
    {
        RAX = 1,
        RDI = 2,
        RSI = 3,
        RDX = 4,
        RCX = 5,
        R8 = 6,
        R9 = 7,
        R10 = 8,
        R11 = 9,
        RSP = 10,
        RBP = 11,
        RBX = 12,
        R12 = 13,
        R13 = 14,
        R14 = 15,
        R15 = 16,
    };

    static constexpr uint32_t CallerSavedRegs[] = {
        Register::RAX, Register::RDI, Register::RSI, Register::RDX, Register::RCX,
        Register::R8,  Register::R9,  Register::R10, Register::R11};

    static constexpr uint32_t CalleeSavedRegs[] = {
        Register::RSP, Register::RBP, Register::RBX, Register::R12,
        Register::R13, Register::R14, Register::R15};

    static constexpr uint32_t ArgRegs[] = {Register::RDI, Register::RSI, Register::RDX,
                                           Register::RCX, Register::R8,  Register::R9};

    // Register numbers can represent physical registers, virtual registers, and
    // sometimes stack slots. The unsigned values are divided into these ranges:
    //   0           Not a register, can be used as a sentinel.
    //   [1;2^30)    Physical registers.
    //   [2^30;2^31) Stack slots. (Rarely used.)
    //   [2^31;2^32) Virtual registers.
    static constexpr uint32_t NoRegister = 0u;
    static constexpr uint32_t FirstPhysicalReg = 1u;
    static constexpr uint32_t FirstStackSlot = 1u << 30;
    static constexpr uint32_t VirtualRegFlag = 1u << 31;

public:
    constexpr Register(uint32_t Val = 0): Reg(Val) {}
    constexpr operator uint32_t() const { return Reg; }

    uint32_t getReg() { return Reg; }

    static bool isStackSlot(uint32_t Reg) {
        return FirstStackSlot <= Reg && Reg < VirtualRegFlag;
    }

    static bool isPhysicalRegister(uint32_t Reg) {
        return FirstPhysicalReg <= Reg && Reg < FirstStackSlot;
    }

    static bool isVirtualRegister(uint32_t Reg) {
        return Reg & VirtualRegFlag && !isStackSlot(Reg);
    }

    static bool isCallerSavedRegister(uint32_t Reg) { return RAX <= Reg && Reg <= R11; }

    static bool isCalleeSavedRegister(uint32_t Reg) { return RSP <= Reg && Reg <= R15; }

    static bool isArgRegister(uint32_t Reg) { return RDI <= Reg && Reg <= R9; }

    /// Compute the frame index from a register value representing a stack slot.
    static int stackSlot2Index(Register Reg) {
        assert(isStackSlot(Reg) && "Not a stack slot");
        return int(Reg - FirstStackSlot);
    }

    /// Convert a non-negative frame index to a stack slot register value.
    static Register index2StackSlot(int FI) {
        assert(FI >= 0 && "Cannot hold a negative frame index.");
        return Register(FI + FirstStackSlot);
    }

    static uint32_t virtReg2Index(Register Reg) {
        assert(isVirtualRegister(Reg) && "Not a virtual register");
        return Reg & ~VirtualRegFlag;
    }

    /// Convert a 0-based index to a virtual register number.
    /// This is the inverse operation of VirtReg2IndexFunctor below.
    static Register index2VirtReg(uint32_t Index) {
        assert(Index < (1u << 31) && "Index too large for virtual register range.");
        return Index | VirtualRegFlag;
    }

    static Register createVirtReg() {
        static uint32_t VirtRegIndex = 0;
        return index2VirtReg(VirtRegIndex++);
    }

    static std::string convertRegisterToString(uint32_t Reg) {
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
        if (isVirtualRegister(Reg))
            return std::string("%virtreg") + std::to_string(virtReg2Index(Reg));
        if (isStackSlot(Reg))
            return std::string("%stackslot") + std::to_string(stackSlot2Index(Reg));
        llvm_unreachable("Invalid Register\n");
    }
};

}  // namespace remniw
