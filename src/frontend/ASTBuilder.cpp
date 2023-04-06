#include "frontend/ASTBuilder.h"
#include "frontend/AST.h"
#include "frontend/Type.h"

namespace remniw {

static std::unique_ptr<ExprAST> visitedExpr = nullptr;
static std::unique_ptr<StmtAST> visitedStmt = nullptr;
static std::unique_ptr<ReturnStmtAST> visitedReturnStmt = nullptr;
static Type *visitedType = nullptr;
static bool exprIsLValue = false;

std::unique_ptr<ProgramAST> ASTBuilder::build(RemniwParser::ProgramContext *Ctx) {
    return std::move(visitProgram(Ctx).as<std::unique_ptr<ProgramAST>>());
}

antlrcpp::Any ASTBuilder::visitIntType(RemniwParser::IntTypeContext *Ctx) {
    visitedType = Type::getIntType(TyCtx);
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitPointerType(RemniwParser::PointerTypeContext *Ctx) {
    visit(Ctx->varType());
    visitedType = visitedType->getPointerTo();
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitFunctionType(RemniwParser::FunctionTypeContext *Ctx) {
    std::vector<Type *> ParamTys;
    for (auto *ParamTyCtx : Ctx->parametersType()->paramType()) {
        visit(ParamTyCtx);
        ParamTys.push_back(visitedType);
    }
    visit(Ctx->scalarType());
    Type *RetType = visitedType;
    visitedType = Type::getFunctionType(ParamTys, RetType);
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitArrayType(RemniwParser::ArrayTypeContext *Ctx) {
    visit(Ctx->varType());
    uint64_t NumElements = 0;
    // strtoll returns long long >= 64 bits, so check it's in range.
    errno = 0;
    std::string S = Ctx->integer()->NUMBER()->getText();
    auto Val = std::strtoull(S.c_str(), nullptr, 10);
    if (errno == 0 && Val >= std::numeric_limits<uint64_t>::min() &&
        Val <= std::numeric_limits<uint64_t>::max()) {
        NumElements = Val;
    }
    visitedType = Type::getArrayType(visitedType, NumElements);
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitProgram(RemniwParser::ProgramContext *Ctx) {
    std::vector<RemniwParser::FunContext *> FunctionContexts = Ctx->fun();
    for (unsigned i = 0 ;i < FunctionContexts.size(); ++i) {
        auto Function = visitFunctionPrototype(FunctionContexts[i]);
        Functions.push_back(std::move(Function));
    }
    for (unsigned i = 0 ;i < FunctionContexts.size(); ++i) {
        visitFunctionBody(FunctionContexts[i], Functions[i].get());
    }
    return std::make_unique<ProgramAST>(std::move(Functions));
}

std::unique_ptr<FunctionDeclAST> ASTBuilder::visitFunctionPrototype(RemniwParser::FunContext *Ctx) {
    // function name
    std::string FuncName = Ctx->id()->IDENTIFIER()->getText();
    // paramters type
    std::vector<Type *> ParamTypes;
    assert(Ctx->parameters()->id().size() == Ctx->parameters()->paramType().size());
    for (std::size_t i = 0; i < Ctx->parameters()->id().size(); ++i) {
        auto *TypeCtx = Ctx->parameters()->paramType(i);
        visit(TypeCtx);
        ParamTypes.push_back(visitedType);
    }
    // return type
    visit(Ctx->scalarType());
    Type *ReturnType = visitedType;
    // create function ast node
    auto Function = std::make_unique<FunctionDeclAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        FuncName, Type::getFunctionType(ParamTypes, ReturnType));
    return Function;
}

void ASTBuilder::visitFunctionBody(RemniwParser::FunContext *Ctx, FunctionDeclAST* Function) {
    CurrentFunction = Function;
    // param declarations
    std::vector<std::unique_ptr<VarDeclAST>> ParamDecls;
    assert(Ctx->parameters()->id().size() == Ctx->parameters()->paramType().size());
    for (std::size_t i = 0; i < Ctx->parameters()->id().size(); ++i) {
        auto *IdCtx = Ctx->parameters()->id(i);
        auto *TypeCtx = Ctx->parameters()->paramType(i);
        visit(TypeCtx);
        auto ParamDecl = std::make_unique<VarDeclAST>(
            SourceLocation {IdCtx->getStart()->getLine(),
                            IdCtx->getStart()->getCharPositionInLine()},
            IdCtx->IDENTIFIER()->getText(), visitedType);
        ParamDecls.push_back(std::move(ParamDecl));
    }
    Function->setParamDecls(std::move(ParamDecls));
    // var declarations
    std::vector<std::unique_ptr<VarDeclAST>> Vars;
    for (auto *VarDeclCtx : Ctx->varDeclarations()) {
        visit(VarDeclCtx->varType());
        for (auto *VarCtx : VarDeclCtx->id()) {
            auto Var = std::make_unique<VarDeclAST>(
                SourceLocation {VarCtx->getStart()->getLine(),
                                VarCtx->getStart()->getCharPositionInLine()},
                VarCtx->IDENTIFIER()->getText(), visitedType);
            Vars.push_back(std::move(Var));
        }
    }
    auto LocalVarDecls =
        std::make_unique<LocalVarDeclStmtAST>(SourceLocation {0, 0}, std::move(Vars));
    Function->setLocalVarDecls(std::move(LocalVarDecls));
    // function body
    std::vector<std::unique_ptr<StmtAST>> Body;
    for (auto *StmtCtx : Ctx->stmt()) {
        visit(StmtCtx);
        Body.push_back(std::move(visitedStmt));
    }
    Function->setBody(std::move(Body));
    // return statement
    visit(Ctx->returnStmt());
    Function->setReturnStmt(std::move(visitedReturnStmt));
}

// TODO: delete
// antlrcpp::Any ASTBuilder::visitFun(RemniwParser::FunContext *Ctx) {
//     // function name
//     std::string FuncName = Ctx->id()->IDENTIFIER()->getText();
//     // paramters
//     std::vector<std::unique_ptr<VarDeclAST>> ParamDecls;
//     std::vector<Type *> ParamTypes;
//     assert(Ctx->parameters()->id().size() == Ctx->parameters()->paramType().size());
//     for (std::size_t i = 0; i < Ctx->parameters()->id().size(); ++i) {
//         auto *IdCtx = Ctx->parameters()->id(i);
//         auto *TypeCtx = Ctx->parameters()->paramType(i);
//         visit(TypeCtx);
//         auto ParamDecl = std::make_unique<VarDeclAST>(
//             SourceLocation {IdCtx->getStart()->getLine(),
//                             IdCtx->getStart()->getCharPositionInLine()},
//             IdCtx->IDENTIFIER()->getText(), visitedType);
//         ParamDecls.push_back(std::move(ParamDecl));
//         ParamTypes.push_back(visitedType);
//     }
//     // var declarations
//     std::vector<std::unique_ptr<VarDeclAST>> Vars;
//     for (auto *VarDeclCtx : Ctx->varDeclarations()) {
//         visit(VarDeclCtx->varType());
//         for (auto *VarCtx : VarDeclCtx->id()) {
//             auto Var = std::make_unique<VarDeclAST>(
//                 SourceLocation {VarCtx->getStart()->getLine(),
//                                 VarCtx->getStart()->getCharPositionInLine()},
//                 VarCtx->IDENTIFIER()->getText(), visitedType);
//             Vars.push_back(std::move(Var));
//         }
//     }
//     auto LocalVarDecls =
//         std::make_unique<LocalVarDeclStmtAST>(SourceLocation {0, 0}, std::move(Vars));
//     // function body
//     std::vector<std::unique_ptr<StmtAST>> Body;
//     for (auto *StmtCtx : Ctx->stmt()) {
//         visit(StmtCtx);
//         Body.push_back(std::move(visitedStmt));
//     }
//     // return statement
//     visit(Ctx->returnStmt());
//     std::unique_ptr<ReturnStmtAST> Return = std::move(visitedReturnStmt);
//     // return type
//     visit(Ctx->scalarType());
//     Type *ReturnType = visitedType;
//     // create ast node
//     visitedFunction = std::make_unique<FunctionDeclAST>(
//         SourceLocation {Ctx->getStart()->getLine(),
//                         Ctx->getStart()->getCharPositionInLine()},
//         FuncName, Type::getFunctionType(ParamTypes, ReturnType), std::move(ParamDecls),
//         std::move(LocalVarDecls), std::move(Body), std::move(Return));
//     return nullptr;
// }

antlrcpp::Any ASTBuilder::visitMulExpr(RemniwParser::MulExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    // The type of MulExpr is same as the type of LHS and the type of RHS.
    auto *Ty = LHS->getType();
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ty, BinaryExprAST::OpKind::Mul, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitDivExpr(RemniwParser::DivExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    // The type of DivExpr is same as the type of LHS and the type of RHS.
    auto *Ty = LHS->getType();
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ty, BinaryExprAST::OpKind::Div, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitAddExpr(RemniwParser::AddExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    // The type of AddExpr is same as the type of LHS and the type of RHS.
    auto *Ty = LHS->getType();
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ty, BinaryExprAST::OpKind::Add, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitSubExpr(RemniwParser::SubExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    // The type of SubExpr is same as the type of LHS and the type of RHS.
    auto *Ty = LHS->getType();
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ty, BinaryExprAST::OpKind::Sub, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitIdExpr(RemniwParser::IdExprContext *Ctx) {
    visit(Ctx->id());
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitId(RemniwParser::IdContext *Ctx) {
    bool LValue = exprIsLValue;
    std::string Name = Ctx->IDENTIFIER()->getText();
    auto *Decl = lookupDeclInScope(Name);
    visitedExpr = std::make_unique<DeclRefExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ctx->IDENTIFIER()->getText(), Decl, LValue);
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitIntExpr(RemniwParser::IntExprContext *Ctx) {
    // strtoll returns long long >= 64 bits, so check it's in range.
    errno = 0;
    std::string S = Ctx->integer()->NUMBER()->getText();
    auto Val = std::strtoll(S.c_str(), nullptr, 10);
    if (errno == 0 && Val >= std::numeric_limits<int64_t>::min() &&
        Val <= std::numeric_limits<int64_t>::max()) {
        visitedExpr = std::make_unique<NumberExprAST>(
            SourceLocation {Ctx->getStart()->getLine(),
                            Ctx->getStart()->getCharPositionInLine()},
            Type::getIntType(TyCtx), Val);
    } else {
        // integer falls out of range int64_t
        visitedExpr = std::make_unique<NumberExprAST>(
            SourceLocation {Ctx->getStart()->getLine(),
                            Ctx->getStart()->getCharPositionInLine()},
            Type::getIntType(TyCtx), Val);
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitNegIntExpr(RemniwParser::NegIntExprContext *Ctx) {
    // strtoll returns long long >= 64 bits, so check it's in range.
    errno = 0;
    std::string S = '-' + Ctx->integer()->NUMBER()->getText();
    auto Val = std::strtoll(S.c_str(), nullptr, 10);
    if (errno == 0 && Val >= std::numeric_limits<int64_t>::min() &&
        Val <= std::numeric_limits<int64_t>::max()) {
        visitedExpr = std::make_unique<NumberExprAST>(
            SourceLocation {Ctx->getStart()->getLine(),
                            Ctx->getStart()->getCharPositionInLine()},
            Type::getIntType(TyCtx), Val);
    } else {
        // integer falls out of range int64_t
        visitedExpr = std::make_unique<NumberExprAST>(
            SourceLocation {Ctx->getStart()->getLine(),
                            Ctx->getStart()->getCharPositionInLine()},
            Type::getIntType(TyCtx), Val);
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitAddrOfExpr(RemniwParser::AddrOfExprContext *Ctx) {
    auto *Decl = lookupDeclInScope(Ctx->id()->IDENTIFIER()->getText());
    auto Var = std::make_unique<DeclRefExprAST>(
        SourceLocation {Ctx->id()->getStart()->getLine(),
                        Ctx->id()->getStart()->getCharPositionInLine()},
        Ctx->id()->IDENTIFIER()->getText(), Decl, /*LValue*/ true);\
    auto *Ty = Decl->getType()->getPointerTo();
    visitedExpr = std::make_unique<AddrOfExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ty, std::move(Var));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitDerefExpr(RemniwParser::DerefExprContext *Ctx) {
    bool LValue = exprIsLValue;
    // If DerefExpr is lvalue, then the Ptr expr of DerefExpr is lvalue.
    // If DerefExpr is rvalue, then the Ptr expr of DerefExpr is rvalue.
    visit(Ctx->expr());
    auto *Ty = visitedExpr->getType()->getPointerPointeeType();
    visitedExpr = std::make_unique<DerefExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ty, LValue, std::move(visitedExpr));
    return nullptr;
}

antlrcpp::Any
ASTBuilder::visitArraySubscriptExpr(RemniwParser::ArraySubscriptExprContext *Ctx) {
    bool LValue = exprIsLValue;
    exprIsLValue = true;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> Base = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> Selector = std::move(visitedExpr);
    auto *Ty = Base->getType()->getArrayElementType();
    visitedExpr = std::make_unique<ArraySubscriptExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ty, LValue, std::move(Base), std::move(Selector));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitEqualExpr(RemniwParser::EqualExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Type::getIntType(TyCtx) /*FIXME: the type of EqualExpr is IntType*/,
        BinaryExprAST::OpKind::Eq, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitRelationalExpr(RemniwParser::RelationalExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Type::getIntType(TyCtx) /*FIXME: the type of RelationalExpr is IntType*/,
        BinaryExprAST::OpKind::Gt, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitParenExpr(RemniwParser::ParenExprContext *Ctx) {
    visit(Ctx->expr());
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitFuncCallExpr(RemniwParser::FuncCallExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr());
    std::unique_ptr<ExprAST> Callee = std::move(visitedExpr);
    std::vector<std::unique_ptr<ExprAST>> Args;
    for (auto *Arg : Ctx->arguments()->expr()) {
        exprIsLValue = false;
        visit(Arg);
        Args.push_back(std::move(visitedExpr));
    }
    auto *Ty= Callee->getType()->getFunctionReturnType();
    visitedExpr = std::make_unique<FunctionCallExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ty, std::move(Callee), std::move(Args));
    return nullptr;
}

// // TODO
// virtual antlrcpp::Any
// ASTBuilder::visitRecordCreateExpr(RemniwParser::RecordCreateExprContext *Ctx) {
//     return nullptr;
// }

// // TODO
// virtual antlrcpp::Any
// ASTBuilder::visitRecordAccessExpr(RemniwParser::RecordAccessExprContext *Ctx) {
//     return nullptr;
// }

antlrcpp::Any ASTBuilder::visitNullExpr(RemniwParser::NullExprContext *Ctx) {
    visitedExpr = std::make_unique<NullExprAST>(SourceLocation {
        Ctx->getStart()->getLine(), Ctx->getStart()->getCharPositionInLine()});
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitSizeofExpr(RemniwParser::SizeofExprContext *Ctx) {
    visit(Ctx->varType());
    visitedExpr = std::make_unique<SizeofExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Type::getIntType(TyCtx), visitedType);
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitInputExpr(RemniwParser::InputExprContext *Ctx) {
    visitedExpr = std::make_unique<InputExprAST>(SourceLocation {
        Ctx->getStart()->getLine(), Ctx->getStart()->getCharPositionInLine()},
        Type::getIntType(TyCtx));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitEmptyStmt(RemniwParser::EmptyStmtContext *Ctx) {
    visitedStmt = std::make_unique<EmptyStmtAST>(SourceLocation {
        Ctx->getStart()->getLine(), Ctx->getStart()->getCharPositionInLine()});
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitOutputStmt(RemniwParser::OutputStmtContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr());
    visitedStmt = std::make_unique<OutputStmtAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(visitedExpr));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitAllocStmt(RemniwParser::AllocStmtContext *Ctx) {
    exprIsLValue = true;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> Ptr = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> Size = std::move(visitedExpr);
    visitedStmt = std::make_unique<AllocStmtAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(Ptr), std::move(Size));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitDeallocStmt(RemniwParser::DeallocStmtContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr());
    visitedStmt = std::make_unique<DeallocStmtAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(visitedExpr));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitBlockStmt(RemniwParser::BlockStmtContext *Ctx) {
    std::vector<std::unique_ptr<StmtAST>> Stmts;
    for (auto *StmtCtx : Ctx->stmt()) {
        visit(StmtCtx);
        Stmts.push_back(std::move(visitedStmt));
    }
    visitedStmt = std::make_unique<BlockStmtAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(Stmts));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitReturnStmt(RemniwParser::ReturnStmtContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr());
    visitedReturnStmt = std::make_unique<ReturnStmtAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(visitedExpr));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitIfStmt(RemniwParser::IfStmtContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr());
    std::unique_ptr<ExprAST> Cond = std::move(visitedExpr);
    if (Ctx->stmt().size() == 2) {
        visit(Ctx->stmt(0));
        std::unique_ptr<StmtAST> Then = std::move(visitedStmt);
        visit(Ctx->stmt(1));
        std::unique_ptr<StmtAST> Else = std::move(visitedStmt);
        visitedStmt = std::make_unique<IfStmtAST>(
            SourceLocation {Ctx->getStart()->getLine(),
                            Ctx->getStart()->getCharPositionInLine()},
            std::move(Cond), std::move(Then), std::move(Else));
    } else if (Ctx->stmt().size() == 1) {
        visit(Ctx->stmt(0));
        std::unique_ptr<StmtAST> Then = std::move(visitedStmt);
        visitedStmt = std::make_unique<IfStmtAST>(
            SourceLocation {Ctx->getStart()->getLine(),
                            Ctx->getStart()->getCharPositionInLine()},
            std::move(Cond), std::move(Then), nullptr);
    } else {
        assert(0 && "Unexpected IfStmtContext stmt().size()");
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitWhileStmt(RemniwParser::WhileStmtContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr());
    std::unique_ptr<ExprAST> Cond = std::move(visitedExpr);
    visit(Ctx->stmt());
    std::unique_ptr<StmtAST> Body = std::move(visitedStmt);
    visitedStmt = std::make_unique<WhileStmtAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(Cond), std::move(Body));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitAssignmentStmt(RemniwParser::AssignmentStmtContext *Ctx) {
    exprIsLValue = true;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    visitedStmt = std::make_unique<AssignmentStmtAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(LHS), std::move(RHS));
    return nullptr;
}

DeclAST *ASTBuilder::lookupDeclInScope(std::string Name) {
    for (auto *Param : CurrentFunction->getParamDecls()) {
        if (Param->getName().equals(Name))
            return Param;
    }
    for (auto *LocalVar: CurrentFunction->getLocalVarDecls()->getVars()) {
        if (LocalVar->getName().equals(Name))
            return LocalVar;
    }
    for (const auto& Function: Functions) {
        if (Function->getName().equals(Name))
            return Function.get();
    }
    return nullptr;
}

}  // namespace remniw
