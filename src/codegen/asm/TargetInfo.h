#pragma once

#include "AsmInstruction.h"
#include "llvm/ADT/ArrayRef.h"
#include <cstdint>

namespace remniw {

enum Target {
    x86,
    riscv
};

class TargetRegisterInfo {
public:
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

    virtual void getFreeRegistersForRegisterAllocator(
        llvm::SmallVector<bool> &FreeRegisters) const = 0;

    virtual std::string convertRegisterToString(uint32_t Reg) const = 0;
};

class TargetInstrInfo {
public:
    virtual void print(AsmInstruction &I, llvm::raw_ostream &OS) const = 0;
};

}  // namespace remniw
