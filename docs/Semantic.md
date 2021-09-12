# Semantic

remniw 编译器的语义分析极其简单，只实现了简单的类型分析。

remniw 源代码中需要显示指定变量、函数参数和返回值的类型，可以根据显示指定的类型是否满足表达式或语句所要求的操作数类型之间的关系来做简单的类型分析。

类型分析的实现参考 [Static Program Analysis](https://cs.au.dk/~amoeller/spa/) 第三章 Type Analysis 的内容。