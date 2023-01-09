#pragma once

#include "codegen/asm/X86TargetInfo.h"
#include "codegen/asm/AsmFunction.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"

namespace remniw {

class X86AsmPrinter :public AsmPrinter {
public:
    X86AsmPrinter(const TargetInfo &TI, llvm::raw_ostream &OS,
               llvm::SmallVector<AsmFunction *> &AsmFunctions,
               llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> GVs,
               llvm::SmallVector<llvm::Function *> GlobalCtors):
        AsmPrinter(TI, OS, AsmFunctions, GVs, GlobalCtors) {}

    void EmitFunctionDeclaration(AsmFunction *F) override {
        OS << ".text\n"
           << ".globl " << F->FuncName << "\n"
           << ".type " << F->FuncName << ", @function\n"
           << F->FuncName << ":\n";
    }

    void EmitFunctionBody(AsmFunction *F) override {
        for (auto &AsmInst : *F) {
            TI.print(AsmInst, OS);
        }
    }

    void EmitGlobalVariables() override {
        for (auto p : GlobalVariables) {
            p.first->print(OS);
            OS << ":\n";
            OS << "\t.asciz ";
            OS << "\"";
            OS.write_escaped(p.second);
            OS << "\"\n";
        }
    }

    void EmitInitArray() override {
        for (auto *F : GlobalCtors) {
            OS << ".section\t.init_array,\"aw\",@init_array\n";
            OS << ".p2align\t3\n";
            OS << ".quad\t" << F->getName() << "\n";
        }
    }

    void PrintAsmInstruction(const AsmInstruction &I, llvm::raw_ostream &OS) const override {
        switch (I.getOpcode()) {
        case X86::MOV: {
            OS << "\tmovq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case X86::LEA: {
            OS << "\tleaq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            if (I.getOperand(0).isLabel() &&
                (I.getOperand(0).getLabel()->isFunction() ||
                 I.getOperand(0).getLabel()->isGlobalVariable())) {
                OS << "(%rip)";  // rip relative addressing
            }
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case X86::CMP: {
            OS << "\tcmpq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case X86::JMP: {
            OS << "\tjmp\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::JE: {
            OS << "\tje\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::JNE: {
            OS << "\tjne\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::JG: {
            OS << "\tjg\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::JLE: {
            OS << "\tjle\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::ADD: {
            OS << "\taddq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case X86::SUB: {
            OS << "\tsubq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case X86::IMUL: {
            OS << "\timulq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case X86::IDIV: {
            OS << "\tidivq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::CQTO: {
            OS << "\tcqto\n";
            break;
        }
        case X86::CALL: {
            OS << "\tcallq\t";
            if (!I.getOperand(0).isLabel())  // Indirect call
                OS << "*";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::XOR: {
            OS << "\txorq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << ", ";
            PrintAsmOperand(I.getOperand(1), OS);
            OS << "\n";
            break;
        }
        case X86::PUSH: {
            OS << "\tpushq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::POP: {
            OS << "\tpopq\t";
            PrintAsmOperand(I.getOperand(0), OS);
            OS << "\n";
            break;
        }
        case X86::RET: {
            OS << "\tretq\n";
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
            if (Op.Mem.Disp != 0)
                OS << Op.Mem.Disp;
            OS << "(";
            if (Op.Mem.BaseReg)
                OS << PrintRegister(Op.Mem.BaseReg, OS);
            if (Op.Mem.IndexReg) {
                OS << ", " << PrintRegister(Op.Mem.IndexReg, OS);
                OS << ", " << Op.Mem.Scale;
            }
            OS << ")";
            break;
        case AsmOperand::Label: Op.Lbl.Symbol->print(OS); break;
        default: llvm_unreachable("Invalid AsmOperand");
        }
    }


    void PrintRegister(uint32_t Reg, llvm::raw_ostream &OS) const override {
        switch (Reg) {
        case X86::RAX: OS << "%rax"; break;
        case X86::RBX: OS << "%rbx"; break;
        case X86::RCX: OS << "%rcx"; break;
        case X86::RDX: OS <<  "%rdx"; break;
        case X86::RSP: OS <<  "%rsp"; break;
        case X86::RBP: OS <<  "%rbp"; break;
        case X86::RDI: OS <<  "%rdi"; break;
        case X86::RSI: OS <<  "%rsi"; break;
        case X86::R8: OS <<  "%r8"; break;
        case X86::R9: OS <<  "%r9"; break;
        case X86::R10: OS <<  "%r10"; break;
        case X86::R11: OS <<  "%r11"; break;
        case X86::R12: OS <<  "%r12"; break;
        case X86::R13: OS <<  "%r13"; break;
        case X86::R14: OS <<  "%r14"; break;
        case X86::R15: OS <<  "%r15"; break;
        default: llvm_unreachable("Invalid Register\n");
        };
    }

};

}  // namespace remniw
