#pragma once

#include "frontend/Type.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
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
        // Program
        Program,
        // Variable Declaration
        VarDecl,
        // Function Declaration
        FunctionDecl,
        // Expression
        NumberExpr,
        VariableExpr,
        FunctionCallExpr,
        NullExpr,
        SizeofExpr,
        RefExpr,
        DerefExpr,
        ArraySubscriptExpr,
        InputExpr,
        BinaryExpr,
        // Statement
        LocalVarDeclStmt,
        EmptyStmt,
        OutputStmt,
        AllocStmt,
        DeallocStmt,
        BlockStmt,
        ReturnStmt,
        IfStmt,
        WhileStmt,
        AssignmentStmt,
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

class DeclAST: public ASTNode {
public:
    DeclAST(ASTNode::Kind K, SourceLocation Loc, std::string Name, remniw::Type *Ty):
        ASTNode(K, Loc), Name(Name), Ty(Ty) {}

    static bool classof(const ASTNode *Node) {
        return Node->getKind() >= ASTNode::VarDecl &&
               Node->getKind() <= ASTNode::FunctionDecl;
    }

    llvm::StringRef getName() const { return Name; }

    remniw::Type *getType() const { return Ty; }

private:
    std::string Name;
    remniw::Type *Ty;
};

class VarDeclAST: public DeclAST {
public:
    VarDeclAST(SourceLocation Loc, std::string Name, remniw::Type *Ty):
        DeclAST(ASTNode::VarDecl, Loc, Name, Ty) {}

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::VarDecl;
    }
};

class ExprAST: public ASTNode {
public:
    ExprAST(ASTNode::Kind K, SourceLocation Loc, remniw::Type *Ty, bool LValue):
        ASTNode(K, Loc), Ty(Ty), LValue(LValue) {}

    static bool classof(const ASTNode *Node) {
        return Node->getKind() >= ASTNode::NumberExpr &&
               Node->getKind() <= ASTNode::BinaryExpr;
    }

    bool isLValue() const { return LValue; }

    remniw::Type *getType() const { return Ty; }

private:
    remniw::Type *Ty;
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

class NumberExprAST: public ExprAST {
public:
    NumberExprAST(SourceLocation Loc, remniw::Type* Ty, int64_t Val):
        ExprAST(ASTNode::NumberExpr, Loc, Ty, /*LValue*/ false), Val(Val) {}

    int64_t getValue() const { return Val; }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::NumberExpr;
    }

private:
    int64_t Val;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
// TODO: rename DeclRefExpr?
class VariableExprAST: public ExprAST {
public:
    VariableExprAST(SourceLocation Loc, std::string Name, DeclAST *Decl, bool LValue):
        ExprAST(ASTNode::VariableExpr, Loc, Decl->getType(), LValue), Decl(Decl), Name(Name) {}

    llvm::StringRef getName() const { return Name; }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::VariableExpr;
    }

private:
    std::string Name;
    DeclAST *Decl;
};

class FunctionCallExprAST: public ExprAST {
public:
    FunctionCallExprAST(SourceLocation Loc, remniw::Type* Ty, std::unique_ptr<ExprAST> Callee,
                        std::vector<std::unique_ptr<ExprAST>> Args):
        ExprAST(ASTNode::FunctionCallExpr, Loc, Ty, /*LValue*/ false),
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
    NullExprAST(SourceLocation Loc): ExprAST(ASTNode::NullExpr, Loc, /*Ty*/ nullptr, /*LValue*/ false) {}

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::NullExpr;
    }
};

class SizeofExprAST: public ExprAST {
public:
    SizeofExprAST(SourceLocation Loc, remniw::Type* Ty, remniw::Type *DataTy):
        ExprAST(ASTNode::SizeofExpr, Loc, Ty, /*LValue*/ false), DataTy(DataTy) {}

    // sizeof(data-type)
    remniw::Type *getDataType() const { return DataTy; }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::SizeofExpr;
    }

private:
    remniw::Type *DataTy;
};

class RefExprAST: public ExprAST {
public:
    RefExprAST(SourceLocation Loc, remniw::Type* Ty, std::unique_ptr<VariableExprAST> Var):
        ExprAST(ASTNode::RefExpr, Loc, Ty, /*LValue*/ false), Var(std::move(Var)) {}

    VariableExprAST *getVar() const { return Var.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::RefExpr;
    }

private:
    std::unique_ptr<VariableExprAST> Var;
};

class DerefExprAST: public ExprAST {
public:
    DerefExprAST(SourceLocation Loc, remniw::Type* Ty, bool LValue, std::unique_ptr<ExprAST> Ptr):
        ExprAST(ASTNode::DerefExpr, Loc, Ty, LValue), Ptr(std::move(Ptr)) {}

    ExprAST *getPtr() const { return Ptr.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::DerefExpr;
    }

private:
    std::unique_ptr<ExprAST> Ptr;
};

class ArraySubscriptExprAST: public ExprAST {
public:
    ArraySubscriptExprAST(SourceLocation Loc, remniw::Type* Ty, bool LValue, std::unique_ptr<ExprAST> Base,
                          std::unique_ptr<ExprAST> Selector):
        ExprAST(ASTNode::ArraySubscriptExpr, Loc, Ty, LValue),
        Base(std::move(Base)), Selector(std::move(Selector)) {}

    ExprAST *getBase() const { return Base.get(); }

