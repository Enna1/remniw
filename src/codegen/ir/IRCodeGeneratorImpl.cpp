#include "IRCodeGeneratorImpl.h"
#include "AST.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

extern cl::OptionCategory RemniwCat;

cl::opt<bool> EnableAphoticShield("enable-aphotic-shield",
                                  cl::desc("Enable APHOTIC_SHIELD allocator"),
                                  cl::init(false), cl::Hidden, cl::cat(RemniwCat));

namespace remniw {

// Convert remniw::Type to corresponding llvm::Type
llvm::Type *IRCodeGeneratorImpl::REMNIWTypeToLLVMType(remniw::Type *Ty) {
    if (llvm::isa<remniw::IntType>(Ty)) {
        return llvm::Type::getInt64Ty(*TheLLVMContext) /*->getScalarType()*/;
    } else if (auto *PointerTy = llvm::dyn_cast<remniw::PointerType>(Ty)) {
        llvm::Type *PointeeTy = REMNIWTypeToLLVMType(PointerTy->getPointeeType());
        assert(PointeeTy != nullptr &&
               "The pointee type of pointer type must not be nullptr");
        return PointeeTy->getPointerTo();
    } else if (auto *ArrayTy = llvm::dyn_cast<remniw::ArrayType>(Ty)) {
        llvm::Type *ArrayElementTy = REMNIWTypeToLLVMType(ArrayTy->getElementType());
        return llvm::ArrayType::get(ArrayElementTy, ArrayTy->getNumElements());
    } else if (auto *FuncTy = llvm::dyn_cast<remniw::FunctionType>(Ty)) {
        SmallVector<llvm::Type *, 4> ParamTypes;
        for (auto *ParamType : FuncTy->getParamTypes())
            ParamTypes.push_back(REMNIWTypeToLLVMType(ParamType));
        return llvm::FunctionType::get(REMNIWTypeToLLVMType(FuncTy->getReturnType()),
                                       ParamTypes, false)
            ->getPointerTo();
    }
    llvm_unreachable("Unhandled remniw::Type");
    return nullptr;
}

// Get size in bytes required of remniw::Type.
// First convert remniw::Type to corresponding llvm::Type,
// Then get Size(bytes) of llvm::Type
uint64_t IRCodeGeneratorImpl::getSizeOfREMNIWType(remniw::Type *Ty) {
    llvm::Type *LLVMTy = REMNIWTypeToLLVMType(Ty);
    return TheModule->getDataLayout().getTypeAllocSize(LLVMTy);
}

// utility function for emit scanf, printf, etc
Value *IRCodeGeneratorImpl::emitLibCall(StringRef LibFuncName, llvm::Type *ReturnType,
                                        ArrayRef<llvm::Type *> ParamTypes,
                                        ArrayRef<Value *> Operands, bool IsVaArgs) {
    Module *M = IRB->GetInsertBlock()->getModule();
    llvm::FunctionType *FuncType =
        llvm::FunctionType::get(ReturnType, ParamTypes, IsVaArgs);
    FunctionCallee Callee = M->getOrInsertFunction(LibFuncName, FuncType);
    CallInst *CI = IRB->CreateCall(Callee, Operands);
    if (const Function *F = dyn_cast<Function>(Callee.getCallee()->stripPointerCasts()))
        CI->setCallingConv(F->getCallingConv());
    return CI;
}

Value *IRCodeGeneratorImpl::emitPrintf(Value *Fmt, Value *VAList) {
    unsigned AS = Fmt->getType()->getPointerAddressSpace();
    return emitLibCall("printf", IRB->getInt32Ty(), {IRB->getInt8PtrTy()},
                       {IRB->CreateBitCast(Fmt, IRB->getInt8PtrTy(AS), "cstr"), VAList},
                       /*IsVaArgs=*/true);
}

Value *IRCodeGeneratorImpl::emitScanf(Value *Fmt, Value *VAList) {
    unsigned AS = Fmt->getType()->getPointerAddressSpace();
    return emitLibCall("scanf", IRB->getInt32Ty(), {IRB->getInt8PtrTy()},
                       {IRB->CreateBitCast(Fmt, IRB->getInt8PtrTy(AS), "cstr"), VAList},
                       /*IsVaArgs=*/true);
}

Value *IRCodeGeneratorImpl::emitMalloc(llvm::Type *ReturnType, Value *Size) {
    if (EnableAphoticShield)
        return emitLibCall("as_alloc", ReturnType, {Size->getType()}, {Size},
                           /*IsVaArgs=*/false);
    return emitLibCall("malloc", ReturnType, {Size->getType()}, {Size},
                       /*IsVaArgs=*/false);
}

Value *IRCodeGeneratorImpl::emitFree(Value *Ptr) {
    if (EnableAphoticShield)
        return emitLibCall("as_dealloc", IRB->getVoidTy(), {Ptr->getType()}, {Ptr},
                           /*IsVaArgs=*/false);
    return emitLibCall("free", IRB->getVoidTy(), {Ptr->getType()}, {Ptr},
                       /*IsVaArgs=*/false);
}

//  Create an alloca instruction in the entry block of the function.
static AllocaInst *createEntryBlockAlloca(Function *TheFunction, Twine VarName,
                                          llvm::Type *Ty) {
    IRBuilder<> Tmp(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return Tmp.CreateAlloca(Ty, nullptr, VarName);
}

IRCodeGeneratorImpl::IRCodeGeneratorImpl(llvm::LLVMContext *LLVMContext) {
    TheLLVMContext = LLVMContext;
    TheModule = std::make_unique<Module>("", *TheLLVMContext);
    InitializeNativeTarget();
    auto TM = std::unique_ptr<TargetMachine>(EngineBuilder().selectTarget());
    assert(TM && "Cannot initialize TargetMachine");
    TheModule->setDataLayout(TM->createDataLayout());
    IRB = std::make_unique<IRBuilder<>>(*TheLLVMContext);
    InputFmtStr = IRB->CreateGlobalString("%lli", "fmtstr", 0, TheModule.get());
    OutputFmtStr = IRB->CreateGlobalString("%lli\n", "fmtstr", 0, TheModule.get());
}

Value *IRCodeGeneratorImpl::codegenExpr(ExprAST *Expr) {
    Value *Ret = nullptr;
    switch (Expr->getKind()) {
    case ASTNode::NumberExpr:
        Ret = codegenNumberExpr(static_cast<NumberExprAST *>(Expr));
        break;
    case ASTNode::VariableExpr:
        Ret = codegenVariableExpr(static_cast<VariableExprAST *>(Expr));
        break;
    case ASTNode::FunctionCallExpr:
        Ret = codegenFunctionCallExpr(static_cast<FunctionCallExprAST *>(Expr));
        break;
    case ASTNode::NullExpr:
        Ret = codegenNullExpr(static_cast<NullExprAST *>(Expr));
        break;
    case ASTNode::SizeofExpr:
        Ret = codegenSizeofExpr(static_cast<SizeofExprAST *>(Expr));
        break;
    case ASTNode::RefExpr: Ret = codegenRefExpr(static_cast<RefExprAST *>(Expr)); break;
    case ASTNode::DerefExpr:
        Ret = codegenDerefExpr(static_cast<DerefExprAST *>(Expr));
        break;
    case ASTNode::ArraySubscriptExpr:
        Ret = codegenArraySubscriptExpr(static_cast<ArraySubscriptExprAST *>(Expr));
        break;
    case ASTNode::InputExpr:
        Ret = codegenInputExpr(static_cast<InputExprAST *>(Expr));
        break;
    case ASTNode::BinaryExpr:
        Ret = codegenBinaryExpr(static_cast<BinaryExprAST *>(Expr));
        break;
    default: llvm_unreachable("Invalid expr");
    }
    return Ret;
}

Value *IRCodeGeneratorImpl::codegenStmt(StmtAST *Stmt) {
    Value *Ret = nullptr;
    switch (Stmt->getKind()) {
    case ASTNode::LocalVarDeclStmt:
        Ret = codegenLocalVarDeclStmt(static_cast<LocalVarDeclStmtAST *>(Stmt));
        break;
    case ASTNode::EmptyStmt:
        Ret = codegenEmptyStmt(static_cast<EmptyStmtAST *>(Stmt));
        break;
    case ASTNode::OutputStmt:
        Ret = codegenOutputStmt(static_cast<OutputStmtAST *>(Stmt));
        break;
    case ASTNode::AllocStmt:
        Ret = codegenAllocStmt(static_cast<AllocStmtAST *>(Stmt));
        break;
    case ASTNode::DeallocStmt:
        Ret = codegenDeallocStmt(static_cast<DeallocStmtAST *>(Stmt));
        break;
    case ASTNode::BlockStmt:
        Ret = codegenBlockStmt(static_cast<BlockStmtAST *>(Stmt));
        break;
    case ASTNode::ReturnStmt:
        Ret = codegenReturnStmt(static_cast<ReturnStmtAST *>(Stmt));
        break;
    case ASTNode::IfStmt: Ret = codegenIfStmt(static_cast<IfStmtAST *>(Stmt)); break;
    case ASTNode::WhileStmt:
        Ret = codegenWhileStmt(static_cast<WhileStmtAST *>(Stmt));
        break;
    case ASTNode::AssignmentStmt:
        Ret = codegenAssignmentStmt(static_cast<AssignmentStmtAST *>(Stmt));
        break;
    default: llvm_unreachable("Invalid stmt");
    }
    return Ret;
}

Value *IRCodeGeneratorImpl::codegenNumberExpr(NumberExprAST *NumberExpr) {
    return ConstantInt::get(IRB->getInt64Ty(), NumberExpr->getValue(), /*IsSigned=*/true);
}

Value *IRCodeGeneratorImpl::codegenVariableExpr(VariableExprAST *VariableExpr) {
    std::string Name = VariableExpr->getName().str();
    if (NamedValues.count(Name)) {
        Value *V = NamedValues[Name];
        if (VariableExpr->IsLValue()) {
            return V;
        } else {
            assert(V->getType()->isPointerTy());
            return IRB->CreateLoad(V->getType()->getPointerElementType(), V, Name);
        }
    }

    if (llvm::Function *F = TheModule->getFunction(Name)) {
        return F;
    }

    llvm_unreachable("Unknown VariableExprAST");
}

Value *IRCodeGeneratorImpl::codegenVarDeclNode(VarDeclNodeAST *VarDeclNode) {
    // We handle VarDeclNode in Function()
    return nullptr;
}

Value *
IRCodeGeneratorImpl::codegenFunctionCallExpr(FunctionCallExprAST *FunctionCallExpr) {
    Value *CalledValue = codegenExpr(FunctionCallExpr->getCallee());
    SmallVector<Value *, 4> CallArgs;
    for (auto *Arg : FunctionCallExpr->getArgs()) {
        Value *V = codegenExpr(Arg);
        CallArgs.push_back(V);
    }
    if (auto *CalledFunction = llvm::dyn_cast<llvm::Function>(CalledValue)) {
        assert(CalledFunction->arg_size() == FunctionCallExpr->getArgSize() &&
               "Incorrect #arguments passed");
        unsigned Idx = 0;
        for (auto &Arg : CalledFunction->args()) {
            assert((Arg.getType() == CallArgs[Idx++]->getType()) &&
                   "Inconsistent argument type");
        }
        return IRB->CreateCall(CalledFunction, CallArgs, "call");
    } else {
        assert(CalledValue->getType()->isPointerTy() &&
               CalledValue->getType()->getPointerElementType()->isFunctionTy());
        auto *FT =
            cast<llvm::FunctionType>(CalledValue->getType()->getPointerElementType());
        return IRB->CreateCall(FunctionCallee(FT, CalledValue), CallArgs, "call");
    }
}

// TODO
Value *IRCodeGeneratorImpl::codegenNullExpr(NullExprAST *NullExpr) {
    return nullptr;
}

Value *IRCodeGeneratorImpl::codegenSizeofExpr(SizeofExprAST *SizeofExpr) {
    uint64_t SizeInBytes = getSizeOfREMNIWType(SizeofExpr->getType());
    return ConstantInt::get(IRB->getInt64Ty(), SizeInBytes, /*IsSigned=*/true);
}

Value *IRCodeGeneratorImpl::codegenRefExpr(RefExprAST *RefExpr) {
    assert(!TheModule->getFunction(RefExpr->getVar()->getName()) &&
           "Operand of RefExpr cannot be function");
    Value *Val = codegenVariableExpr(RefExpr->getVar());
    return Val;
}

Value *IRCodeGeneratorImpl::codegenDerefExpr(DerefExprAST *DerefExpr) {
    Value *V = codegenExpr(DerefExpr->getPtr());
    assert(V && "Invalid operand of DerefExpr");
    return IRB->CreateLoad(V->getType()->getPointerElementType(), V);
}

Value *IRCodeGeneratorImpl::codegenArraySubscriptExpr(
    ArraySubscriptExprAST *ArraySubscriptExpr) {
    Value *Base = codegenExpr(ArraySubscriptExpr->getBase());
    Value *Selector = codegenExpr(ArraySubscriptExpr->getSelector());
    assert((Base && Selector) && "Invalid operand of ArraySubscriptExpr");
    assert(Base->getType()->isPointerTy());
    auto BasePointeeTy = Base->getType()->getPointerElementType();
    assert(BasePointeeTy->isArrayTy() &&
           "Base operand of ArraySubscriptExpr must be ArrayType");
    Value *Ret =
        IRB->CreateInBoundsGEP(BasePointeeTy, Base, {IRB->getInt64(0), Selector});
    if (!ArraySubscriptExpr->IsLValue()) {
        Ret = IRB->CreateLoad(Ret->getType()->getPointerElementType(), Ret);
    }
    return Ret;
}

Value *IRCodeGeneratorImpl::codegenInputExpr(InputExprAST *InputExpr) {
    llvm::Function *F = IRB->GetInsertBlock()->getParent();
    Value *Ptr = createEntryBlockAlloca(F, "input", IRB->getInt64Ty());
    Value *Call = emitScanf(InputFmtStr, Ptr);
    return IRB->CreateLoad(Ptr->getType()->getPointerElementType(), Ptr);
}

Value *IRCodeGeneratorImpl::codegenBinaryExpr(BinaryExprAST *BinaryExpr) {
    Value *V1 = codegenExpr(BinaryExpr->getLHS());
    Value *V2 = codegenExpr(BinaryExpr->getRHS());
    Value *V = nullptr;
    assert((V1 && V2) && "Invalid operand of BinaryExpr");
    switch (BinaryExpr->getOp()) {
    case BinaryExprAST::OpKind::Mul: V = IRB->CreateMul(V1, V2, "mul"); break;
    case BinaryExprAST::OpKind::Div: V = IRB->CreateSDiv(V1, V2, "div"); break;
    case BinaryExprAST::OpKind::Add: V = IRB->CreateAdd(V1, V2, "add"); break;
    case BinaryExprAST::OpKind::Sub: V = IRB->CreateSub(V1, V2, "sub"); break;
    case BinaryExprAST::OpKind::Gt: V = IRB->CreateICmpSGT(V1, V2, "icmp.sgt"); break;
    case BinaryExprAST::OpKind::Eq: V = IRB->CreateICmpEQ(V1, V2, "icmp.eq"); break;
    default: llvm_unreachable("Invalid binary operation!");
    }
    return V;
}

Value *
IRCodeGeneratorImpl::codegenLocalVarDeclStmt(LocalVarDeclStmtAST *LocalVarDeclStmt) {
    // We handle LocalVarDeclStmt in Function()
    return nullptr;
}

Value *IRCodeGeneratorImpl::codegenEmptyStmt(EmptyStmtAST *EmptyStmt) {
    // do nothing
    return nullptr;
}

Value *IRCodeGeneratorImpl::codegenOutputStmt(OutputStmtAST *OutputStmt) {
    Value *V = codegenExpr(OutputStmt->getExpr());
    assert(V && "Invalid operand of OutputStmt");
    return emitPrintf(OutputFmtStr, V);
}

Value *IRCodeGeneratorImpl::codegenAllocStmt(AllocStmtAST *AllocStmt) {
    assert(AllocStmt->getPtr()->IsLValue() && "AllocStmt first operand must be lvalue");
    Value *Ptr = codegenExpr(AllocStmt->getPtr());
    Value *Size = codegenExpr(AllocStmt->getSize());
    assert(Ptr->getType()->isPointerTy() && llvm::isa<ConstantInt>(Size) &&
           "AllocStmt first operand must be pointer type and "
           "second operand must be contant int");
    Value *Addr = emitMalloc(Ptr->getType()->getPointerElementType(), Size);
    return IRB->CreateStore(Addr, Ptr);
}

Value *IRCodeGeneratorImpl::codegenDeallocStmt(DeallocStmtAST *DeallocStmt) {
    Value *Ptr = codegenExpr(DeallocStmt->getExpr());
    assert(Ptr && Ptr->getType()->isPointerTy() && "Invalid operand of OutputStmt");
    return emitFree(Ptr);
}

Value *IRCodeGeneratorImpl::codegenBlockStmt(BlockStmtAST *BlockStmt) {
    for (auto *Stmt : BlockStmt->getStmts())
        codegenStmt(Stmt);
    return nullptr;
}

Value *IRCodeGeneratorImpl::codegenReturnStmt(ReturnStmtAST *ReturnStmt) {
    // We handle ReturnStmt in codegenFunction()
    return nullptr;
}

Value *IRCodeGeneratorImpl::codegenIfStmt(IfStmtAST *IfStmt) {
    Value *CondV = codegenExpr(IfStmt->getCond());
    assert(CondV && "Invalid condtion operand of IfStmt");
    // Convert condition to a bool by comparing non-equal to 0.
    if (!CondV->getType()->isIntegerTy(1))
        CondV = IRB->CreateICmpNE(
            CondV, ConstantInt::get(IRB->getInt64Ty(), 0, /*IsSigned=*/true), "");
    llvm::Function *F = IRB->GetInsertBlock()->getParent();
    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *ThenBB = BasicBlock::Create(*TheLLVMContext, "if.then", F);
    BasicBlock *ElseBB = BasicBlock::Create(*TheLLVMContext, "if.else");
    BasicBlock *MergeBB = BasicBlock::Create(*TheLLVMContext, "if.end");
    IRB->CreateCondBr(CondV, ThenBB, ElseBB);

    // Emit then block.
    IRB->SetInsertPoint(ThenBB);
    codegenStmt(IfStmt->getThen());
    IRB->CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    ThenBB = IRB->GetInsertBlock();
    // Emit else block.
    F->getBasicBlockList().push_back(ElseBB);
    IRB->SetInsertPoint(ElseBB);
    if (auto *Else = IfStmt->getElse())
        codegenStmt(Else);
    IRB->CreateBr(MergeBB);
    // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
    ElseBB = IRB->GetInsertBlock();

    // Emit merge block.
    F->getBasicBlockList().push_back(MergeBB);
    IRB->SetInsertPoint(MergeBB);

    return nullptr;
}

Value *IRCodeGeneratorImpl::codegenWhileStmt(WhileStmtAST *WhileStmt) {
    llvm::Function *F = IRB->GetInsertBlock()->getParent();
    // Make the new basic block for the loop header, inserting after current
    // block.
    BasicBlock *LoopCondBB = BasicBlock::Create(*TheLLVMContext, "while.cond", F);
    // Insert an explicit fall through from the current block to the LoopBB.
    IRB->CreateBr(LoopCondBB);
    // Start insertion in LoopBB.
    IRB->SetInsertPoint(LoopCondBB);

    Value *CondV = codegenExpr(WhileStmt->getCond());
    assert(CondV && "Invalid condtion operand of WhileStmt");

    // Convert condition to a bool by comparing non-equal to 0.
    if (!isa<CmpInst>(CondV))
        CondV = IRB->CreateICmpNE(
            CondV, ConstantInt::get(IRB->getInt64Ty(), 0, /*IsSigned=*/true), "");

    // Create the "loop body" block and the "loop end" block.
    BasicBlock *LoopBodyBB = BasicBlock::Create(*TheLLVMContext, "while.body");
    BasicBlock *LoopEndBB = BasicBlock::Create(*TheLLVMContext, "while.end");
    IRB->CreateCondBr(CondV, LoopBodyBB, LoopEndBB);

    // Emit the "loop body" block
    F->getBasicBlockList().push_back(LoopBodyBB);
    IRB->SetInsertPoint(LoopBodyBB);
    codegenStmt(WhileStmt->getBody());
    IRB->CreateBr(LoopCondBB);

    // Emit the "loop end" block
    F->getBasicBlockList().push_back(LoopEndBB);
    IRB->SetInsertPoint(LoopEndBB);
    return nullptr;
}

Value *IRCodeGeneratorImpl::codegenAssignmentStmt(AssignmentStmtAST *AssignmentStmt) {
    Value *Val = codegenExpr(AssignmentStmt->getRHS());
    Value *Ptr = codegenExpr(AssignmentStmt->getLHS());
    assert((Ptr && Val) && "Invalid operand of AssignmentStmt");
    return IRB->CreateStore(Val, Ptr);
}

Value *IRCodeGeneratorImpl::codegenFunction(FunctionAST *Function) {
    // Get the function from the module symbol table.
    llvm::Function *F = TheModule->getFunction(Function->getFuncName());
    assert(F && "Function is not in the module symbol table");

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(*TheLLVMContext, "entry", F);
    IRB->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map
    NamedValues.clear();
    unsigned Idx = 0;
    std::vector<VarDeclNodeAST *> ParamDecls = Function->getParamDecls();
    for (auto &Arg : F->args()) {
        Arg.setName(ParamDecls[Idx++]->getName());
        // Create an alloca for this variable.
        AllocaInst *Alloca = createEntryBlockAlloca(F, Arg.getName(), Arg.getType());
        // Store the initial value into the alloca.
        IRB->CreateStore(&Arg, Alloca);
        NamedValues[Arg.getName().str()] = Alloca;
    }

    // Create local variables declarations
    for (auto *VarDeclNode : Function->getLocalVarDecls()->getVars()) {
        Value *LocalVar = IRB->CreateAlloca(REMNIWTypeToLLVMType(VarDeclNode->getType()),
                                            nullptr, VarDeclNode->getName());
        NamedValues[LocalVar->getName().str()] = LocalVar;
    }

    // Codegen the function body
    for (auto *Stmt : Function->getBody()) {
        codegenStmt(Stmt);
    }

    // Finish off the function.
    Value *Ret = codegenExpr(Function->getReturn()->getExpr());
    // TODO: if (Function->getReturnType()->isIntType && !Ret->getType()->isIntegerTy(64))
    if (Function->getFuncName() == "main" && !Ret->getType()->isIntegerTy(64))
        Ret = IRB->CreateIntCast(Ret, IRB->getInt64Ty(), /*isSigned*/ true);
    IRB->CreateRet(Ret);

    // Validate the generated code, checking for consistency.
    verifyFunction(*F);
    return F;
}

FunctionCallee
IRCodeGeneratorImpl::createAphoticShieldCtorAndInitFunctions(StringRef CtorName,
                                                             StringRef InitName) {
    assert(!InitName.empty() && "Expected init function name");
    // Declare as_init, the aphotic_shield runtime libarary init function.
    auto *AphoticShieldInitFuncType =
        llvm::FunctionType::get(IRB->getVoidTy(), {}, false);
    FunctionCallee AphoticShieldInitFunc =
        TheModule->getOrInsertFunction(InitName, AphoticShieldInitFuncType);
    // Define module_ctor function as_module_ctor,
    // add it to global ctors that implemented as __attribute__((constructor)),
    // so as_module_ctor will be called before main function executed.
    auto *AphoticShieldCtorFuncType =
        llvm::FunctionType::get(IRB->getVoidTy(), {}, false);
    Function *AphoticShieldCtorFunc =
        llvm::Function::Create(AphoticShieldCtorFuncType, GlobalValue::InternalLinkage, 0,
                               CtorName, TheModule.get());
    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(*TheLLVMContext, "entry", AphoticShieldCtorFunc);
    IRB->SetInsertPoint(BB);
    IRB->CreateCall(AphoticShieldInitFunc, {});
    IRB->CreateRetVoid();
    appendToGlobalCtors(*TheModule, AphoticShieldCtorFunc, /*Priority*/ 65535);
    return AphoticShieldCtorFunc;
}

std::unique_ptr<Module> IRCodeGeneratorImpl::codegen(ProgramAST *AST) {
    // Add prototype for each function
    // This make emit FunctionCallExprAST easy
    for (auto *FuncAST : AST->getFunctions()) {
        SmallVector<llvm::Type *, 4> ParamTypes;
        for (auto *ParamType : FuncAST->getParamTypes())
            ParamTypes.push_back(REMNIWTypeToLLVMType(ParamType));
        auto *FT = llvm::FunctionType::get(REMNIWTypeToLLVMType(FuncAST->getReturnType()),
                                           ParamTypes, false);
        TheModule->getOrInsertFunction(FuncAST->getFuncName(), FT);
    }

    // Emit LLVM IR for all functions
    for (auto *FuncAST : AST->getFunctions())
        codegenFunction(FuncAST);

    if (EnableAphoticShield)
        createAphoticShieldCtorAndInitFunctions("as_module_ctor", "as_init");

    // Verify the generated code.
    verifyModule(*TheModule);

    return std::move(TheModule);
}

}  // namespace remniw
