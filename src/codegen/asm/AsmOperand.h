#pragma once

#include "AsmSymbol.h"
#include "Register.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>
#include <memory>

namespace remniw {

struct AsmOperand {
public:
    enum KindTy
    {
        Register,
        Memory,
        Immediate,
        Label
    } Kind;

    struct RegOp {
        uint32_t RegNo;
    };

    struct MemOp {
        int64_t Disp;
        uint32_t BaseReg;
        uint32_t IndexReg;
        uint32_t Scale;
    };

    struct ImmOp {
        int64_t Val;
    };

    struct LabelOp {
        AsmSymbol* Symbol;
    };

    union {
        struct RegOp Reg;
        struct MemOp Mem;
        struct ImmOp Imm;
        struct LabelOp Lbl;
    };

    AsmOperand(KindTy Kind): Kind(Kind) {}

    static std::unique_ptr<AsmOperand> createReg(uint32_t RegNo) {
        auto Res = std::make_unique<AsmOperand>(KindTy::Register);
        Res->Reg.RegNo = RegNo;
        return Res;
    }

    static std::unique_ptr<AsmOperand> createImm(int64_t Val) {
        auto Res = std::make_unique<AsmOperand>(KindTy::Immediate);
        Res->Imm.Val = Val;
        return Res;
    }

    static std::unique_ptr<AsmOperand> createMem(int64_t Disp,
                                                 uint32_t BaseReg = Register::RBP,
                                                 uint32_t IndexReg = Register::NoRegister,
                                                 uint32_t Scale = 1) {
        auto Res = std::make_unique<AsmOperand>(KindTy::Memory);
        Res->Mem.Disp = Disp;
        Res->Mem.BaseReg = BaseReg;
        Res->Mem.IndexReg = IndexReg;
        Res->Mem.Scale = Scale;
        return Res;
    }

    static std::unique_ptr<AsmOperand> createLabel(AsmSymbol* Symbol) {
        auto Res = std::make_unique<AsmOperand>(KindTy::Label);
        Res->Lbl.Symbol = Symbol;
        return Res;
    }

    std::unique_ptr<AsmOperand> clone() { return std::make_unique<AsmOperand>(*this); }

    bool isReg() const { return Kind == Register; }

    bool isVirtReg() const {
        return Kind == Register && Register::isVirtualRegister(Reg.RegNo);
    }

    bool isPhysReg() const {
        return Kind == Register && Register::isPhysicalRegister(Reg.RegNo);
    }

    bool isStackSlotReg() const {
        return Kind == Register && Register::isStackSlot(Reg.RegNo);
    }

    bool isMem() const { return Kind == Memory; }

    bool isImm() const { return Kind == Immediate; }

    bool isLabel() const { return Kind == Label; }

    uint32_t getReg() const {
        assert(Kind == Register && "Not a Register AsmOperand");
        return Reg.RegNo;
    }

    uint32_t getMemDisp() const {
        assert(Kind == Memory && "Not a Memory AsmOperand");
        return Mem.Disp;
    }

    uint32_t getMemBaseReg() const {
        assert(Kind == Memory && "Not a Memory AsmOperand");
        return Mem.BaseReg;
    }

    uint32_t getMemIndexReg() const {
        assert(Kind == Memory && "Not a Memory AsmOperand");
        return Mem.IndexReg;
    }

    uint32_t getMemScale() const {
        assert(Kind == Memory && "Not a Memory AsmOperand");
        return Mem.Scale;
    }

    AsmSymbol* getLabel() const {
        assert(Kind == Label && "Not a Label AsmOperand");
        return Lbl.Symbol;
    }

    void print(llvm::raw_ostream& OS) const {
        switch (Kind) {
        case Register: OS << Register::convertRegisterToString(Reg.RegNo); break;
        case Immediate: OS << "$" << Imm.Val; break;
        case Memory:
            if (Mem.Disp != 0)
                OS << Mem.Disp;
            OS << "(";
            if (Mem.BaseReg)
                OS << Register::convertRegisterToString(Mem.BaseReg);
            if (Mem.IndexReg) {
                OS << ", " << Register::convertRegisterToString(Mem.IndexReg);
                OS << ", " << Mem.Scale;
            }
            OS << ")";
            break;
        case Label: Lbl.Symbol->print(OS); break;
        default: llvm_unreachable("Invalid AsmOperand");
        }
    }
};

}  // namespace remniw
