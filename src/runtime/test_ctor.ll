; llc test_ctor.ll
; clang test_ctor.s -Wl,-rpath,/runtime/build -Lruntime/build -laphotic_shield ; ./a.out
; remniw-llc test_ctor.ll
; clang test_ctor.s -Wl,-rpath,/runtime/build -Lruntime/build -laphotic_shield ; ./a.out

; ModuleID = 'test_ctor.cpp'
source_filename = "test_ctor.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @as_init, i8* null }]

declare dso_local void @as_init()

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca i8*, align 8
  %ch = alloca i8, align 1
  store i32 0, i32* %retval, align 4
  %call = call i8* @as_alloc(i64 4000)
  store i8* %call, i8** %ptr, align 8
  %0 = load i8*, i8** %ptr, align 8
  %call1 = call i8* @as_dealloc(i8* %0)
  %1 = load i8*, i8** %ptr, align 8
  %2 = load i8, i8* %1, align 1
  store i8 %2, i8* %ch, align 1
  ret i32 0
}

declare dso_local i8* @as_alloc(i64)

declare dso_local i8* @as_dealloc(i8*)
