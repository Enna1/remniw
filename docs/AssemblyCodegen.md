# Assembly Codegen

根据 LLVM IR 生成 x64 assembly code 是基于 iburg/olive code generator generator 实现的。

寄存器分配是基于线性扫描寄存器分配算法 Linear Scan Register Allocation 实现的。

1. 关于 iburg/olive code generator generator 见
    - Engineering a simple, efficient code-generator generator. [https://dl.acm.org/doi/10.1145/151640.151642](https://dl.acm.org/doi/10.1145/151640.151642)
    - Olive. [https://suif.stanford.edu/pub/tjiang/olive.tar.gz](https://suif.stanford.edu/pub/tjiang/olive.tar.gz)
2. 关于 线性扫描寄存器分配算法 Linear Scan Register Allocation 见
    - Linear Scan Register Allocation. [https://dl.acm.org/doi/10.1145/330249.330250](https://dl.acm.org/doi/10.1145/330249.330250)
3. 关于 x86-64 psABI 见
    - [https://gitlab.com/x86-psABIs/x86-64-ABI/-/tree/master](https://gitlab.com/x86-psABIs/x86-64-ABI/-/tree/master)