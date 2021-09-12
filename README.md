# REMNIW

> **寒暑**易节，始一反焉。—— 《愚公移山》

**win**ter, sum**mer** ➡ **rem**mus,ret**niw** ➡ **remniw**

## 初衷 Intention

以往我在学习编译技术时，更多是阅读 LLVM 源码，停留在读上面，因此可能对编译原理理解不够深刻。

本项目的初衷是通过动手实践来加深理解，帮助我更好地学习编译技术（包括不限于静态分析、机器无关优化、代码生成、垃圾回收）。

目前，该项目实现了一个简单的编译器，该编译器将 remniw 语言（自己设计的一个玩具语言）源代码编译为 x64 汇编。

未来，预计会在该编译器上实现更多的内容：如垃圾回收算法、经典的机器无关优化算法等。

希望本项目对编译技术爱好者有所帮助。

因本人水平有限，代码中很多不足之处，欢迎各位以 Issue 和 Pull Request 的方式批评指正，感激不尽。

## 构建 Build

```
$ git clone git@github.com:Enna1/remniw.git
$ cd remniw
$ ./utils/bootstrap.sh
$ mkdir build
$ cmake ..
$ make
```

## 使用 Use

```
$ cat /path/to/remniw/test/more_args.rw
func f(arg1 int, arg2 int, arg3 int, arg4 int, arg5 int,
       arg6 int, arg7 int, arg8 int, arg9 int, arg10 int) int {
    return arg1 + arg2 + arg3 + arg4 + arg5 + arg6 + arg7 + arg8 + arg9 + arg10;
}

func main() int {
    output f(1,2,3,4,5,6,7,8,9,10);
    return 0;
}

# generate llvm ir
$ /path/to/remniw/build/bin/remniw /path/to/remniw/test/more_args.rw -o more_args.ll
# generate assembly code
$ /path/to/remniw/build/bin/remniw-llc more_args.ll -o more_args.s
# generate executable 
$ clang more_args.s -o more_args
# execute
$ ./more_args
55
```

## 设计 Design

remniw 编译器包含 5 个 phase：

- frontend 前端。输入为 remniw 的源代码，输出为 AST。remniw 使用 [ANTLR4](https://www.antlr.org/) 来定义 remniw 的语法、生成 Lexer 和 Parser。
- semantic analysis 语义分析。不是本项目的重点，只在 AST 上做了非常简单的 type checking。
- ir code generation 生成 LLVM IR。输入为 AST，输出为 IR。remnniw 编译器的中间表示使用的是 [LLVM](https://www.llvm.org/) IR 的一个子集，只使用 LLVM IR 的子集有一个好处，在实现程序的静态分析、优化时只需要考虑有限的 llvm instruction，这样方便我们实现算法，更专注于分析、优化算法本身，而不会陷于繁多的 llvm instruction。
- optimization 机器无关优化。输入为 LLVM IR，输出为优化后的 LLVM IR。
- asm code generation 生成汇编代码。输入为优化后的 LLVM IR，输出为 x64 汇编。

### 语法 Syntax

remniw 语言的语法设计脱胎于 [Static Program Analysis](https://cs.au.dk/~amoeller/spa/) 中的 TIP 语言，目前只支持整型、指针这两种变量类型。

下面给出一个使用 remniw 语言编写的计算斐波那契数列的程序源代码：

```
func apply(f func(int) int, a int) int {
    return f(a);
}

func fib(n int) int {
    var result int;

    if( n>1 ){
        result = fib(n-1)+fib(n-2);
    } else {
        result=1;
    }

    return result;
}

func main() int {
    output apply(fib,5);
    return 0;
}
```

### 中间表示 Intermediate Representation

remnniw 编译器的中间表示使用的是 LLVM IR 的一个子集，目前只使用了如下 instruction：

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

只使用 LLVM IR 的子集有一个好处，在实现机器无关优化时只需要考虑有限的 llvm instruction，这样方便我们更专注于分析、优化算法本身，而不会陷于繁多的 llvm instruction。

### 汇编代码 Assembly Code

本项目与其他一些基于 LLVM 实现的编译器相比，不同点在于：没有使用 LLVM 的提供的由 LLVM IR 生成汇编代码的接口，而是自己动手实现了从 LLVM IR（子集）生成 x64 汇编。

## 文档 Documents

remniw 编译器 5 个 phase 的详细文档位于 docs 目录下，这些详细文档中记录了我在实现各个 phase 时的思路及参考资料：

- [frontend](docs/FrontEnd.md)
- [semantic analysis](docs/Semantic.md)
- [ir code generation](docs/LLVMIRCodegen.md)
- [optimization](docs/Optimizer.md)
- [asm code generation](docs/AssemblyCodegen.md)

## 项目依赖 Dependency 

- [ANTLR4](https://www.antlr.org/)
- [CMake](https://cmake.org/)
- [LLVM](https://www.llvm.org/)
- [Olive code-generator generator](https://suif.stanford.edu/pub/tjiang/olive.tar.gz)

