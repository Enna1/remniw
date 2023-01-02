#pragma once

#include "codegen/asm/AsmInstruction.h"
#include "codegen/asm/TargetInfo.h"
#include "llvm/Support/raw_ostream.h"

namespace remniw {

namespace X86 {

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

}  // namespace X86

class X86InstrInfo: public TargetInstrInfo {
public:
    void print(AsmInstruction &I, llvm::raw_ostream &OS) const override {
        switch (I.getOpcode()) {
        case X86::MOV: {
            OS << "\tmovq\t";
            I.getOperand(0).print(OS);
            OS << ", ";
            I.getOperand(1).print(OS);
            OS << "\n";
            break;
        }
        case X86::LEA: {
            OS << "\tleaq\t";
            I.getOperand(0).print(OS);
            if (I.getOperand(0).isLabel() &&
                (I.getOperand(0).getLabel()->isFunction() ||
                 I.getOperand(0).getLabel()->isGlobalVariable())) {
                OS << "(%rip)";  // rip relative addressing
            }
            OS << ", ";
            I.getOperand(1).print(OS);
            OS << "\n";
            break;
        }
        case X86::CMP: {
            OS << "\tcmpq\t";
            I.getOperand(0).print(OS);
            OS << ", ";
            I.getOperand(1).print(OS);
            OS << "\n";
            break;
        }
        case X86::JMP: {
            OS << "\tjmp\t";
            I.getOperand(0).print(OS);
            OS << "\n";
            break;
        }
        case X86::JE: {
            OS << "\tje\t";
            I.getOperand(0).print(OS);
            OS << "\n";
            break;
        }
        case X86::JNE: {
            OS << "\tjne\t";
            I.getOperand(0).print(OS);
            OS << "\n";
            break;
        }
        case X86::JG: {
            OS << "\tjg\t";
            I.getOperand(0).print(OS);
            OS << "\n";
            break;
        }
        case X86::JLE: {
            OS << "\tjle\t";
            I.getOperand(0).print(OS);
            OS << "\n";
            break;
        }
        case X86::ADD: {
            OS << "\taddq\t";
            I.getOperand(0).print(OS);
            OS << ", ";
            I.getOperand(1).print(OS);
            OS << "\n";
            break;
        }
        case X86::SUB: {
            OS << "\tsubq\t";
            I.getOperand(0).print(OS);
            OS << ", ";
            I.getOperand(1).print(OS);
            OS << "\n";
            break;
        }
        case X86::IMUL: {
            OS << "\timulq\t";
            I.getOperand(0).print(OS);
            OS << ", ";
            I.getOperand(1).print(OS);
            OS << "\n";
            break;
        }
        case X86::IDIV: {
            OS << "\tidivq\t";
            I.getOperand(0).print(OS);
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
            I.getOperand(0).print(OS);
            OS << "\n";
            break;
        }
        case X86::XOR: {
            OS << "\txorq\t";
            I.getOperand(0).print(OS);
            OS << ", ";
            I.getOperand(1).print(OS);
            OS << "\n";
            break;
        }
        case X86::PUSH: {
            OS << "\tpushq\t";
            I.getOperand(0).print(OS);
            OS << "\n";
            break;
        }
        case X86::POP: {
            OS << "\tpopq\t";
            I.getOperand(0).print(OS);
            OS << "\n";
            break;
        }
        case X86::RET: {
            OS << "\tretq\n";
            break;
        }
        case X86::LABEL: {
            I.getOperand(0).print(OS);
            OS << ":\n";
            break;
        }
        default: llvm_unreachable("Invalid AsmInstruction");
        }
    }
};

}  // namespace remniw
