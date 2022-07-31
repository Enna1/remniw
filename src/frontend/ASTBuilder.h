#pragma once

#include "AST.h"
#include "RemniwBaseVisitor.h"
#include "Type.h"
#include "antlr4-runtime.h"
#include <memory>

namespace remniw {

class ASTBuilder: public RemniwBaseVisitor {
private:
    TypeContext &TyCtx;

public:
    ASTBuilder(TypeContext &TyCtx): TyCtx(TyCtx) {}

    std::unique_ptr<ProgramAST> build(RemniwParser::ProgramContext *Ctx);

    virtual antlrcpp::Any visitIntType(RemniwParser::IntTypeContext *Ctx);

    virtual antlrcpp::Any visitPointerType(RemniwParser::PointerTypeContext *Ctx);

    virtual antlrcpp::Any visitFunctionType(RemniwParser::FunctionTypeContext *Ctx);

    // virtual antlrcpp::Any visitScalarType(RemniwParser::ScalarTypeContext *Ctx);

    virtual antlrcpp::Any visitArrayType(RemniwParser::ArrayTypeContext *Ctx);

    // virtual antlrcpp::Any visitVarType(RemniwParser::VarTypeContext *Ctx);

    // virtual antlrcpp::Any visitParamType(RemniwParser::ParamTypeContext *Ctx);

    virtual antlrcpp::Any visitProgram(RemniwParser::ProgramContext *Ctx);

    virtual antlrcpp::Any visitFun(RemniwParser::FunContext *Ctx);

    virtual antlrcpp::Any visitMulExpr(RemniwParser::MulExprContext *Ctx);

    virtual antlrcpp::Any visitDivExpr(RemniwParser::DivExprContext *Ctx);

    virtual antlrcpp::Any visitAddExpr(RemniwParser::AddExprContext *Ctx);

    virtual antlrcpp::Any visitSubExpr(RemniwParser::SubExprContext *Ctx);

    virtual antlrcpp::Any visitEqualExpr(RemniwParser::EqualExprContext *Ctx);

    virtual antlrcpp::Any visitRelationalExpr(RemniwParser::RelationalExprContext *Ctx);

    virtual antlrcpp::Any visitIdExpr(RemniwParser::IdExprContext *Ctx);

    virtual antlrcpp::Any visitIntExpr(RemniwParser::IntExprContext *Ctx);

    virtual antlrcpp::Any visitNegIntExpr(RemniwParser::NegIntExprContext *Ctx);

    virtual antlrcpp::Any visitRefExpr(RemniwParser::RefExprContext *Ctx);

    virtual antlrcpp::Any visitDerefExpr(RemniwParser::DerefExprContext *Ctx);

    virtual antlrcpp::Any
    visitArraySubscriptExpr(RemniwParser::ArraySubscriptExprContext *Ctx);

    virtual antlrcpp::Any visitNullExpr(RemniwParser::NullExprContext *Ctx);

    virtual antlrcpp::Any visitSizeofExpr(RemniwParser::SizeofExprContext *Ctx);

    virtual antlrcpp::Any visitInputExpr(RemniwParser::InputExprContext *Ctx);

    virtual antlrcpp::Any visitParenExpr(RemniwParser::ParenExprContext *Ctx);

    virtual antlrcpp::Any visitFuncCallExpr(RemniwParser::FuncCallExprContext *Ctx);

    // // TODO
    // virtual antlrcpp::Any
    // visitRecordCreateExpr(RemniwParser::RecordCreateExprContext *Ctx);

    // // TODO
    // virtual antlrcpp::Any
    // visitRecordAccessExpr(RemniwParser::RecordAccessExprContext *Ctx);

    virtual antlrcpp::Any visitId(RemniwParser::IdContext *Ctx);

    virtual antlrcpp::Any visitEmptyStmt(RemniwParser::EmptyStmtContext *Ctx);

    virtual antlrcpp::Any visitOutputStmt(RemniwParser::OutputStmtContext *Ctx);

    virtual antlrcpp::Any visitAllocStmt(RemniwParser::AllocStmtContext *Ctx);

    virtual antlrcpp::Any visitDeallocStmt(RemniwParser::DeallocStmtContext *Ctx);

    virtual antlrcpp::Any visitBlockStmt(RemniwParser::BlockStmtContext *Ctx);

    virtual antlrcpp::Any visitReturnStmt(RemniwParser::ReturnStmtContext *Ctx);

    virtual antlrcpp::Any visitIfStmt(RemniwParser::IfStmtContext *Ctx);

    virtual antlrcpp::Any visitWhileStmt(RemniwParser::WhileStmtContext *Ctx);

    virtual antlrcpp::Any visitAssignmentStmt(RemniwParser::AssignmentStmtContext *Ctx);
};

}  // namespace remniw