    ExprAST *getSelector() const { return Selector.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::ArraySubscriptExpr;
    }

private:
    std::unique_ptr<ExprAST> Base;
    std::unique_ptr<ExprAST> Selector;
};

class InputExprAST: public ExprAST {
public:
    InputExprAST(SourceLocation Loc, remniw::Type* Ty):
        ExprAST(ASTNode::InputExpr, Loc, Ty, /*LValue*/ false) {}

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

    BinaryExprAST(SourceLocation Loc, remniw::Type* Ty, OpKind Op, std::unique_ptr<ExprAST> LHS,
                  std::unique_ptr<ExprAST> RHS):
        ExprAST(ASTNode::BinaryExpr, Loc, Ty, /*LValue*/ false),
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
                        std::vector<std::unique_ptr<VarDeclAST>> Vars):
        StmtAST(ASTNode::LocalVarDeclStmt, Loc),
        Vars(std::move(Vars)) {}

    std::vector<VarDeclAST *> getVars() const { return rawPtrs(Vars); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::LocalVarDeclStmt;
    }

private:
    std::vector<std::unique_ptr<VarDeclAST>> Vars;
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

class AllocStmtAST: public StmtAST {
public:
    AllocStmtAST(SourceLocation Loc, std::unique_ptr<ExprAST> Ptr,
                 std::unique_ptr<ExprAST> Size):
        StmtAST(ASTNode::AllocStmt, Loc),
        Ptr(std::move(Ptr)), Size(std::move(Size)) {}

    ExprAST *getPtr() const { return Ptr.get(); }
    ExprAST *getSize() const { return Size.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::AllocStmt;
    }

private:
    std::unique_ptr<ExprAST> Ptr;
    std::unique_ptr<ExprAST> Size;
};

class DeallocStmtAST: public StmtAST {
public:
    DeallocStmtAST(SourceLocation Loc, std::unique_ptr<ExprAST> Expr):
        StmtAST(ASTNode::DeallocStmt, Loc), Expr(std::move(Expr)) {}

    ExprAST *getExpr() const { return Expr.get(); }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::DeallocStmt;
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

/// FunctionAST - This class represents a function definition itself.
class FunctionDeclAST: public DeclAST {
public:
    FunctionDeclAST(SourceLocation Loc, std::string FuncName, remniw::FunctionType *FuncTy):
        DeclAST(ASTNode::FunctionDecl, Loc, FuncName, FuncTy) {}

    void setParamDecls(std::vector<std::unique_ptr<VarDeclAST>> ParamDecls) {
        this->ParamDecls = std::move(ParamDecls);
    }

    void setLocalVarDecls(std::unique_ptr<LocalVarDeclStmtAST> LocalVarDecls) {
        this->LocalVarDecls = std::move(LocalVarDecls);
    }

    void setBody(std::vector<std::unique_ptr<StmtAST>> Body) {
        this->Body = std::move(Body);
    }

    void setReturnStmt(std::unique_ptr<ReturnStmtAST> ReturnStmt) {
        this->ReturnStmt = std::move(ReturnStmt);
    }

    FunctionDeclAST(SourceLocation Loc, std::string FuncName, remniw::FunctionType *FuncTy,
                std::vector<std::unique_ptr<VarDeclAST>> ParamDecls,
                std::unique_ptr<LocalVarDeclStmtAST> LocalVarDecls,
                std::vector<std::unique_ptr<StmtAST>> Body,
                std::unique_ptr<ReturnStmtAST> ReturnStmt):
        DeclAST(ASTNode::FunctionDecl, Loc, FuncName, FuncTy),
        ParamDecls(std::move(ParamDecls)),
        LocalVarDecls(std::move(LocalVarDecls)), Body(std::move(Body)),
        ReturnStmt(std::move(ReturnStmt)) {}

    std::vector<VarDeclAST *> getParamDecls() const { return rawPtrs(ParamDecls); }

    std::size_t getParamSize() const { return ParamDecls.size(); }

    LocalVarDeclStmtAST *getLocalVarDecls() const { return LocalVarDecls.get(); }

    std::vector<StmtAST *> getBody() const { return rawPtrs(Body); }

    ReturnStmtAST *getReturn() const { return ReturnStmt.get(); }

    llvm::ArrayRef<remniw::Type *> getParamTypes() const {
        return llvm::cast<remniw::FunctionType>(getType())->getParamTypes();
    }

    remniw::Type *getReturnType() const {
        return llvm::cast<remniw::FunctionType>(getType())->getReturnType();
    }

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::FunctionDecl;
    }

private:
    std::vector<std::unique_ptr<VarDeclAST>> ParamDecls;
    std::unique_ptr<LocalVarDeclStmtAST> LocalVarDecls;
    std::vector<std::unique_ptr<StmtAST>> Body;
    std::unique_ptr<ReturnStmtAST> ReturnStmt;
};

class ProgramAST: public ASTNode {
public:
    ProgramAST(std::vector<std::unique_ptr<FunctionDeclAST>> Functions):
        ASTNode(ASTNode::Program, SourceLocation {0, 0}),
        Functions(std::move(Functions)) {}

    std::vector<FunctionDeclAST *> getFunctions() const { return rawPtrs(Functions); }

    std::unique_ptr<llvm::Module> codegen(llvm::LLVMContext &);

    static bool classof(const ASTNode *Node) {
        return Node->getKind() == ASTNode::Program;
    }

private:
    std::vector<std::unique_ptr<FunctionDeclAST>> Functions;
};

}  // namespace remniw
