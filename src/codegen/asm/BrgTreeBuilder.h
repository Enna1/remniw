#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen/asm/AsmOperand.h"
#include "codegen/asm/AsmSymbol.h"
#include "codegen/asm/Register.h"
#include "codegen/asm/TargetInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"

struct burm_state;

// To make changes to BrgTerm, make sure to synced up with
// %term defined in BrgTerm.gen
enum BrgTerm {
    Undef = 0,
    Const,
    Label,
    Reg,
    CallArgs,
    FuncArg,
#define HANDLE_INST(N, OPC, CLASS) OPC,
#include "llvm/IR/Instruction.def"
#undef HANDLE_INST
};

class BrgTreeNode {
public:
    enum KindTy {
        UndefNode,
        InstNode,
        RegNode,
        MemNode,
        ImmNode,
        LabelNode,
        CallArgsNode,  // actual call arguments
        FuncArgNode,   // formal function arguments
        AllocaNode,    // stack object
    };

private:
    burm_state *State;
    KindTy Kind;
    int Op;
    std::vector<BrgTreeNode *> Kids;
    bool ActionExecuted;
    union {
        llvm::Instruction *Inst;            // InstNode
        remniw::AsmOperand::RegOp Reg;      // RegNode
        remniw::AsmOperand::MemOp Mem;      // MemNode
        remniw::AsmOperand::ImmOp Imm;      // ImmNode
        remniw::AsmOperand::LabelOp Label;  // LabelNode
        llvm::Argument *FuncArg;            // FuncArgNode
        uint32_t StackObjectIndex;          // AllocaNode
        // CallArgsNode make use of std::vector<BrgTreeNode *> Kids
    };

    BrgTreeNode(KindTy Kind, int Op): Kind(Kind), Op(Op), ActionExecuted(false) {}

    BrgTreeNode(KindTy Kind, int Op, std::vector<BrgTreeNode *> Kids):
        Kind(Kind), Op(Op), Kids(Kids), ActionExecuted(false) {}

public:
    ~BrgTreeNode() {}

    KindTy getNodeKind() const { return Kind; }

    const char *getNodeKindString() {
        switch (Kind) {
        case UndefNode: return "UndefNode";
        case InstNode: return "InstNode";
        case RegNode: return "RegNode";
        case MemNode: return "MemNode";
        case ImmNode: return "ImmNode";
        case LabelNode: return "LabelNode";
        case CallArgsNode: return "ArgsNode";
        case FuncArgNode: return "FuncArgNode";
        case AllocaNode: return "AllocaNode";
        }
        llvm_unreachable("Invalid NodeKind");
    };

    int getOp() { return Op; }

    burm_state *getState() { return State; }

    void setState(burm_state *S) { State = S; }

    // Note: The elements of std::vector are stored contiguously, so elements
    // can be accessed using offsets to regular pointers to elements
    BrgTreeNode **getKids() { return &Kids[0]; }

    std::vector<BrgTreeNode *> getKidsVector() { return Kids; }

    void setKids(const std::vector<BrgTreeNode *> &Nodes) { Kids = Nodes; }

    bool isActionExecuted() { return ActionExecuted; }

    void setActionExecuted() { ActionExecuted = true; }

    static BrgTreeNode *getUndefNode() {
        static BrgTreeNode Undef(KindTy::UndefNode, BrgTerm::Undef);
        return &Undef;
    }

    static BrgTreeNode *createInstNode(llvm::Instruction *I,
                                       std::vector<BrgTreeNode *> Kids) {
        switch (I->getOpcode()) {
#define HANDLE_INST(NUM, OPCODE, CLASS)                                                  \
    case llvm::Instruction::OPCODE: {                                                    \
        auto *Ret = new BrgTreeNode(KindTy::InstNode, BrgTerm::OPCODE, Kids);            \
        Ret->Inst = I;                                                                   \
        return Ret;                                                                      \
    }
#include "llvm/IR/Instruction.def"
#undef HANDLE_INST
        }
        llvm_unreachable("Unknown instruction type encountered");
        return nullptr;
    }

    static BrgTreeNode *createRegNode(uint32_t RegNo) {
        auto *Ret = new BrgTreeNode(KindTy::RegNode, BrgTerm::Reg);
        Ret->Reg.RegNo = RegNo;
        return Ret;
    }

