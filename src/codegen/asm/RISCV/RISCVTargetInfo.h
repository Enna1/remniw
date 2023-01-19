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

static constexpr uint32_t FreeRegs[] = {
    RISCV::T0,  RISCV::T1, RISCV::T2, RISCV::S1, RISCV::A0, RISCV::A1, RISCV::A2,
    RISCV::A3,  RISCV::A4, RISCV::A5, RISCV::A6, RISCV::A7, RISCV::S2, RISCV::S3,
    RISCV::S4,  RISCV::S5, RISCV::S6, RISCV::S7, RISCV::S8, RISCV::S9, RISCV::S10,
    RISCV::S11, RISCV::T3, RISCV::T4, RISCV::T5, RISCV::T6};

static constexpr unsigned NumCallerSavedRegs =
    sizeof(CallerSavedRegs) / sizeof(CallerSavedRegs[0]);

static constexpr unsigned NumCalleeSavedRegs =
    sizeof(CalleeSavedRegs) / sizeof(CalleeSavedRegs[0]);

static constexpr unsigned NumArgRegs = sizeof(ArgRegs) / sizeof(ArgRegs[0]);

enum {
    INSTRUCTION_LIST_BEGIN = 0,

    // Load and Store instructions
    LD,  // Fetch of 64-bit value from memory and loads into the destination register.
         // Syntax: ld rd, offset(rs1)
    SD,  // Copy of 64-bit value from register and loads into the memory. Syntax: sd rs2,
         // offset(rs1)
    MV,  // Pseudo Instruction. Copy contents of one register to another. Syntax: mv rd,
         // rs1
    LI,  // Pseudo Instruction. Load a register (rd) with an immeidate value given.
         // Syntax: li rd, CONSTANT
    LA,  // Pseudo Instruction. Load the location address of the specified SYMBOL. Syntax:
         // la rd, SYMBOL

    // Branch Instructions
    BEQ,  // Compare the contents of source register with source register rs2, if found
          // equal, the control is transferred to the specified label. Syntax: beq rs1,
          // rs2, label
    BNE,  // Compare the contents of source register with source register rs2, if they are
          // not equal control is transferred to the label as mentioned. Syntax: bne rs1,
          // rs2, label
    BGT,  // Shift the program counter to the specified location if the value in a
          // register is greater than that of another. Syntax: bgt rs1, rs2, label. Pseudo
          // Instruction
    BLE,  // Shift the program counter to the specified location if the value in a
          // register is less than or equal to that of another. Syntax: ble rs1, rs2,
          // label. Pseudo Instruction
    J,    // Pseudo Instruction. Plain unconditional jump instruction used to jump to
          // anywhere in the code memory.

    // Arithmetic Instructions
    ADD,   // Add the contents of two registers and stores the result in another register.
           // Syntax: add rd, rs1, rs2
    ADDI,  // Add content of the source registers rs1, immediate data (imm) and store the
           // result in the destination register (rd). Syntax: addi rd, rs1, imm
    SUB,   // Subtract contents of one register from another and stores the result in
           // another register. Syntax: sub rd, rs1, rs2
    MUL,   // calculates the product of the multiplier in source register 1 (rs1) and
           // multiplicand in source register 2 (rs2), with the resulting product being
           // stored in destination register (rd). Syntax: mul rd, rs1, rs2
    DIV,   // divide on the value in source register (rs1) with the value in the source
          // register (rs2) and stores quotient in (rd) register. Syntax: div rd, rs1, rs2

    // Others
    CALL,
    JALR,
    RET,
    LABEL,
    INSTRUCTION_LIST_END
};

}  // namespace RISCV

class RISCVTargetInfo: public TargetInfo {
public:
    unsigned getRegisterSize() const { return RISCV::RegisterSize; }

    bool isCallerSavedRegister(uint32_t Reg) const override {
        return std::any_of(std::begin(RISCV::CallerSavedRegs),
                           std::end(RISCV::CallerSavedRegs),
                           [&](int R) { return R == Reg; });
    }

    bool isCalleeSavedRegister(uint32_t Reg) const override {
        return std::any_of(std::begin(RISCV::CalleeSavedRegs),
                           std::end(RISCV::CalleeSavedRegs),
                           [&](int R) { return R == Reg; });
    }

    bool isArgRegister(uint32_t Reg) const override {
        return std::any_of(std::begin(RISCV::ArgRegs), std::end(RISCV::ArgRegs),
                           [&](int R) { return R == Reg; });
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
    llvm::ArrayRef<uint32_t> getFreeRegistersForRegisterAllocator() const override {
        return RISCV::FreeRegs;
    }
};

}  // namespace remniw
