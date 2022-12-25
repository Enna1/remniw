#ifndef __OLIVE_HEADER_INCLUDED__
#define __OLIVE_HEADER_INCLUDED__
#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include "AsmBuilder.h"
#define DEBUG_TYPE "remniw-AsmBuilderHelper"
#define BRG_UNDEF 0
#define BRG_CONST 1
#define BRG_LABEL 2
#define BRG_ARGS 3
#define BRG_REG 4
#define BRG_RET 5
#define BRG_BR 6
#define BRG_SWITCH 7
#define BRG_INDIRECTBR 8
#define BRG_INVOKE 9
#define BRG_RESUME 10
#define BRG_UNREACHABLE 11
#define BRG_CLEANUPRET 12
#define BRG_CATCHRET 13
#define BRG_CATCHSWITCH 14
#define BRG_CALLBR 15
#define BRG_FNEG 16
#define BRG_ADD 17
#define BRG_FADD 18
#define BRG_SUB 19
#define BRG_FSUB 20
#define BRG_MUL 21
#define BRG_FMUL 22
#define BRG_UDIV 23
#define BRG_SDIV 24
#define BRG_FDIV 25
#define BRG_UREM 26
#define BRG_SREM 27
#define BRG_FREM 28
#define BRG_SHL 29
#define BRG_LSHR 30
#define BRG_ASHR 31
#define BRG_AND 32
#define BRG_OR 33
#define BRG_XOR 34
#define BRG_ALLOCA 35
#define BRG_LOAD 36
#define BRG_STORE 37
#define BRG_GETELEMENTPTR 38
#define BRG_FENCE 39
#define BRG_ATOMICCMPXCHG 40
#define BRG_ATOMICRMW 41
#define BRG_TRUNC 42
#define BRG_ZEXT 43
#define BRG_SEXT 44
#define BRG_FPTOUI 45
#define BRG_FPTOSI 46
#define BRG_UITOFP 47
#define BRG_SITOFP 48
#define BRG_FPTRUNC 49
#define BRG_FPEXT 50
#define BRG_PTRTOINT 51
#define BRG_INTTOPTR 52
#define BRG_BITCAST 53
#define BRG_ADDRSPACECAST 54
#define BRG_CLEANUPPAD 55
#define BRG_CATCHPAD 56
#define BRG_ICMP 57
#define BRG_FCMP 58
#define BRG_PHI 59
#define BRG_CALL 60
#define BRG_SELECT 61
#define BRG_USEROP1 62
#define BRG_USEROP2 63
#define BRG_VAARG 64
#define BRG_EXTRACTELEMENT 65
#define BRG_INSERTELEMENT 66
#define BRG_SHUFFLEVECTOR 67
#define BRG_EXTRACTVALUE 68
#define BRG_INSERTVALUE 69
#define BRG_LANDINGPAD 70
#define BRG_FREEZE 71

struct burm_state {
  int op;
  NODEPTR node;
  struct burm_state **kids;
  COST cost[9];
  struct {
    unsigned burm_stmt:4;
    unsigned burm_reg:5;
    unsigned burm_imm:1;
    unsigned burm_mem:3;
    unsigned burm_label:1;
    unsigned burm_cond:2;
    unsigned burm_arg:3;
    unsigned burm_args:2;
  } rule;
};


struct burm_state *burm_label(NODEPTR);
struct burm_state *burm_label1(NODEPTR);

void dumpCover(NODEPTR,int,int);

#endif