    static BrgTreeNode *createMemNode(int64_t Offset, uint32_t BaseReg,
                                      uint32_t IndexReg = remniw::Register::NoRegister,
                                      uint32_t Scale = 1) {
        auto *Ret = new BrgTreeNode(KindTy::MemNode, BrgTerm::Alloca);
        Ret->Mem.Disp = Offset;
        Ret->Mem.BaseReg = BaseReg;
        Ret->Mem.IndexReg = IndexReg;
        Ret->Mem.Scale = Scale;
        return Ret;
    }

    static BrgTreeNode *createImmNode(int64_t Val) {
        auto *Ret = new BrgTreeNode(KindTy::ImmNode, BrgTerm::Const);
        Ret->Imm.Val = Val;
        return Ret;
    }

    static BrgTreeNode *createLabelNode(remniw::AsmSymbol *Symbol) {
        auto *Ret = new BrgTreeNode(KindTy::LabelNode, BrgTerm::Label);
        Ret->Label.Symbol = Symbol;
        return Ret;
    }

    static BrgTreeNode *createCallArgsNode(std::vector<BrgTreeNode *> Kids) {
        auto *Ret = new BrgTreeNode(KindTy::CallArgsNode, BrgTerm::CallArgs, Kids);
        return Ret;
    }

    static BrgTreeNode *createFuncArgNode(llvm::Argument *FuncArg) {
        auto *Ret = new BrgTreeNode(KindTy::FuncArgNode, BrgTerm::FuncArg);
        Ret->FuncArg = FuncArg;
        return Ret;
    }

    static BrgTreeNode *createAllocaNode(uint32_t Index) {
        auto *Ret = new BrgTreeNode(KindTy::AllocaNode, BrgTerm::Alloca);
        Ret->StackObjectIndex = Index;
        return Ret;
    }

    llvm::Instruction *getInstruction() {
        assert(Kind == KindTy::InstNode && "Not a InstNode");
        return Inst;
    }

    remniw::AsmOperand::RegOp getAsAsmOperandReg() {
        assert(Kind == KindTy::RegNode && "Not a RegNode");
        return Reg;
    }

    remniw::AsmOperand::MemOp getAsAsmOperandMem() {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        return Mem;
    }

    remniw::AsmOperand::ImmOp getAsAsmOperandImm() {
        assert(Kind == KindTy::ImmNode && "Not a ImmNode");
        return Imm;
    }

    remniw::AsmOperand::LabelOp getAsAsmOperandLabel() {
        assert(Kind == KindTy::LabelNode && "Not a LabelNode");
        return Label;
    }

    llvm::Argument *getFunctionArgument() {
        assert(Kind == KindTy::FuncArgNode && "Not a FuncArgNode");
        return FuncArg;
    }

    uint32_t getRegNo() {
        assert(Kind == KindTy::RegNode && "Not a RegNode");
        return Reg.RegNo;
    }

    void setRegNo(uint32_t RegNo) {
        assert(Kind == KindTy::RegNode && "Not a RegNode");
        Reg.RegNo = RegNo;
    }

    void setReg(remniw::AsmOperand::RegOp R) {
        Kind = KindTy::RegNode;
        Reg = R;
    }

    remniw::AsmOperand::MemOp getMem() {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        return Mem;
    }

    int64_t getMemDisp() {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        return Mem.Disp;
    }

    uint32_t getMemBaseReg() {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        return Mem.BaseReg;
    }

    uint32_t getMemIndexReg() {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        return Mem.IndexReg;
    }

    uint32_t getMemScale() {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        return Mem.Scale;
    }

    void setMemDisp(int64_t Disp) {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        Mem.Disp = Disp;
    }

    void setMemBaseReg(uint32_t BaseReg) {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        Mem.BaseReg = BaseReg;
    }

    void setMemIndexReg(uint32_t IndexReg) {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        Mem.IndexReg = IndexReg;
    }

    void setMemScale(uint32_t Scale) {
        assert(Kind == KindTy::MemNode && "Not a MemNode");
        Mem.Scale = Scale;
    }

    void setMem(remniw::AsmOperand::MemOp M) {
        Kind = KindTy::MemNode;
        Mem = M;
    }

    int64_t getImmVal() {
        assert(Kind == KindTy::ImmNode && "Not a ImmNode");
        return Imm.Val;
    }

    remniw::AsmSymbol *getLabel() {
        assert(Kind == KindTy::LabelNode && "Not a LabelNode");
        return Label.Symbol;
    }

    uint32_t getStackObjectIndex() {
        assert(Kind == KindTy::AllocaNode && "Not a AllocaNode");
        return StackObjectIndex;
    }
};

