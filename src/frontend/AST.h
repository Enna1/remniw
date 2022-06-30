#pragma once

#include "Type.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

namespace remniw {

template<typename T>
std::vector<T *> rawPtrs(const std::vector<std::unique_ptr<T>> &v) {
    std::vector<T *> r;
    std::transform(v.begin(), v.end(), std::back_inserter(r),
                   [](auto &up) { return up.get(); });
    return r;
}

struct SourceLocation {
    size_t Line;
    size_t Col;
};

class ASTNode {
public:
    enum Kind {
        // Variable Declaration
        VarDecl,
        // Expression
        NumberExpr,
        VariableExpr,
        FunctionCallExpr,
        NullExpr,
        AllocExpr,
        RefExpr,
        DerefExpr,
        InputExpr,
        BinaryExpr,
        // Statement
        LocalVarDeclStmt,
        EmptyStmt,
        OutputStmt,
        BlockStmt,
        ReturnStmt,
        IfStmt,
        WhileStmt,
        BasicAssignmentStmt,
        DerefAssignmentStmt,
        AssignmentStmt,
        // Function
        Function,
        // Program
        Program,
    };

    ASTNode(Kind K, SourceLocation Loc): ASTNodeKind(K), Loc(Loc) {}
    virtual ~ASTNode() = default;

    Kind getKind() const { return ASTNodeKind; }
    int getLine() const { return Loc.Line; }
    int getCol() const { return Loc.Col; }

private:
    const Kind ASTNodeKind;
    SourceLocation Loc;
};

class ExprAST: public ASTNode {
public:
    ExprAST(ASTNode::Kind K, SourceLocation Loc, bool LValue):
        ASTNode(K, Loc), LValue(LValue) {}

    static bool classof(const ASTNode *Node) {
        return Node->getKind() >= ASTNode::NumberExpr &&
               Node->getKind() <= ASTNode::BinaryExpr;
    }

    bool IsLValue() const { return LValue; }

private:
    bool LValue;
};

class StmtAST: public ASTNode {
public:
    StmtAST(ASTNode::Kind K, SourceLocation Loc): ASTNode(K, Loc) {}

    static bool classof(const ASTNode *Node) {
        return Node->getKind() >= ASTNode::LocalVarDeclStmt &&
               Node->getKind() <= ASTNode::AssignmentStmt;
    }
};

class VarDeclNodeAST: public ASTNode {
public:
    VarDeclNodeAST(SourceLocation Loc, std::string Name, remniw::Type *Ty):
        ASTNode(ASTNode::VarDecl, Loc), Name(Name), Ty(Ty) {}

    llvm::StringRef getName() const { return Name; }

    remniw::Type *getType() const { return Ty; }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::VarDecl;
    }

private:
    std::string Name;
    remniw::Type *Ty;
};

class NumberExprAST: public ExprAST {
public:
    NumberExprAST(SourceLocation Loc, int64_t Val):
        ExprAST(ASTNode::NumberExpr, Loc, /*LValue*/ false), Val(Val) {}

    int64_t getValue() const { return Val; }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::NumberExpr;
    }

private:
    int64_t Val;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST: public ExprAST {
public:
    VariableExprAST(SourceLocation Loc, std::string Name, bool LValue):
        ExprAST(ASTNode::VariableExpr, Loc, LValue), Name(Name) {}

    llvm::StringRef getName() const { return Name; }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::VariableExpr;
    }

private:
    std::string Name;
};

class FunctionCallExprAST: public ExprAST {
public:
    FunctionCallExprAST(SourceLocation Loc, std::unique_ptr<ExprAST> Callee,
                        std::vector<std::unique_ptr<ExprAST>> Args):
        ExprAST(ASTNode::FunctionCallExpr, Loc, /*LValue*/ false),
        Callee(std::move(Callee)), Args(std::move(Args)) {}

    ExprAST *getCallee() const { return Callee.get(); }

    std::vector<ExprAST *> getArgs() const { return rawPtrs(Args); }
    size_t getArgSize() const { return Args.size(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::FunctionCallExpr;
    }

private:
    std::unique_ptr<ExprAST> Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
};

class NullExprAST: public ExprAST {
public:
    NullExprAST(SourceLocation Loc): ExprAST(ASTNode::NullExpr, Loc, /*LValue*/ false) {}

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::NullExpr;
    }
};

class AllocExprAST: public ExprAST {
private:
    std::unique_ptr<ExprAST> Init;

public:
    AllocExprAST(SourceLocation Loc, std::unique_ptr<ExprAST> Init):
        ExprAST(ASTNode::AllocExpr, Loc, /*LValue*/ true), Init(std::move(Init)) {}

    ExprAST *getInit() const { return Init.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::AllocExpr;
    }
};

class RefExprAST: public ExprAST {
public:
    RefExprAST(SourceLocation Loc, std::unique_ptr<VariableExprAST> Var):
        ExprAST(ASTNode::RefExpr, Loc, /*LValue*/ false), Var(std::move(Var)) {}

