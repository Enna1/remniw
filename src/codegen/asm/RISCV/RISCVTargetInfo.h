#pragma once

#include "codegen/asm/AsmOperand.h"
#include "codegen/asm/Register.h"
#include "codegen/asm/TargetInfo.h"
#include <algorithm>

namespace remniw {

namespace RISCV {

enum {
    NoRegister = 0,
    ZERO,  // X0. Hard-wired zero
    RA,    // X1. Return address, not preserved across function calls
    SP,    // X2. Stack pointer, preserved across function calls
    GP,    // X3. Gloabl pointer.
    TP,    // X4. Thread pointer.
    T0,    // X5. Temporary/alternate link register, not preserved across function calls
    T1,    // X6. Temporary, not preserved across function calls
    T2,    // X7. Temporary, not preserved across function calls
    FP,    // X8/S0. Saved regsiter/frame register, preserved across function calls
    S1,    // X9. Saved register, preserved across function calls
    A0,    // X10. Function argument/return-value, not preserved across function calls
    A1,    // X11. Function argument/return-value, not preserved across function calls
    A2,    // X12. Function argument, not preserved across function calls
    A3,    // X13. Function argument, not preserved across function calls
    A4,    // X14. Function argument, not preserved across function calls
    A5,    // X15. Function argument, not preserved across function calls
    A6,    // X16. Function argument, not preserved across function calls
    A7,    // X17. Function argument, not preserved across function calls
    S2,    // X18. Saved register, preserved across function calls
    S3,    // X19. Saved register, preserved across function calls
    S4,    // X20. Saved register, preserved across function calls
    S5,    // X21. Saved register, preserved across function calls
    S6,    // X22. Saved register, preserved across function calls
    S7,    // X23. Saved register, preserved across function calls
    S8,    // X24. Saved register, preserved across function calls
    S9,    // X25. Saved register, preserved across function calls
    S10,   // X26. Saved register, preserved across function calls
    S11,   // X27. Saved register, preserved across function calls
    T3,    // X28. Temporary, not preserved across function calls
    T4,    // X29. Temporary, not preserved across function calls
    T5,    // X30. Temporary, not preserved across function calls
    T6,    // X31. Temporary, not preserved across function calls
    NUM_TARGET_REGS
};

static_assert(Register::FirstPhysicalReg <= RISCV::ZERO &&
                  RISCV::NUM_TARGET_REGS <= Register::FirstStackSlot,
              "Target registers ranges check failed");

static constexpr unsigned RegisterSize = 8 /*bytes*/;

static constexpr uint32_t CallerSavedRegs[] = {
    RISCV::RA, RISCV::T0, RISCV::T1, RISCV::T2, RISCV::A0, RISCV::A1,
    RISCV::A2, RISCV::A3, RISCV::A4, RISCV::A5, RISCV::A6, RISCV::A7,
    RISCV::T3, RISCV::T4, RISCV::T5, RISCV::T6};

static constexpr uint32_t CalleeSavedRegs[] = {
    RISCV::SP, RISCV::FP, RISCV::S1, RISCV::S2, RISCV::S3,  RISCV::S4, RISCV::S5,
    RISCV::S6, RISCV::S7, RISCV::S8, RISCV::S9, RISCV::S10, RISCV::S11};

static constexpr uint32_t ArgRegs[] = {RISCV::A0, RISCV::A1, RISCV::A2, RISCV::A3,
                                       RISCV::A4, RISCV::A5, RISCV::A6, RISCV::A7};

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

}  // namespace RISCV

class RISCVTargetInfo: public TargetInfo {
public:
    unsigned getRegisterSize() const { return RISCV::RegisterSize; }

    bool isCallerSavedRegister(uint32_t Reg) const override {
        return std::any_of(begin(RISCV::CallerSavedRegs), end(RISCV::CallerSavedRegs),
                           [&](int R) { return R == Reg; });
    }

    bool isCalleeSavedRegister(uint32_t Reg) const override {
        return std::any_of(begin(RISCV::CalleeSavedRegs), end(RISCV::CalleeSavedRegs),
                           [&](int R) { return R == Reg; });
    }

    bool isArgRegister(uint32_t Reg) const override {
        return std::any_of(begin(ArgRegs), end(ArgRegs), [&](int R) { return R == Reg; });
    }

    unsigned getNumCallerSavedRegisters() const override {
        return RISCV::NumCallerSavedRegs;
    }

    unsigned getNumCalleeSavedRegisters() const override {
        return RISCV::NumCalleeSavedRegs;
    }

    unsigned getNumArgRegisters() const override { return RISCV::NumArgRegs; }

    llvm::ArrayRef<uint32_t> getCallerSavedRegisters() const override {
        return RISCV::CallerSavedRegs;
    }

    llvm::ArrayRef<uint32_t> getCalleeSavedRegisters() const override {
        return RISCV::CalleeSavedRegs;
    }

    llvm::ArrayRef<uint32_t> getArgRegisters() const override { return RISCV::ArgRegs; }

    virtual uint32_t getStackPointerRegister() const override { return RISCV::SP; }

    virtual uint32_t getFramePointerRegister() const override { return RISCV::FP; }

    // TODO: Free register priority?
    void getFreeRegistersForRegisterAllocator(
        llvm::SmallVector<bool> &FreeRegisters) const override {
        FreeRegisters.resize(RISCV::NUM_TARGET_REGS /*number of registers + 1*/);
        FreeRegisters[RISCV::NoRegister] = false;
        FreeRegisters[RISCV::ZERO] = false;
        FreeRegisters[RISCV::RA] = false;
        FreeRegisters[RISCV::SP] = false;
        FreeRegisters[RISCV::GP] = false;
        FreeRegisters[RISCV::TP] = false;
        FreeRegisters[RISCV::T0] = true;
        FreeRegisters[RISCV::T1] = true;
        FreeRegisters[RISCV::T2] = true;
        FreeRegisters[RISCV::FP] = false;
        FreeRegisters[RISCV::S1] = true;
        FreeRegisters[RISCV::A0] = true;
        FreeRegisters[RISCV::A1] = true;
        FreeRegisters[RISCV::A2] = true;
        FreeRegisters[RISCV::A3] = true;
        FreeRegisters[RISCV::A4] = true;
        FreeRegisters[RISCV::A5] = true;
        FreeRegisters[RISCV::A6] = true;
        FreeRegisters[RISCV::A7] = true;
        FreeRegisters[RISCV::S2] = true;
        FreeRegisters[RISCV::S3] = true;
        FreeRegisters[RISCV::S4] = true;
        FreeRegisters[RISCV::S5] = true;
        FreeRegisters[RISCV::S6] = true;
        FreeRegisters[RISCV::S7] = true;
        FreeRegisters[RISCV::S8] = true;
        FreeRegisters[RISCV::S9] = true;
        FreeRegisters[RISCV::S10] = true;
        FreeRegisters[RISCV::S11] = true;
        FreeRegisters[RISCV::T3] = true;
        FreeRegisters[RISCV::T4] = true;
        FreeRegisters[RISCV::T5] = true;
        FreeRegisters[RISCV::T6] = true;
    }
};

}  // namespace remniw