typedef BrgTreeNode *NODEPTR;

#define GET_KIDS(p)     ((p)->getKids())
#define PANIC           printf
#define STATE_LABEL(p)  ((p)->getState())
#define SET_STATE(p, s) ((p)->setState(s))
#define DEFAULT_COST    break
#define OP_LABEL(p)     ((p)->getOp())
#define NO_ACTION(x)

struct COST {
    COST(int cost): cost(cost) {}
    int cost;
};

#define COST_LESS(a, b) ((a).cost < (b).cost)
static COST COST_INFINITY = COST(32767);
static COST COST_ZERO = COST(0);

static int shouldTrace = 0;
static int shouldCover = 0;

static void burm_trace(NODEPTR, int, COST);

namespace remniw {

struct BrgFunction {
    BrgFunction(llvm::Function *F): F(F) {}

    BrgFunction(const BrgFunction &) = delete;
    BrgFunction &operator=(const BrgFunction &) = delete;

    ~BrgFunction() {
        for (const auto &DM : InstToNodeMap)
            delete DM.second;
        for (const auto &DM : BasicBlockToNodeMap)
            delete DM.second;
        for (const auto &DM : ArgToNodeMap)
            delete DM.second;
        for (auto *Node : TmpArgNode)
            delete Node;
    }

    llvm::Function *F;
    int64_t LocalFrameSize {0};
    int64_t MaxCallFrameSize {0};
    llvm::SmallVector<remniw::StackObject> StackObjects;

    llvm::SmallVector<BrgTreeNode *> Insts;
    llvm::DenseMap<llvm::Instruction *, BrgTreeNode *> InstToNodeMap;
    llvm::DenseMap<llvm::BasicBlock *, BrgTreeNode *> BasicBlockToNodeMap;
    llvm::DenseMap<llvm::Argument *, BrgTreeNode *> ArgToNodeMap;
    llvm::SmallVector<BrgTreeNode *> TmpArgNode;
};

class BrgTreeBuilder: public llvm::InstVisitor<BrgTreeBuilder, BrgTreeNode *> {
private:
    llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> ConstantStrings;
    llvm::SmallVector<llvm::Function *> GlobalCtors;
    llvm::SmallVector<BrgFunction *> Functions;
    llvm::DenseMap<llvm::GlobalVariable *, BrgTreeNode *> GlobalVariableToNodeMap;
    llvm::DenseMap<llvm::Function *, BrgTreeNode *> FunctionToNodeMap;
    llvm::DenseMap<llvm::ConstantInt *, BrgTreeNode *> ConstantIntToNodeMap;
    llvm::DenseMap<uint64_t, BrgTreeNode *> ImmToNodeMap;
    const TargetInfo &TI;
    AsmContext &AsmCtx;
    BrgFunction *CurrentFunction {nullptr};
    int64_t CurrentFunctionLocalFrameSize {0};
    int64_t CurrentFunctionMaxCallFrameSize {0};
    int CurrentFunctionAllocaInstIndex {0};

public:
    BrgTreeBuilder(const TargetInfo &TI, AsmContext &AsmCtx): TI(TI), AsmCtx(AsmCtx) {}

    BrgTreeBuilder(const BrgTreeBuilder &) = delete;
    BrgTreeBuilder &operator=(const BrgTreeBuilder &) = delete;

    ~BrgTreeBuilder() {
        for (auto *F : Functions)
            delete F;
        for (const auto &DM : GlobalVariableToNodeMap)
            delete DM.second;
        for (const auto &DM : FunctionToNodeMap)
            delete DM.second;
        for (const auto &DM : ConstantIntToNodeMap)
            delete DM.second;
        for (const auto &DM : ImmToNodeMap)
            delete DM.second;
    }

    llvm::DenseMap<remniw::AsmSymbol *, llvm::StringRef> getConstantStrings() {
        return ConstantStrings;
    }

    const llvm::SmallVector<BrgFunction *> &getFunctions() { return Functions; }

    llvm::SmallVector<llvm::Function *> getGlobalCtors() { return GlobalCtors; }

    void build(llvm::Module &M) { visit(M); }

    template<class Iterator>
    void visit(Iterator Start, Iterator End) {
        while (Start != End) {
            visit(*Start++);
        }
    }