    VariableExprAST *getVar() const { return Var.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::RefExpr;
    }

private:
    std::unique_ptr<VariableExprAST> Var;
};

class DerefExprAST: public ExprAST {
public:
    DerefExprAST(SourceLocation Loc, std::unique_ptr<ExprAST> Ptr, bool LValue):
        ExprAST(ASTNode::DerefExpr, Loc, LValue), Ptr(std::move(Ptr)) {}

    ExprAST *getPtr() const { return Ptr.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::DerefExpr;
    }

private:
    std::unique_ptr<ExprAST> Ptr;
};

class InputExprAST: public ExprAST {
public:
    InputExprAST(SourceLocation Loc):
        ExprAST(ASTNode::InputExpr, Loc, /*LValue*/ false) {}

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::InputExpr;
    }
};

// // TODO
// class RecordCreateExprAST : public ExprAST
// {
// public:
//     RecordCreateExprAST(SourceLocation Loc)
//         : ExprAST(Loc) {}
// };

// // TODO
// class RecordAccessExprAST : public ExprAST
// {
// public:
//     RecordAccessExprAST(SourceLocation Loc)
//         : ExprAST(Loc) {}
// };

class BinaryExprAST: public ExprAST {
public:
    enum OpKind {
        Mul,
        Div,
        Add,
        Sub,
        Gt,
        Eq,
    };

    BinaryExprAST(SourceLocation Loc, OpKind Op, std::unique_ptr<ExprAST> LHS,
                  std::unique_ptr<ExprAST> RHS):
        ExprAST(ASTNode::BinaryExpr, Loc, /*LValue*/ false),
        Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    OpKind getOp() const { return Op; }

    std::string getOpString() const {
        std::string OpStr;
        switch (Op) {
        case OpKind::Mul: OpStr = "*"; break;
        case OpKind::Div: OpStr = "/"; break;
        case OpKind::Add: OpStr = "+"; break;
        case OpKind::Sub: OpStr = "-"; break;
        case OpKind::Gt: OpStr = ">"; break;
        case OpKind::Eq: OpStr = "=="; break;
        default: break;
        }
        return OpStr;
    }

    ExprAST *getLHS() const { return LHS.get(); }

    ExprAST *getRHS() const { return RHS.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::BinaryExpr;
    }

private:
    OpKind Op;
    std::unique_ptr<ExprAST> LHS, RHS;
};

class LocalVarDeclStmtAST: public StmtAST {
public:
    LocalVarDeclStmtAST(SourceLocation Loc,
                        std::vector<std::unique_ptr<VarDeclNodeAST>> Vars):
        StmtAST(ASTNode::LocalVarDeclStmt, Loc),
        Vars(std::move(Vars)) {}

    std::vector<VarDeclNodeAST *> getVars() const { return rawPtrs(Vars); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::LocalVarDeclStmt;
    }

private:
    std::vector<std::unique_ptr<VarDeclNodeAST>> Vars;
};

class EmptyStmtAST: public StmtAST {
public:
    EmptyStmtAST(SourceLocation Loc): StmtAST(ASTNode::EmptyStmt, Loc) {}

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::EmptyStmt;
    }
};

class OutputStmtAST: public StmtAST {
public:
    OutputStmtAST(SourceLocation Loc, std::unique_ptr<ExprAST> Expr):
        StmtAST(ASTNode::OutputStmt, Loc), Expr(std::move(Expr)) {}

    ExprAST *getExpr() const { return Expr.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::OutputStmt;
    }

private:
    std::unique_ptr<ExprAST> Expr;
};

class BlockStmtAST: public StmtAST {
public:
    BlockStmtAST(SourceLocation Loc, std::vector<std::unique_ptr<StmtAST>> Stmts):
        StmtAST(ASTNode::BlockStmt, Loc), Stmts(std::move(Stmts)) {}

    std::vector<StmtAST *> getStmts() const { return rawPtrs(Stmts); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::BlockStmt;
    }

private:
    std::vector<std::unique_ptr<StmtAST>> Stmts;
};

class ReturnStmtAST: public StmtAST {
public:
    ReturnStmtAST(SourceLocation Loc, std::unique_ptr<ExprAST> Expr):
        StmtAST(ASTNode::ReturnStmt, Loc), Expr(std::move(Expr)) {}

    ExprAST *getExpr() const { return Expr.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::ReturnStmt;
    }

private:
    std::unique_ptr<ExprAST> Expr;
};

class IfStmtAST: public StmtAST {
public:
    IfStmtAST(SourceLocation Loc, std::unique_ptr<ExprAST> Cond,
              std::unique_ptr<StmtAST> Then, std::unique_ptr<StmtAST> Else):
        StmtAST(ASTNode::IfStmt, Loc),
        Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

    ExprAST *getCond() const { return Cond.get(); }

    StmtAST *getThen() const { return Then.get(); }

    StmtAST *getElse() const { return Else.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::IfStmt;
    }

private:
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<StmtAST> Then, Else;
};

