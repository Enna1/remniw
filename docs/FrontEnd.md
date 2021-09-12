# FrontEnd

FrontEnd 的输入为 remniw 的源代码，输出为 AST。FrontEnd 可以再分为三个阶段（见src/frontend/FrontEnd.cpp）：

- Lexer
- Parser
- ASTBuilder

remniw 编译器前端使用 [ANTLR4](https://www.antlr.org/) 来定义 remniw 的语法，ANTLR 会生成 Lexer 和 Parser 代码。

remniw 的语法脱胎自 TIP 语言的语法，remniw 的语法定义见 grammar/Remniw.g4，TIP 的语法定义见 [https://github.com/matthewbdwyer/tipc/blob/main/tipg4/TIP.g4](https://github.com/matthewbdwyer/tipc/blob/main/tipg4/TIP.g4)。

如何使用 ANTLR4 定义语法可以参考 [https://github.com/antlr/grammars-v4](https://github.com/antlr/grammars-v4)，使用 ANTLR4 定义了非常多的编程语言的语法。

有了通过 ANTLR4 定义的 remniw 语法，就可以通过 antlr4 生成对应的 Lexer, Parser 和 Visitor，这部分可以参考 [https://github.com/antlr/antlr4/blob/master/doc/cpp-target.md](https://github.com/antlr/antlr4/blob/master/doc/cpp-target.md) 和 [https://github.com/antlr/antlr4/tree/master/runtime/Cpp/cmake](https://github.com/antlr/antlr4/tree/master/runtime/Cpp/cmake)。

用于构建 AST 的类 ASTBuilder(src/frontend/ASTBuilder.h) 就是继承自 ANTLR 生成的 RemniwBaseVisitor 类来实现的。