    void visit(llvm::Module &M) {
        // Handle constant strings
        for (llvm::GlobalVariable &GV : M.globals()) {
            if (llvm::ConstantDataArray *CDA =
                    llvm::dyn_cast<llvm::ConstantDataArray>(GV.getInitializer())) {
                if (CDA->isCString()) {
                    GlobalVariableToNodeMap[&GV] =
                        BrgTreeNode::createLabelNode(AsmCtx.getOrCreateSymbol(&GV));
                    ConstantStrings[AsmCtx.getOrCreateSymbol(&GV)] = CDA->getAsCString();
                }
            }
        }
        // Handle llvm.global_ctors
        findGlobalCtors(M);
        // Handle functions
        visit(M.begin(), M.end());
    }

    void visit(llvm::Function &F) {
        if (F.isDeclaration())
            return;

        Functions.push_back(new BrgFunction(&F));
        CurrentFunction = Functions.back();
        CurrentFunctionLocalFrameSize = 0;
        CurrentFunctionMaxCallFrameSize = 0;
        CurrentFunctionAllocaInstIndex = 0;

        for (unsigned i = 0, e = F.arg_size(); i != e; ++i) {
            llvm::Argument *Arg = F.getArg(i);
            llvm::Type *Ty = Arg->getType();
            uint64_t SizeInBytes = F.getParent()->getDataLayout().getTypeAllocSize(Ty);
            assert(Ty->isIntOrPtrTy() &&
                   "The Type of funtion argument must be integerType or PointerType");
            assert(SizeInBytes <= TI.getRegisterSize() &&
                   "Size of function argument must be less or equal than register size");
            auto *FuncArgNode = BrgTreeNode::createFuncArgNode(Arg);
            CurrentFunction->ArgToNodeMap[Arg] = FuncArgNode;
            if (i >= TI.getNumArgRegisters()) /* incomming arg on stack */ {
                int FuncArgOnStackIndex = TI.getNumArgRegisters() - i - 1;
                CurrentFunction->StackObjects.push_back(
                    {FuncArgOnStackIndex, SizeInBytes, /*Offset unknown*/ 0, Arg});
            }
        }

        for (auto &BB : F) {
            CurrentFunction->Insts.push_back(getBrgNodeForValue(&BB));
            for (auto &I : BB) {
                auto *InstNode = InstVisitor::visit(I);
                CurrentFunction->Insts.push_back(InstNode);
            }
        }

        CurrentFunction->LocalFrameSize = CurrentFunctionLocalFrameSize;
        CurrentFunction->MaxCallFrameSize = CurrentFunctionMaxCallFrameSize;
    }

    BrgTreeNode *visitAllocaInst(llvm::AllocaInst &AI) {
        uint64_t AllocaSizeInBytes = getAllocaSizeInBytes(AI);
        CurrentFunctionLocalFrameSize += AllocaSizeInBytes;
        CurrentFunction->StackObjects.push_back(
            {CurrentFunctionAllocaInstIndex++, AllocaSizeInBytes, 0, &AI});
        uint32_t Index = (uint32_t)CurrentFunction->StackObjects.size() - 1;
        auto *Node = BrgTreeNode::createAllocaNode(Index);
        CurrentFunction->InstToNodeMap[&AI] = Node;
        return Node;
    }

    BrgTreeNode *visitBranchInst(llvm::BranchInst &BI) {
        BrgTreeNode *InstNode = nullptr;
        if (BI.isUnconditional()) {
            InstNode = BrgTreeNode::createInstNode(
                &BI, {getBrgNodeForValue(BI.getSuccessor(0)), BrgTreeNode::getUndefNode(),
                      BrgTreeNode::getUndefNode()});
        } else {
            auto *Int1Ty = llvm::Type::getInt1Ty(BI.getContext());
            auto *ConstantIntTrue = llvm::ConstantInt::getTrue(Int1Ty);
            auto *ConstantIntFalse = llvm::ConstantInt::getFalse(Int1Ty);
            if (BI.getCondition() == ConstantIntTrue) {
                InstNode = BrgTreeNode::createInstNode(
                    &BI, {getBrgNodeForValue(BI.getSuccessor(0)),
                          BrgTreeNode::getUndefNode(), BrgTreeNode::getUndefNode()});
            } else if (BI.getCondition() == ConstantIntFalse) {
                InstNode = BrgTreeNode::createInstNode(
                    &BI, {getBrgNodeForValue(BI.getSuccessor(1)),
                          BrgTreeNode::getUndefNode(), BrgTreeNode::getUndefNode()});
            } else {
                auto *ICI = llvm::cast<llvm::ICmpInst>(BI.getCondition());
                assert(CurrentFunction->InstToNodeMap.count(ICI) &&
                       "BI.getCondition() must in InstToNodeMap");
                InstNode = BrgTreeNode::createInstNode(
                    &BI, {CurrentFunction->InstToNodeMap[ICI],
                          getBrgNodeForValue(BI.getSuccessor(0)),
                          getBrgNodeForValue(BI.getSuccessor(1))});
            }
        }
        CurrentFunction->InstToNodeMap[&BI] = InstNode;
        return InstNode;
    }

