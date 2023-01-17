#pragma once

#include "codegen/asm/AsmFunction.h"
#include "codegen/asm/RISCV/RISCVTargetInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"

namespace remniw {

class RISCVAsmPrinter: public AsmPrinter {
public:
    RISCVAsmPrinter(const TargetInfo &TI): AsmPrinter(TI) {}

    void emitFunctionDeclaration(const AsmFunction *F) override {
        auto &OS = outStreamer();
        OS << ".text\n"
           << ".globl " << F->FuncName << "\n"
           << ".type " << F->FuncName << ", @function\n"
           << F->FuncName << ":\n";
    }

    void emitFunctionBody(const AsmFunction *F) override {
        auto &OS = outStreamer();
        for (auto &AsmInst : *F) {
            PrintAsmInstruction(AsmInst, OS);
        }
    }

    void emitGlobalVariables(
        const llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> &GVs) override {
        auto &OS = outStreamer();
        for (auto p : GVs) {
            p.first->print(OS);
            OS << ":\n";
            OS << "\t.asciz ";
            OS << "\"";
            OS.write_escaped(p.second);
            OS << "\"\n";
        }
    }

    void emitInitArray(const llvm::SmallVector<llvm::Function *> &GlobalCtors) override {
        auto &OS = outStreamer();
        for (auto *F : GlobalCtors) {
            OS << ".section\t.init_array,\"aw\",@init_array\n";
            OS << ".p2align\t3\n";
            OS << ".quad\t" << F->getName() << "\n";
        }
    }

    void PrintAsmInstruction(const AsmInstruction &I,
                             llvm::raw_ostream &OS) const override {
        switch (I.getOpcode()) {
        case RISCV::LD: {
            OS << "\tld\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case RISCV::SD: {
            OS << "\tsd\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case RISCV::MV: {
            OS << "\tmv\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case RISCV::LI: {
            OS << "\tli\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case RISCV::LA: {
            OS << "\tla\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case RISCV::BEQ: {
            OS << "\tbeq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(2), OS);
            OS << "\n";
            break;
        }
        case RISCV::BNE: {
            OS << "\tbne\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(2), OS);
            OS << "\n";
            break;
        }
        case RISCV::BGT: {
            OS << "\tbgt\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(2), OS);
            OS << "\n";
            break;
        }
        case RISCV::BLE: {
            OS << "\tble\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(2), OS);
            OS << "\n";
            break;
        }
        case RISCV::ADD: {
            OS << "\tadd\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(2), OS);
            OS << "\n";
            break;
        }
        case RISCV::ADDI: {
            OS << "\taddi\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(2), OS);
            OS << "\n";
            break;
        }
        case RISCV::SUB: {
            OS << "\tsub\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(2), OS);
            OS << "\n";
            break;
        }
        case RISCV::MUL: {
            OS << "\tmul\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(2), OS);
            OS << "\n";
            break;
        }
        case RISCV::DIV: {
            OS << "\tdiv\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(2), OS);
            OS << "\n";
            break;
        }
        case X86::CALL: {
            OS << "\tcall\t";
            if (!I.getOperand(0).isLabel())  // Indirect call
                OS << "*";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::RET: {
            OS << "\tret\n";
            break;
        }
        case X86::LABEL: {
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ":\n";
            break;
        }
        default: llvm_unreachable("Invalid AsmInstruction");
        }
    }

    void PrintAsmOperand(const AsmOperand &Op, llvm::raw_ostream &OS) const override {
        switch (Op.Kind) {
        case AsmOperand::Register: PrintRegister(Op.Reg.RegNo, OS); break;
        case AsmOperand::Immediate: OS << "$" << Op.Imm.Val; break;
        case AsmOperand::Memory:
            OS << Op.Mem.Disp;
            OS << "(";
            PrintRegister(Op.Mem.BaseReg, OS);
            OS << ")";
            break;
        case AsmOperand::Label: Op.Lbl.Symbol->print(OS); break;
        default: llvm_unreachable("Invalid AsmOperand");
        }
    }

    void PrintRegister(uint32_t Reg, llvm::raw_ostream &OS) const override {
        switch (Reg) {
        case RISCV::ZERO: OS << "zero"; break;
        case RISCV::RA: OS << "ra"; break;
        case RISCV::SP: OS << "sp"; break;
        case RISCV::GP: OS << "gp"; break;
        case RISCV::TP: OS << "tp"; break;
        case RISCV::T0: OS << "t0"; break;
        case RISCV::T1: OS << "t1"; break;
        case RISCV::T2: OS << "t2"; break;
        case RISCV::FP: OS << "fp"; break;
        case RISCV::S1: OS << "s1"; break;
        case RISCV::A0: OS << "a0"; break;
        case RISCV::A1: OS << "a1"; break;
        case RISCV::A2: OS << "a2"; break;
        case RISCV::A3: OS << "a3"; break;
        case RISCV::A4: OS << "a4"; break;
        case RISCV::A5: OS << "a5"; break;
        case RISCV::A6: OS << "a6"; break;
        case RISCV::A7: OS << "a7"; break;
        case RISCV::S2: OS << "s2"; break;
        case RISCV::S3: OS << "s3"; break;
        case RISCV::S4: OS << "s4"; break;
        case RISCV::S5: OS << "s5"; break;
        case RISCV::S6: OS << "s6"; break;
        case RISCV::S7: OS << "s7"; break;
        case RISCV::S8: OS << "s8"; break;
        case RISCV::S9: OS << "s9"; break;
        case RISCV::S10: OS << "s10"; break;
        case RISCV::S11: OS << "s11"; break;
        case RISCV::T3: OS << "t3"; break;
        case RISCV::T4: OS << "t4"; break;
        case RISCV::T5: OS << "t5"; break;
        case RISCV::T6: OS << "t6"; break;
        default: llvm_unreachable("Invalid Register\n");
        };
    }
};

}  // namespace remniw