class WhileStmtAST: public StmtAST {
public:
    WhileStmtAST(SourceLocation Loc, std::unique_ptr<ExprAST> Cond,
                 std::unique_ptr<StmtAST> Body):
        StmtAST(ASTNode::WhileStmt, Loc),
        Cond(std::move(Cond)), Body(std::move(Body)) {}

    ExprAST *getCond() const { return Cond.get(); }

    StmtAST *getBody() const { return Body.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::WhileStmt;
    }

private:
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<StmtAST> Body;
};

class BasicAssignmentStmtAST: public StmtAST {
public:
    BasicAssignmentStmtAST(SourceLocation Loc, std::unique_ptr<ExprAST> LHS,
                           std::unique_ptr<ExprAST> RHS):
        StmtAST(ASTNode::BasicAssignmentStmt, Loc),
        LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    ExprAST *getLHS() const { return LHS.get(); }

    ExprAST *getRHS() const { return RHS.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::BasicAssignmentStmt;
    }

private:
    std::unique_ptr<ExprAST> LHS;
    std::unique_ptr<ExprAST> RHS;
};

class DerefAssignmentStmtAST: public StmtAST {
public:
    DerefAssignmentStmtAST(SourceLocation Loc, std::unique_ptr<ExprAST> LHS,
                           std::unique_ptr<ExprAST> RHS):
        StmtAST(ASTNode::DerefAssignmentStmt, Loc),
        LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    ExprAST *getLHS() const { return LHS.get(); }

    ExprAST *getRHS() const { return RHS.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::DerefAssignmentStmt;
    }

private:
    std::unique_ptr<ExprAST> LHS, RHS;
};

class AssignmentStmtAST: public StmtAST {
public:
    AssignmentStmtAST(SourceLocation Loc, std::unique_ptr<ExprAST> LHS,
                      std::unique_ptr<ExprAST> RHS):
        StmtAST(ASTNode::AssignmentStmt, Loc),
        LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    ExprAST *getLHS() const { return LHS.get(); }

    ExprAST *getRHS() const { return RHS.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::AssignmentStmt;
    }

private:
    std::unique_ptr<ExprAST> LHS, RHS;
};

// // TODO
// class RecordFieldBasicAssignmentStmtAST : public StmtAST
// {
// public:
//     RecordFieldBasicAssignmentStmtAST(SourceLocation Loc)
//         : StmtAST(ASTNode::Re,Loc) {}
// };

// // TODO
// class RecordFieldDerefAssignmentStmtAST : public StmtAST
// {
// public:
//     RecordFieldDerefAssignmentStmtAST(SourceLocation Loc)
//         : StmtAST(Loc) {}
// };

/// FunctionAST - This class represents a function definition itself.
class FunctionAST: public ASTNode {
public:
    FunctionAST(SourceLocation Loc, std::string FuncName, remniw::FunctionType *FuncTy,
                std::vector<std::unique_ptr<VarDeclNodeAST>> ParamDecls,
                std::unique_ptr<LocalVarDeclStmtAST> LocalVarDecls,
                std::vector<std::unique_ptr<StmtAST>> Body,
                std::unique_ptr<ReturnStmtAST> ReturnStmt):
        ASTNode(ASTNode::Function, Loc),
        FuncName(FuncName), FuncTy(FuncTy), ParamDecls(std::move(ParamDecls)),
        LocalVarDecls(std::move(LocalVarDecls)), Body(std::move(Body)),
        ReturnStmt(std::move(ReturnStmt)) {}

    llvm::StringRef getFuncName() const { return FuncName; }

    std::vector<VarDeclNodeAST *> getParamDecls() const { return rawPtrs(ParamDecls); }

    std::size_t getParamSize() const { return ParamDecls.size(); }

    LocalVarDeclStmtAST *getLocalVarDecls() const { return LocalVarDecls.get(); }

    std::vector<StmtAST *> getBody() const { return rawPtrs(Body); }

    ReturnStmtAST *getReturn() const { return ReturnStmt.get(); }

    remniw::FunctionType *getType() const { return FuncTy; }

    llvm::ArrayRef<remniw::Type *> getParamTypes() const {
        return FuncTy->getParamTypes();
    }

    remniw::Type *getReturnType() const { return FuncTy->getReturnType(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::Function;
    }

private:
    std::string FuncName;
    remniw::FunctionType *FuncTy;
    std::vector<std::unique_ptr<VarDeclNodeAST>> ParamDecls;
    std::unique_ptr<LocalVarDeclStmtAST> LocalVarDecls;
    std::vector<std::unique_ptr<StmtAST>> Body;
    std::unique_ptr<ReturnStmtAST> ReturnStmt;
};

class ProgramAST: public ASTNode {
public:
    ProgramAST(std::vector<std::unique_ptr<FunctionAST>> Functions):
        ASTNode(ASTNode::Program, SourceLocation {0, 0}),
        Functions(std::move(Functions)) {}

    std::vector<FunctionAST *> getFunctions() const { return rawPtrs(Functions); }

    std::unique_ptr<llvm::Module> codegen(llvm::LLVMContext &);

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::Program;
    }

private:
    std::vector<std::unique_ptr<FunctionAST>> Functions;
};

}  // namespace remniw