    BrgTreeNode *visitCallInst(llvm::CallInst &CI) {
        std::vector<BrgTreeNode *> Kids;
        BrgTreeNode *Args = BrgTreeNode::createCallArgsNode(
            {BrgTreeNode::getUndefNode(), BrgTreeNode::getUndefNode()});
        CurrentFunction->TmpArgNode.push_back(Args);
        BrgTreeNode *CurrentNode = Args;
        int64_t NeededStackSizeForCallArgs = 0;
        for (unsigned i = 0, e = CI.arg_size(); i != e; ++i) {
            BrgTreeNode *ArgsTmp = BrgTreeNode::createCallArgsNode(
                {BrgTreeNode::getUndefNode(), BrgTreeNode::getUndefNode()});
            CurrentFunction->TmpArgNode.push_back(ArgsTmp);
            CurrentNode->setKids({getBrgNodeForValue(CI.getArgOperand(i)), ArgsTmp});
            CurrentNode = ArgsTmp;
            if (i >= TI.getNumArgRegisters()) /* push arg on stack */ {
                llvm::Type *Ty = CI.getArgOperand(i)->getType();
                uint64_t SizeInBytes =
                    CI.getModule()->getDataLayout().getTypeAllocSize(Ty);
                NeededStackSizeForCallArgs += SizeInBytes;
            }
        }
        if (NeededStackSizeForCallArgs > CurrentFunctionMaxCallFrameSize)
            CurrentFunctionMaxCallFrameSize = NeededStackSizeForCallArgs;
        BrgTreeNode *InstNode = nullptr;
        if (auto *Callee = CI.getCalledFunction()) /*direct call*/ {
            InstNode =
                BrgTreeNode::createInstNode(&CI, {getBrgNodeForValue(Callee), Args});
        } else /* indirect call */ {
            InstNode = BrgTreeNode::createInstNode(
                &CI, {getBrgNodeForValue(CI.getCalledOperand()), Args});
        }
        CurrentFunction->InstToNodeMap[&CI] = InstNode;
        return InstNode;
    }

    BrgTreeNode *visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
        // If this is a GetElementPtrInst for ArrayType,
        // the indices size is 2, and the first index is always constant 0.
        // If this is a GetElementPtrInst for PointerType,
        // the indices size is 1.
        // That is we only care the last index
        // See IRCodeGeneratorImpl::codegenArraySubscriptExpr() for more info.
        assert(I.getNumIndices() >= 1);
        // Only care the pointer operand and last index, i.e. the first and last operand
        std::vector<BrgTreeNode *> Kids {
            getBrgNodeForValue(I.getOperand(0)),
            getBrgNodeForValue(I.getOperand(I.getNumOperands() - 1))};
        auto *InstNode = BrgTreeNode::createInstNode(&I, Kids);
        CurrentFunction->InstToNodeMap[&I] = InstNode;
        return InstNode;
    }

    BrgTreeNode *visitReturnInst(llvm::ReturnInst &I) {
        std::vector<BrgTreeNode *> Kids;
        if (llvm::Value *RetVal = I.getReturnValue()) {
            Kids.push_back(getBrgNodeForValue(RetVal));
        } else {
            Kids.push_back(BrgTreeNode::getUndefNode());
        }
        auto *InstNode = BrgTreeNode::createInstNode(&I, Kids);
        CurrentFunction->InstToNodeMap[&I] = InstNode;
        return InstNode;
    }

    // BrgTreeNode* visitLoadInst(llvm::LoadInst &I);
    // BrgTreeNode* visitStoreInst(llvm::StoreInst &I);
    // BrgTreeNode* visitICmpInst(llvm::ICmpInst &I);
    // BrgTreeNode* visitAdd(llvm::BinaryOperator &I);
    // BrgTreeNode* visitSub(llvm::BinaryOperator &I);
    // BrgTreeNode* visitMul(llvm::BinaryOperator &I);
    // BrgTreeNode* visitSDiv(llvm::BinaryOperator &I);

