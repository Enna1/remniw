# LLVM IR Codegen

该 phase 的输入为 AST，输出为 IR。remnniw 编译器使用的是 [LLVM](https://www.llvm.org/) IR 的一个子集，只使用了如下 instruction：

- `ret` Instruction
- `br` Instruction
- `add` Instruction
- `sub` Instruction
- `mul` Instruction
- `sdiv` Instruction
- `alloca` instruction
- `load` Instruction
- `store` Instruction
- `icmp` Instruction
- `call` Instruction

根据 AST 生成 LLVM IR 的代码见：

- src/codegen/ir/IRCodeGenerator.h
- src/codegen/ir/IRCodeGenerator.cpp
- src/codegen/ir/IRCodeGeneratorImpl.h
- src/codegen/ir/IRCodeGeneratorImpl.cpp