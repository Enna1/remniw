#pragma once

#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <string>

namespace remniw {

class Register {
private:
    uint32_t Reg;

public:
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
};

class RegisterInfo {
public:
    virtual bool isCallerSavedRegister(uint32_t Reg) = 0;
    virtual bool isCalleeSavedRegister(uint32_t Reg) = 0;
    virtual bool isArgRegister(uint32_t Reg) = 0;

    virtual void
    getFreeRegistersForRegisterAllocator(llvm::SmallVector<bool, 32> &FreeRegisters) = 0;

    virtual std::string convertRegisterToString(uint32_t Reg) = 0;
};

}  // namespace remniw
