#pragma once

#include "codegen/asm/AsmInstruction.h"
#include "llvm/ADT/ArrayRef.h"
#include <cstdint>

namespace remniw {

enum Target {
    x86,
    riscv
};

class TargetInfo {
public:
    virtual ~TargetInfo() = default;

    virtual unsigned getRegisterSize() const = 0;

    virtual bool isCallerSavedRegister(uint32_t Reg) const = 0;
    virtual bool isCalleeSavedRegister(uint32_t Reg) const = 0;
    virtual bool isArgRegister(uint32_t Reg) const = 0;

    virtual unsigned getNumCallerSavedRegisters() const = 0;
    virtual unsigned getNumCalleeSavedRegisters() const = 0;
    virtual unsigned getNumArgRegisters() const = 0;

    virtual llvm::ArrayRef<uint32_t> getCallerSavedRegisters() const = 0;
    virtual llvm::ArrayRef<uint32_t> getCalleeSavedRegisters() const = 0;
    virtual llvm::ArrayRef<uint32_t> getArgRegisters() const = 0;

    virtual uint32_t getStackPointerRegister() const = 0;
    virtual uint32_t getFramePointerRegister() const = 0;

    virtual llvm::ArrayRef<uint32_t> getFreeRegistersForRegisterAllocator() const = 0;
};

}  // namespace remniw
