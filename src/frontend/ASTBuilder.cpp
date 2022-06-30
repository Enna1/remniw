#include "ASTBuilder.h"
#include "Type.h"

namespace remniw {

// helper functions for error handling.
void LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
}

static std::unique_ptr<ExprAST> visitedExpr = nullptr;
static std::unique_ptr<StmtAST> visitedStmt = nullptr;
static std::unique_ptr<ReturnStmtAST> visitedReturnStmt = nullptr;
static std::unique_ptr<FunctionAST> visitedFunction = nullptr;
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
    visit(Ctx->type());
    visitedType = visitedType->getPointerTo();
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitFunctionType(RemniwParser::FunctionTypeContext *Ctx) {
    std::vector<Type *> ParamTys;
    for (auto *ParamTyCtx : Ctx->parametersType()->type()) {
        visit(ParamTyCtx);
        ParamTys.push_back(visitedType);
    }
    visit(Ctx->type());
    Type *RetType = visitedType;
    visitedType = Type::getFunctionType(ParamTys, RetType);
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitProgram(RemniwParser::ProgramContext *Ctx) {
    std::vector<std::unique_ptr<FunctionAST>> Functions;
    for (auto *FunCtx : Ctx->fun()) {
        visit(FunCtx);
        Functions.push_back(std::move(visitedFunction));
    }
    return std::make_unique<ProgramAST>(std::move(Functions));
}

antlrcpp::Any ASTBuilder::visitFun(RemniwParser::FunContext *Ctx) {
    // function name
    std::string FuncName = Ctx->id()->IDENTIFIER()->getText();
    // paramters
    std::vector<std::unique_ptr<VarDeclNodeAST>> ParamDecls;
    std::vector<Type *> ParamTypes;
    assert(Ctx->parameters()->id().size() == Ctx->parameters()->type().size());
    for (std::size_t i = 0; i < Ctx->parameters()->id().size(); ++i) {
        auto *IdCtx = Ctx->parameters()->id(i);
        auto *TypeCtx = Ctx->parameters()->type(i);
        visit(TypeCtx);
        auto ParamDecl = std::make_unique<VarDeclNodeAST>(
            SourceLocation {IdCtx->getStart()->getLine(),
                            IdCtx->getStart()->getCharPositionInLine()},
            IdCtx->IDENTIFIER()->getText(), visitedType);
        ParamDecls.push_back(std::move(ParamDecl));
        ParamTypes.push_back(visitedType);
    }
    // var declarations
    std::vector<std::unique_ptr<VarDeclNodeAST>> Vars;
    for (auto *VarDeclCtx : Ctx->varDeclarations()) {
        visit(VarDeclCtx->type());
        for (auto *VarCtx : VarDeclCtx->id()) {
            auto Var = std::make_unique<VarDeclNodeAST>(
                SourceLocation {VarCtx->getStart()->getLine(),
                                VarCtx->getStart()->getCharPositionInLine()},
                VarCtx->IDENTIFIER()->getText(), visitedType);
            Vars.push_back(std::move(Var));
        }
    }
    auto LocalVarDecls =
        std::make_unique<LocalVarDeclStmtAST>(SourceLocation {0, 0}, std::move(Vars));
    // function body
    std::vector<std::unique_ptr<StmtAST>> Body;
    for (auto *StmtCtx : Ctx->stmt()) {
        visit(StmtCtx);
        Body.push_back(std::move(visitedStmt));
    }
    // return statement
    visit(Ctx->returnStmt());
    std::unique_ptr<ReturnStmtAST> Return = std::move(visitedReturnStmt);
    // return type
    visit(Ctx->type());
    Type *ReturnType = visitedType;
    // create ast node
    visitedFunction = std::make_unique<FunctionAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        FuncName, Type::getFunctionType(ParamTypes, ReturnType), std::move(ParamDecls),
        std::move(LocalVarDecls), std::move(Body), std::move(Return));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitMulExpr(RemniwParser::MulExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        BinaryExprAST::OpKind::Mul, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitDivExpr(RemniwParser::DivExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        BinaryExprAST::OpKind::Div, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitAddExpr(RemniwParser::AddExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        BinaryExprAST::OpKind::Add, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitSubExpr(RemniwParser::SubExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    visitedExpr = std::make_unique<BinaryExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        BinaryExprAST::OpKind::Sub, std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitIdExpr(RemniwParser::IdExprContext *Ctx) {
    visit(Ctx->id());
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitId(RemniwParser::IdContext *Ctx) {
    bool LValue = exprIsLValue;
    visitedExpr = std::make_unique<VariableExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        Ctx->IDENTIFIER()->getText(), LValue);
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
            Val);
    } else {
        LogError("integer falls out of range int64_t");
        visitedExpr = std::make_unique<NumberExprAST>(
            SourceLocation {Ctx->getStart()->getLine(),
                            Ctx->getStart()->getCharPositionInLine()},
            Val);
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
            Val);
    } else {
        LogError("integer falls out of range int64_t");
        visitedExpr = std::make_unique<NumberExprAST>(
            SourceLocation {Ctx->getStart()->getLine(),
                            Ctx->getStart()->getCharPositionInLine()},
            Val);
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitRefExpr(RemniwParser::RefExprContext *Ctx) {
    auto Var = std::make_unique<VariableExprAST>(
        SourceLocation {Ctx->id()->getStart()->getLine(),
                        Ctx->id()->getStart()->getCharPositionInLine()},
        Ctx->id()->IDENTIFIER()->getText(), /*LValue*/ true);
    visitedExpr = std::make_unique<RefExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(Var));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitDerefExpr(RemniwParser::DerefExprContext *Ctx) {
    bool LValue = exprIsLValue;
    visit(Ctx->expr());
    visitedExpr = std::make_unique<DerefExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(visitedExpr), LValue);
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
    visitedExpr = std::make_unique<FunctionCallExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(Callee), std::move(Args));
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

antlrcpp::Any ASTBuilder::visitAllocExpr(RemniwParser::AllocExprContext *Ctx) {
    exprIsLValue = false;
    visit(Ctx->expr());
    visitedExpr = std::make_unique<AllocExprAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(visitedExpr));
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitInputExpr(RemniwParser::InputExprContext *Ctx) {
    visitedExpr = std::make_unique<InputExprAST>(SourceLocation {
        Ctx->getStart()->getLine(), Ctx->getStart()->getCharPositionInLine()});
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

antlrcpp::Any
ASTBuilder::visitBasicAssignmentStmt(RemniwParser::BasicAssignmentStmtContext *Ctx) {
    exprIsLValue = true;
    visit(Ctx->id());
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr());
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    visitedStmt = std::make_unique<BasicAssignmentStmtAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(LHS), std::move(RHS));
    return nullptr;
}

antlrcpp::Any
ASTBuilder::visitDerefAssignmentStmt(RemniwParser::DerefAssignmentStmtContext *Ctx) {
    exprIsLValue = true;
    visit(Ctx->expr(0));
    std::unique_ptr<ExprAST> LHS = std::move(visitedExpr);
    exprIsLValue = false;
    visit(Ctx->expr(1));
    std::unique_ptr<ExprAST> RHS = std::move(visitedExpr);
    visitedStmt = std::make_unique<DerefAssignmentStmtAST>(
        SourceLocation {Ctx->getStart()->getLine(),
                        Ctx->getStart()->getCharPositionInLine()},
        std::move(LHS), std::move(RHS));
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

// // TODO
// antlrcpp::Any ASTBuilder::visitRecordFieldBasicAssignmentStmt(
//     RemniwParser::RecordFieldBasicAssignmentStmtContext *Ctx) {
//     return nullptr;
// }

// // TODO
// antlrcpp::Any ASTBuilder::visitRecordFieldDerefAssignmentStmt(
//     RemniwParser::RecordFieldDerefAssignmentStmtContext *Ctx) {
//     return nullptr;
// }

}  // namespace remniw