    /// Specify what to return for unhandled instructions.
    BrgTreeNode *visitInstruction(llvm::Instruction &I) {
        std::vector<BrgTreeNode *> Kids;
        for (unsigned i = 0, e = I.getNumOperands(); i != e; ++i) {
            Kids.push_back(getBrgNodeForValue(I.getOperand(i)));
        }
        auto *InstNode = BrgTreeNode::createInstNode(&I, Kids);
        CurrentFunction->InstToNodeMap[&I] = InstNode;
        return InstNode;
    }

private:
    BrgTreeNode *getBrgNodeForImm(uint64_t V) {
        if (!ImmToNodeMap.count(V))
            ImmToNodeMap[V] = BrgTreeNode::createImmNode(V);
        return ImmToNodeMap[V];
    }

    BrgTreeNode *getBrgNodeForValue(llvm::Value *V) {
        if (auto *CE = llvm::dyn_cast<llvm::ConstantExpr>(V)) {
            // FIXME
            return getBrgNodeForValue(CE->getOperand(0));
        } else if (auto *GV = llvm::dyn_cast<llvm::GlobalVariable>(V)) {
            auto it = GlobalVariableToNodeMap.find(GV);
            assert(it != GlobalVariableToNodeMap.end() &&
                   "GlobalVariable operand must in GlobalVariableToNodeMap");
            return it->second;
        } else if (auto *I = llvm::dyn_cast<llvm::Instruction>(V)) {
            auto it = CurrentFunction->InstToNodeMap.find(I);
            assert(it != CurrentFunction->InstToNodeMap.end() &&
                   "Instruction operand must in InstToNodeMap");
            return it->second;
        } else if (auto *Arg = llvm::dyn_cast<llvm::Argument>(V)) {
            auto it = CurrentFunction->ArgToNodeMap.find(Arg);
            assert(it != CurrentFunction->ArgToNodeMap.end() &&
                   "Argument operand must in ArgToNodeMap");
            return it->second;
        } else if (auto *F = llvm::dyn_cast<llvm::Function>(V)) {
            if (!FunctionToNodeMap.count(F)) {
                FunctionToNodeMap[F] =
                    BrgTreeNode::createLabelNode(AsmCtx.getOrCreateSymbol(F));
            }
            return FunctionToNodeMap[F];
        } else if (auto *BB = llvm::dyn_cast<llvm::BasicBlock>(V)) {
            if (!CurrentFunction->BasicBlockToNodeMap.count(BB)) {
                CurrentFunction->BasicBlockToNodeMap[BB] =
                    BrgTreeNode::createLabelNode(AsmCtx.getOrCreateSymbol(BB));
            }
            return CurrentFunction->BasicBlockToNodeMap[BB];
        } else if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(V)) {
            if (!ConstantIntToNodeMap.count(CI)) {
                ConstantIntToNodeMap[CI] = BrgTreeNode::createImmNode(CI->getSExtValue());
            }
            return ConstantIntToNodeMap[CI];
        }
        llvm_unreachable("unhandled operand");
        return nullptr;
    }

    uint64_t getAllocaSizeInBytes(const llvm::AllocaInst &AI) const {
        uint64_t ArraySize = 1;
        if (AI.isArrayAllocation()) {
            const llvm::ConstantInt *CI =
                llvm::dyn_cast<llvm::ConstantInt>(AI.getArraySize());
            assert(CI && "Non-constant array size");
            ArraySize = CI->getZExtValue();
        }
        llvm::Type *Ty = AI.getAllocatedType();
        uint64_t SizeInBytes = AI.getModule()->getDataLayout().getTypeAllocSize(Ty);
        return SizeInBytes * ArraySize;
    }

    // Find the llvm.global_ctors list, get a list of function name
    void findGlobalCtors(llvm::Module &M) {
        llvm::GlobalVariable *GV = M.getGlobalVariable("llvm.global_ctors");
        if (!GV)
            return;
        llvm::ConstantArray *CA =
            llvm::dyn_cast<llvm::ConstantArray>(GV->getInitializer());
        if (!CA)
            return;
        for (auto &V : CA->operands()) {
            auto *CS = llvm::dyn_cast<llvm::ConstantStruct>(V);
            if (!CS)
                continue;
            auto *F = llvm::dyn_cast<llvm::Function>(CS->getOperand(1));
            if (F)
                GlobalCtors.push_back(F);
        }
    }
};

}  // namespace remniw
