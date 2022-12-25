#include "/home/uenna1/remniw/src/codegen/asm/AsmBuilderHelper.h"
#define burm_stmt_NT 1
#define burm_reg_NT 2
#define burm_imm_NT 3
#define burm_mem_NT 4
#define burm_label_NT 5
#define burm_cond_NT 6
#define burm_arg_NT 7
#define burm_args_NT 8
extern int burm_max_nt;


void stmt_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void reg_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void imm_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void mem_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void label_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void cond_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void arg_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder, unsigned ArgNo);


void args_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder, unsigned ArgNo);
#ifndef ALLOC
#define ALLOC(n) malloc(n)
#endif

#ifndef burm_assert
#define burm_assert(x,y) if (!(x)) {  y; abort(); }
#endif

static NODEPTR burm_np;

#define burm_stmt_NT 1
#define burm_reg_NT 2
#define burm_imm_NT 3
#define burm_mem_NT 4
#define burm_label_NT 5
#define burm_cond_NT 6
#define burm_arg_NT 7
#define burm_args_NT 8
extern int burm_max_nt;
int burm_max_nt = 8;

std::string burm_ntname[] = {
  "",
  "stmt",
  "reg",
  "imm",
  "mem",
  "label",
  "cond",
  "arg",
  "args",
  ""
};

static short burm_nts_0[] = { 0 };
static short burm_nts_1[] = { burm_mem_NT, 0 };
static short burm_nts_2[] = { burm_reg_NT, 0 };
static short burm_nts_3[] = { burm_imm_NT, 0 };
static short burm_nts_4[] = { burm_reg_NT, burm_mem_NT, 0 };
static short burm_nts_5[] = { burm_reg_NT, burm_reg_NT, 0 };
static short burm_nts_6[] = { burm_imm_NT, burm_reg_NT, 0 };
static short burm_nts_7[] = { burm_imm_NT, burm_mem_NT, 0 };
static short burm_nts_8[] = { burm_mem_NT, burm_mem_NT, 0 };
static short burm_nts_9[] = { burm_label_NT, burm_mem_NT, 0 };
static short burm_nts_10[] = { burm_mem_NT, burm_imm_NT, 0 };
static short burm_nts_11[] = { burm_mem_NT, burm_reg_NT, 0 };
static short burm_nts_12[] = { burm_reg_NT, burm_imm_NT, 0 };
static short burm_nts_13[] = { burm_cond_NT, burm_label_NT, burm_label_NT, 0 };
static short burm_nts_14[] = { burm_label_NT, 0 };
static short burm_nts_15[] = { burm_imm_NT, burm_imm_NT, 0 };
static short burm_nts_16[] = { burm_arg_NT, burm_args_NT, 0 };
static short burm_nts_17[] = { burm_label_NT, burm_args_NT, 0 };
static short burm_nts_18[] = { burm_reg_NT, burm_args_NT, 0 };
static short burm_nts_19[] = { burm_mem_NT, burm_args_NT, 0 };

short *burm_nts[] = {
  burm_nts_0,  /* 0 */
  burm_nts_0,  /* 1 */
  burm_nts_0,  /* 2 */
  burm_nts_0,  /* 3 */
  burm_nts_1,  /* 4 */
  burm_nts_2,  /* 5 */
  burm_nts_2,  /* 6 */
  burm_nts_2,  /* 7 */
  burm_nts_0,  /* 8 */
  burm_nts_3,  /* 9 */
  burm_nts_4,  /* 10 */
  burm_nts_5,  /* 11 */
  burm_nts_6,  /* 12 */
  burm_nts_7,  /* 13 */
  burm_nts_8,  /* 14 */
  burm_nts_9,  /* 15 */
  burm_nts_10,  /* 16 */
  burm_nts_11,  /* 17 */
  burm_nts_12,  /* 18 */
  burm_nts_5,  /* 19 */
  burm_nts_5,  /* 20 */
  burm_nts_12,  /* 21 */
  burm_nts_6,  /* 22 */
  burm_nts_13,  /* 23 */
  burm_nts_14,  /* 24 */
  burm_nts_5,  /* 25 */
  burm_nts_12,  /* 26 */
  burm_nts_6,  /* 27 */
  burm_nts_15,  /* 28 */
  burm_nts_5,  /* 29 */
  burm_nts_12,  /* 30 */
  burm_nts_6,  /* 31 */
  burm_nts_15,  /* 32 */
  burm_nts_5,  /* 33 */
  burm_nts_12,  /* 34 */
  burm_nts_6,  /* 35 */
  burm_nts_15,  /* 36 */
  burm_nts_5,  /* 37 */
  burm_nts_12,  /* 38 */
  burm_nts_6,  /* 39 */
  burm_nts_15,  /* 40 */
  burm_nts_2,  /* 41 */
  burm_nts_3,  /* 42 */
  burm_nts_1,  /* 43 */
  burm_nts_14,  /* 44 */
  burm_nts_0,  /* 45 */
  burm_nts_16,  /* 46 */
  burm_nts_17,  /* 47 */
  burm_nts_18,  /* 48 */
  burm_nts_19,  /* 49 */
};

char burm_arity[] = {
  0,  /* 0=BRG_UNDEF */
  0,  /* 1=BRG_CONST */
  0,  /* 2=BRG_LABEL */
  2,  /* 3=BRG_ARGS */
  0,  /* 4=BRG_REG */
  1,  /* 5=BRG_RET */
  3,  /* 6=BRG_BR */
  0,  /* 7=BRG_SWITCH */
  0,  /* 8=BRG_INDIRECTBR */
  0,  /* 9=BRG_INVOKE */
  0,  /* 10=BRG_RESUME */
  0,  /* 11=BRG_UNREACHABLE */
  0,  /* 12=BRG_CLEANUPRET */
  0,  /* 13=BRG_CATCHRET */
  0,  /* 14=BRG_CATCHSWITCH */
  0,  /* 15=BRG_CALLBR */
  0,  /* 16=BRG_FNEG */
  2,  /* 17=BRG_ADD */
  0,  /* 18=BRG_FADD */
  2,  /* 19=BRG_SUB */
  0,  /* 20=BRG_FSUB */
  2,  /* 21=BRG_MUL */
  0,  /* 22=BRG_FMUL */
  0,  /* 23=BRG_UDIV */
  2,  /* 24=BRG_SDIV */
  0,  /* 25=BRG_FDIV */
  0,  /* 26=BRG_UREM */
  0,  /* 27=BRG_SREM */
  0,  /* 28=BRG_FREM */
  0,  /* 29=BRG_SHL */
  0,  /* 30=BRG_LSHR */
  0,  /* 31=BRG_ASHR */
  0,  /* 32=BRG_AND */
  0,  /* 33=BRG_OR */
  0,  /* 34=BRG_XOR */
  0,  /* 35=BRG_ALLOCA */
  1,  /* 36=BRG_LOAD */
  2,  /* 37=BRG_STORE */
  2,  /* 38=BRG_GETELEMENTPTR */
  0,  /* 39=BRG_FENCE */
  0,  /* 40=BRG_ATOMICCMPXCHG */
  0,  /* 41=BRG_ATOMICRMW */
  0,  /* 42=BRG_TRUNC */
  0,  /* 43=BRG_ZEXT */
  0,  /* 44=BRG_SEXT */
  0,  /* 45=BRG_FPTOUI */
  0,  /* 46=BRG_FPTOSI */
  0,  /* 47=BRG_UITOFP */
  0,  /* 48=BRG_SITOFP */
  0,  /* 49=BRG_FPTRUNC */
  0,  /* 50=BRG_FPEXT */
  0,  /* 51=BRG_PTRTOINT */
  0,  /* 52=BRG_INTTOPTR */
  0,  /* 53=BRG_BITCAST */
  0,  /* 54=BRG_ADDRSPACECAST */
  0,  /* 55=BRG_CLEANUPPAD */
  0,  /* 56=BRG_CATCHPAD */
  2,  /* 57=BRG_ICMP */
  0,  /* 58=BRG_FCMP */
  0,  /* 59=BRG_PHI */
  2,  /* 60=BRG_CALL */
  0,  /* 61=BRG_SELECT */
  0,  /* 62=BRG_USEROP1 */
  0,  /* 63=BRG_USEROP2 */
  0,  /* 64=BRG_VAARG */
  0,  /* 65=BRG_EXTRACTELEMENT */
  0,  /* 66=BRG_INSERTELEMENT */
  0,  /* 67=BRG_SHUFFLEVECTOR */
  0,  /* 68=BRG_EXTRACTVALUE */
  0,  /* 69=BRG_INSERTVALUE */
  0,  /* 70=BRG_LANDINGPAD */
  0,  /* 71=BRG_FREEZE */
};

std::string burm_opname[] = {
  /* 0 */  "BRG_UNDEF",
  /* 1 */  "BRG_CONST",
  /* 2 */  "BRG_LABEL",
  /* 3 */  "BRG_ARGS",
  /* 4 */  "BRG_REG",
  /* 5 */  "BRG_RET",
  /* 6 */  "BRG_BR",
  /* 7 */  "BRG_SWITCH",
  /* 8 */  "BRG_INDIRECTBR",
  /* 9 */  "BRG_INVOKE",
  /* 10 */  "BRG_RESUME",
  /* 11 */  "BRG_UNREACHABLE",
  /* 12 */  "BRG_CLEANUPRET",
  /* 13 */  "BRG_CATCHRET",
  /* 14 */  "BRG_CATCHSWITCH",
  /* 15 */  "BRG_CALLBR",
  /* 16 */  "BRG_FNEG",
  /* 17 */  "BRG_ADD",
  /* 18 */  "BRG_FADD",
  /* 19 */  "BRG_SUB",
  /* 20 */  "BRG_FSUB",
  /* 21 */  "BRG_MUL",
  /* 22 */  "BRG_FMUL",
  /* 23 */  "BRG_UDIV",
  /* 24 */  "BRG_SDIV",
  /* 25 */  "BRG_FDIV",
  /* 26 */  "BRG_UREM",
  /* 27 */  "BRG_SREM",
  /* 28 */  "BRG_FREM",
  /* 29 */  "BRG_SHL",
  /* 30 */  "BRG_LSHR",
  /* 31 */  "BRG_ASHR",
  /* 32 */  "BRG_AND",
  /* 33 */  "BRG_OR",
  /* 34 */  "BRG_XOR",
  /* 35 */  "BRG_ALLOCA",
  /* 36 */  "BRG_LOAD",
  /* 37 */  "BRG_STORE",
  /* 38 */  "BRG_GETELEMENTPTR",
  /* 39 */  "BRG_FENCE",
  /* 40 */  "BRG_ATOMICCMPXCHG",
  /* 41 */  "BRG_ATOMICRMW",
  /* 42 */  "BRG_TRUNC",
  /* 43 */  "BRG_ZEXT",
  /* 44 */  "BRG_SEXT",
  /* 45 */  "BRG_FPTOUI",
  /* 46 */  "BRG_FPTOSI",
  /* 47 */  "BRG_UITOFP",
  /* 48 */  "BRG_SITOFP",
  /* 49 */  "BRG_FPTRUNC",
  /* 50 */  "BRG_FPEXT",
  /* 51 */  "BRG_PTRTOINT",
  /* 52 */  "BRG_INTTOPTR",
  /* 53 */  "BRG_BITCAST",
  /* 54 */  "BRG_ADDRSPACECAST",
  /* 55 */  "BRG_CLEANUPPAD",
  /* 56 */  "BRG_CATCHPAD",
  /* 57 */  "BRG_ICMP",
  /* 58 */  "BRG_FCMP",
  /* 59 */  "BRG_PHI",
  /* 60 */  "BRG_CALL",
  /* 61 */  "BRG_SELECT",
  /* 62 */  "BRG_USEROP1",
  /* 63 */  "BRG_USEROP2",
  /* 64 */  "BRG_VAARG",
  /* 65 */  "BRG_EXTRACTELEMENT",
  /* 66 */  "BRG_INSERTELEMENT",
  /* 67 */  "BRG_SHUFFLEVECTOR",
  /* 68 */  "BRG_EXTRACTVALUE",
  /* 69 */  "BRG_INSERTVALUE",
  /* 70 */  "BRG_LANDINGPAD",
  /* 71 */  "BRG_FREEZE",
};


std::string burm_string[] = {
  /* 0 */  "label: BRG_LABEL",
  /* 1 */  "imm: BRG_CONST",
  /* 2 */  "reg: BRG_REG",
  /* 3 */  "mem: BRG_ALLOCA",
  /* 4 */  "reg: BRG_LOAD(mem)",
  /* 5 */  "reg: BRG_LOAD(reg)",
  /* 6 */  "stmt: reg",
  /* 7 */  "stmt: BRG_RET(reg)",
  /* 8 */  "stmt: BRG_RET(BRG_UNDEF)",
  /* 9 */  "stmt: BRG_RET(imm)",
  /* 10 */  "stmt: BRG_STORE(reg,mem)",
  /* 11 */  "stmt: BRG_STORE(reg,reg)",
  /* 12 */  "stmt: BRG_STORE(imm,reg)",
  /* 13 */  "stmt: BRG_STORE(imm,mem)",
  /* 14 */  "stmt: BRG_STORE(mem,mem)",
  /* 15 */  "stmt: BRG_STORE(label,mem)",
  /* 16 */  "mem: BRG_GETELEMENTPTR(mem,imm)",
  /* 17 */  "mem: BRG_GETELEMENTPTR(mem,reg)",
  /* 18 */  "mem: BRG_GETELEMENTPTR(reg,imm)",
  /* 19 */  "mem: BRG_GETELEMENTPTR(reg,reg)",
  /* 20 */  "cond: BRG_ICMP(reg,reg)",
  /* 21 */  "cond: BRG_ICMP(reg,imm)",
  /* 22 */  "cond: BRG_ICMP(imm,reg)",
  /* 23 */  "stmt: BRG_BR(cond,label,label)",
  /* 24 */  "stmt: BRG_BR(label,BRG_UNDEF,BRG_UNDEF)",
  /* 25 */  "reg: BRG_ADD(reg,reg)",
  /* 26 */  "reg: BRG_ADD(reg,imm)",
  /* 27 */  "reg: BRG_ADD(imm,reg)",
  /* 28 */  "reg: BRG_ADD(imm,imm)",
  /* 29 */  "reg: BRG_SUB(reg,reg)",
  /* 30 */  "reg: BRG_SUB(reg,imm)",
  /* 31 */  "reg: BRG_SUB(imm,reg)",
  /* 32 */  "reg: BRG_SUB(imm,imm)",
  /* 33 */  "reg: BRG_MUL(reg,reg)",
  /* 34 */  "reg: BRG_MUL(reg,imm)",
  /* 35 */  "reg: BRG_MUL(imm,reg)",
  /* 36 */  "reg: BRG_MUL(imm,imm)",
  /* 37 */  "reg: BRG_SDIV(reg,reg)",
  /* 38 */  "reg: BRG_SDIV(reg,imm)",
  /* 39 */  "reg: BRG_SDIV(imm,reg)",
  /* 40 */  "reg: BRG_SDIV(imm,imm)",
  /* 41 */  "arg: reg",
  /* 42 */  "arg: imm",
  /* 43 */  "arg: mem",
  /* 44 */  "arg: label",
  /* 45 */  "args: BRG_ARGS(BRG_UNDEF,BRG_UNDEF)",
  /* 46 */  "args: BRG_ARGS(arg,args)",
  /* 47 */  "reg: BRG_CALL(label,args)",
  /* 48 */  "reg: BRG_CALL(reg,args)",
  /* 49 */  "reg: BRG_CALL(mem,args)",
};


std::string burm_files[] = {
"/home/uenna1/remniw/src/codegen/asm/AsmBuilderHelper.brg",
};

int burm_file_numbers[] = {
  /* 0 */  0,
  /* 1 */  0,
  /* 2 */  0,
  /* 3 */  0,
  /* 4 */  0,
  /* 5 */  0,
  /* 6 */  0,
  /* 7 */  0,
  /* 8 */  0,
  /* 9 */  0,
  /* 10 */  0,
  /* 11 */  0,
  /* 12 */  0,
  /* 13 */  0,
  /* 14 */  0,
  /* 15 */  0,
  /* 16 */  0,
  /* 17 */  0,
  /* 18 */  0,
  /* 19 */  0,
  /* 20 */  0,
  /* 21 */  0,
  /* 22 */  0,
  /* 23 */  0,
  /* 24 */  0,
  /* 25 */  0,
  /* 26 */  0,
  /* 27 */  0,
  /* 28 */  0,
  /* 29 */  0,
  /* 30 */  0,
  /* 31 */  0,
  /* 32 */  0,
  /* 33 */  0,
  /* 34 */  0,
  /* 35 */  0,
  /* 36 */  0,
  /* 37 */  0,
  /* 38 */  0,
  /* 39 */  0,
  /* 40 */  0,
  /* 41 */  0,
  /* 42 */  0,
  /* 43 */  0,
  /* 44 */  0,
  /* 45 */  0,
  /* 46 */  0,
  /* 47 */  0,
  /* 48 */  0,
  /* 49 */  0,
};

int burm_line_numbers[] = {
  /* 0 */  96,
  /* 1 */  101,
  /* 2 */  106,
  /* 3 */  111,
  /* 4 */  116,
  /* 5 */  128,
  /* 6 */  140,
  /* 7 */  146,
  /* 8 */  153,
  /* 9 */  158,
  /* 10 */  165,
  /* 11 */  174,
  /* 12 */  183,
  /* 13 */  192,
  /* 14 */  201,
  /* 15 */  221,
  /* 16 */  230,
  /* 17 */  242,
  /* 18 */  252,
  /* 19 */  262,
  /* 20 */  273,
  /* 21 */  285,
  /* 22 */  297,
  /* 23 */  311,
  /* 24 */  320,
  /* 25 */  327,
  /* 26 */  340,
  /* 27 */  353,
  /* 28 */  366,
  /* 29 */  377,
  /* 30 */  390,
  /* 31 */  403,
  /* 32 */  416,
  /* 33 */  427,
  /* 34 */  440,
  /* 35 */  453,
  /* 36 */  466,
  /* 37 */  477,
  /* 38 */  490,
  /* 39 */  503,
  /* 40 */  516,
  /* 41 */  527,
  /* 42 */  534,
  /* 43 */  541,
  /* 44 */  548,
  /* 45 */  555,
  /* 46 */  560,
  /* 47 */  568,
  /* 48 */  582,
  /* 49 */  596,
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static short burm_decode_stmt[] = {
   -1,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  23,
  24,
};

static short burm_decode_reg[] = {
   -1,
  2,
  4,
  5,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  32,
  33,
  34,
  35,
  36,
  37,
  38,
  39,
  40,
  47,
  48,
  49,
};

static short burm_decode_imm[] = {
   -1,
  1,
};

static short burm_decode_mem[] = {
   -1,
  3,
  16,
  17,
  18,
  19,
};

static short burm_decode_label[] = {
   -1,
  0,
};

static short burm_decode_cond[] = {
   -1,
  20,
  21,
  22,
};

static short burm_decode_arg[] = {
   -1,
  41,
  42,
  43,
  44,
};

static short burm_decode_args[] = {
   -1,
  45,
  46,
};

int burm_rule(struct burm_state *state, int goalnt) {
  burm_assert(goalnt >= 1 && goalnt <= 8,
        PANIC("Bad goal nonterminal %d in burm_rule\n", goalnt));
  if (!state)
    return 0;
  switch (goalnt) {
  case burm_stmt_NT:  return burm_decode_stmt[((struct burm_state *)state)->rule.burm_stmt];
  case burm_reg_NT:  return burm_decode_reg[((struct burm_state *)state)->rule.burm_reg];
  case burm_imm_NT:  return burm_decode_imm[((struct burm_state *)state)->rule.burm_imm];
  case burm_mem_NT:  return burm_decode_mem[((struct burm_state *)state)->rule.burm_mem];
  case burm_label_NT:  return burm_decode_label[((struct burm_state *)state)->rule.burm_label];
  case burm_cond_NT:  return burm_decode_cond[((struct burm_state *)state)->rule.burm_cond];
  case burm_arg_NT:  return burm_decode_arg[((struct burm_state *)state)->rule.burm_arg];
  case burm_args_NT:  return burm_decode_args[((struct burm_state *)state)->rule.burm_args];
  default:
    burm_assert(0, PANIC("Bad goal nonterminal %d in burm_rule\n", goalnt));
  }
  return 0;
}


struct burm_action {
  int nt;
  struct burm_state* st;
};

#ifndef RULE
#define RULE(n,s) \
    (act = (burm_action*) malloc(sizeof(struct burm_action)),act->nt=n,act->st=s,act)
#endif

int burm_cost_code(COST *_c, int _ern,struct burm_state *_s)
{
  NODEPTR *_children;
  struct burm_action *act;
  switch(_ern){
  default:
    DEFAULT_COST;
  case 0:
{


 (*_c).cost = 0; 
}
  break;
  case 1:
{


 (*_c).cost = 0; 
}
  break;
  case 2:
{


 (*_c).cost = 0; 
}
  break;
  case 3:
{


 (*_c).cost = 0; 
}
  break;
  case 4:
{


 (*_c).cost = _s->kids[0]->cost[burm_mem_NT].cost + 1; 
}
  break;
  case 5:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 6:
{


 (*_c).cost = _s->cost[burm_reg_NT].cost; 
}
  break;
  case 7:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 8:
{


 (*_c).cost = 0; 
}
  break;
  case 9:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 10:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_mem_NT].cost + 1; 
}
  break;
  case 11:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 12:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 13:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_mem_NT].cost + 1; 
}
  break;
  case 14:
{


 (*_c).cost = _s->kids[0]->cost[burm_mem_NT].cost + _s->kids[1]->cost[burm_mem_NT].cost + 1; 
}
  break;
  case 15:
{


 (*_c).cost = _s->kids[0]->cost[burm_label_NT].cost + _s->kids[1]->cost[burm_mem_NT].cost + 1; 
}
  break;
  case 16:
{


 (*_c).cost = _s->kids[0]->cost[burm_mem_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 17:
{


 (*_c).cost = _s->kids[0]->cost[burm_mem_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 18:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 19:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 20:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 21:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 22:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 23:
{


 (*_c).cost = _s->kids[0]->cost[burm_cond_NT].cost + _s->kids[1]->cost[burm_label_NT].cost + _s->kids[2]->cost[burm_label_NT].cost + 1; 
}
  break;
  case 24:
{


 (*_c).cost = _s->kids[0]->cost[burm_label_NT].cost + 1; 
}
  break;
  case 25:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 26:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 27:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 28:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 29:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 30:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 31:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 2; 
}
  break;
  case 32:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 33:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 34:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 35:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 36:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 37:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 2; 
}
  break;
  case 38:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 3; 
}
  break;
  case 39:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_reg_NT].cost + 2; 
}
  break;
  case 40:
{


 (*_c).cost = _s->kids[0]->cost[burm_imm_NT].cost + _s->kids[1]->cost[burm_imm_NT].cost + 3; 
}
  break;
  case 41:
{


 (*_c).cost = _s->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 42:
{


 (*_c).cost = _s->cost[burm_imm_NT].cost + 1; 
}
  break;
  case 43:
{


 (*_c).cost = _s->cost[burm_mem_NT].cost + 1; 
}
  break;
  case 44:
{


 (*_c).cost = _s->cost[burm_label_NT].cost + 1; 
}
  break;
  case 45:
{


 (*_c).cost = 0; 
}
  break;
  case 46:
{


 (*_c).cost = _s->kids[0]->cost[burm_arg_NT].cost + _s->kids[1]->cost[burm_args_NT].cost; 
}
  break;
  case 47:
{


 (*_c).cost = _s->kids[0]->cost[burm_label_NT].cost + 1; 
}
  break;
  case 48:
{


 (*_c).cost = _s->kids[0]->cost[burm_reg_NT].cost + 1; 
}
  break;
  case 49:
{


 (*_c).cost = _s->kids[0]->cost[burm_mem_NT].cost + 1; 
}
  break;
  }
  return 1;
}


void stmt_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void reg_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void imm_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void mem_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void label_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void cond_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder);


void arg_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder, unsigned ArgNo);


void args_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder, unsigned ArgNo);


#include <stdarg.h>

void burm_exec(struct burm_state *state, int nterm, ...) 
{
  va_list(ap);
  va_start(ap,nterm);

  burm_assert(nterm >= 1 && nterm <= 8,
        PANIC("Bad nonterminal %d in $exec\n", nterm));

  if (state)
    switch (nterm) {
    case burm_stmt_NT:
      stmt_action(state,va_arg(ap,remniw::AsmBuilderPtr));
      break;
    case burm_reg_NT:
      reg_action(state,va_arg(ap,remniw::AsmBuilderPtr));
      break;
    case burm_imm_NT:
      imm_action(state,va_arg(ap,remniw::AsmBuilderPtr));
      break;
    case burm_mem_NT:
      mem_action(state,va_arg(ap,remniw::AsmBuilderPtr));
      break;
    case burm_label_NT:
      label_action(state,va_arg(ap,remniw::AsmBuilderPtr));
      break;
    case burm_cond_NT:
      cond_action(state,va_arg(ap,remniw::AsmBuilderPtr));
      break;
    case burm_arg_NT:
      arg_action(state,va_arg(ap,remniw::AsmBuilderPtr),va_arg(ap,unsigned));
      break;
    case burm_args_NT:
      args_action(state,va_arg(ap,remniw::AsmBuilderPtr),va_arg(ap,unsigned));
      break;
    default:
      PANIC("Bad nonterminal %d in $exec\n", nterm);
      break;
    }
  else
    PANIC("Bad state for $exec in nonterminal %d \n",nterm);
  va_end(ap);
}

#define EXEC(s,n,a) ( \
  (n == burm_stmt_NT)? burm_exec(s,n,a): \
  (n == burm_reg_NT)? burm_exec(s,n,a): \
  (n == burm_imm_NT)? burm_exec(s,n,a): \
  (n == burm_mem_NT)? burm_exec(s,n,a): \
  (n == burm_label_NT)? burm_exec(s,n,a): \
  (n == burm_cond_NT)? burm_exec(s,n,a): \
  (n == burm_arg_NT)? burm_exec(s,n,a): \
  (n == burm_args_NT)? burm_exec(s,n,a): \
  PANIC("Bad nonterminal %d in $exec\n", n))

struct burm_state *burm_immed(struct burm_state *s,int n);
#ifndef NO_ACTION
#define NO_ACTION assert(0)
#endif


void stmt_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder)
{
  struct burm_state *_t;
  int _ern=burm_decode_stmt[_s->rule.burm_stmt];
  NODEPTR *_children;
  if(_s->rule.burm_stmt==0)
    NO_ACTION(stmt);
  switch(_ern){
  case 6:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: reg\n";);
    reg_action(_s,Builder);

}
  break;
  case 7:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: BRG_RET(reg)\n";);
    reg_action(_s->kids[0],Builder);
    Builder->handleRET(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg());

}
  break;
  case 8:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: BRG_RET(BRG_UNDEF)\n";);

}
  break;
  case 9:
{



    LLVM_DEBUG(llvm::outs() << "brg action: " << "stmt: BRG_RET(imm)\n";);
    imm_action(_s->kids[0],Builder);
    Builder->handleRET(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandImm());

}
  break;
  case 10:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: BRG_STORE(reg, mem)\n";);
    reg_action(_s->kids[0],Builder);
    mem_action(_s->kids[1],Builder);
    Builder->handleSTORE(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(),
                         _s->kids[1]->node->getAsAsmOperandMem());

}
  break;
  case 11:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: BRG_STORE(reg, reg)\n";);
    reg_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    Builder->handleSTORE(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(),
                         _s->kids[1]->node->getAsAsmOperandReg());

}
  break;
  case 12:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: BRG_STORE(imm, reg)\n";);
    imm_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    Builder->handleSTORE(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandImm(),
                         _s->kids[1]->node->getAsAsmOperandReg());

}
  break;
  case 13:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: BRG_STORE(imm, mem)\n";);
    imm_action(_s->kids[0],Builder);
    mem_action(_s->kids[1],Builder);
    Builder->handleSTORE(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandImm(),
                         _s->kids[1]->node->getAsAsmOperandMem());

}
  break;
  case 14:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: BRG_STORE(mem, mem)\n";);
    mem_action(_s->kids[0],Builder);
    mem_action(_s->kids[1],Builder);
    auto *SI = llvm::cast<llvm::StoreInst>(_s->node->getInstruction());
    // In BrgTreeBuilder, we construct MemBrgTreeNode for argument.
    // So here we specially handle StoreInst which initializes
    // permanent storage for a function argument
    if (llvm::isa<llvm::Argument>(SI->getValueOperand())) {
        Builder->handleSTORE(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandMem(),
                             _s->kids[1]->node->getAsAsmOperandMem(),
                             /*bool DestIsArgument=*/true);
    } else {
        Builder->handleSTORE(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandMem(),
                             _s->kids[1]->node->getAsAsmOperandMem(),
                             /*bool DestIsArgument=*/false);
    }

}
  break;
  case 15:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: BRG_STORE(label, mem)\n";);
    label_action(_s->kids[0],Builder);
    mem_action(_s->kids[1],Builder);
    Builder->handleSTORE(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandLabel(),
                         _s->kids[1]->node->getAsAsmOperandMem());

}
  break;
  case 23:
{



    LLVM_DEBUG(llvm::outs() << "brg action: stmt: BRG_BR(cond, label, label)\n";);
    cond_action(_s->kids[0],Builder);
    Builder->handleBR(_s->node->getInstruction(), _s->kids[1]->node->getAsAsmOperandLabel(),
                      _s->kids[2]->node->getAsAsmOperandLabel());

}
  break;
  case 24:
{



    LLVM_DEBUG(
        llvm::outs() << "brg action: stmt: BRG_BR(label, BRG_UNDEF, BRG_UNDEF)\n";);
    Builder->handleBR(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandLabel());

}
  break;
  }
}


void reg_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder)
{
  struct burm_state *_t;
  int _ern=burm_decode_reg[_s->rule.burm_reg];
  NODEPTR *_children;
  if(_s->rule.burm_reg==0)
    NO_ACTION(reg);
  switch(_ern){
  case 2:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_REG\n";);

}
  break;
  case 4:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_LOAD(mem)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    mem_action(_s->kids[0],Builder);
    remniw::AsmOperand::RegOp Reg =
        Builder->handleLOAD(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandMem());
    _s->node->setReg(Reg);

}
  break;
  case 5:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_LOAD(reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    remniw::AsmOperand::RegOp Reg =
        Builder->handleLOAD(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 25:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_ADD(reg, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleADD(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 26:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_ADD(reg, imm)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleADD(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(), _s->kids[1]->node->getAsAsmOperandImm());
    _s->node->setReg(Reg);

}
  break;
  case 27:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_ADD(imm, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    imm_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleADD(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandImm(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 28:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_ADD(imm, imm)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    imm_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    llvm_unreachable("Unexpected action: reg: BRG_ADD(imm, imm)!\n");

}
  break;
  case 29:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_SUB(reg, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleSUB(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandImm(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 30:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_SUB(reg, imm)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleSUB(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(), _s->kids[1]->node->getAsAsmOperandImm());
    _s->node->setReg(Reg);

}
  break;
  case 31:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_SUB(imm, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    imm_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleSUB(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandImm(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 32:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_SUB(imm, imm)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    imm_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    llvm_unreachable("Unexpected action reg: BRG_SUB(imm, imm)\n");

}
  break;
  case 33:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_MUL(reg, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleMUL(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 34:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_MUL(reg, imm)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleMUL(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(), _s->kids[1]->node->getAsAsmOperandImm());
    _s->node->setReg(Reg);

}
  break;
  case 35:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_MUL(imm, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    imm_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleMUL(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandImm(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 36:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_MUL(imm, imm)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    imm_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    llvm_unreachable("Unexpected action reg: BRG_MUL(imm, imm)\n");

}
  break;
  case 37:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_SDIV(reg, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleSDIV(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 38:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_SDIV(reg, imm)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleSDIV(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(), _s->kids[1]->node->getAsAsmOperandImm());
    _s->node->setReg(Reg);

}
  break;
  case 39:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_SDIV(imm, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    imm_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    remniw::AsmOperand::RegOp Reg = Builder->handleSDIV(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandImm(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 40:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_SDIV(imm, imm)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    imm_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    llvm_unreachable("Unexpected action reg: BRG_SDIV(imm, imm)!\n");

}
  break;
  case 47:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_CALL(label, args)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    label_action(_s->kids[0],Builder);
    args_action(_s->kids[1],Builder, 0);
    remniw::AsmOperand::RegOp Reg =
        Builder->handleCALL(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandLabel());
    _s->node->setReg(Reg);

}
  break;
  case 48:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_CALL(reg, args)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    args_action(_s->kids[1],Builder, 0);
    remniw::AsmOperand::RegOp Reg =
        Builder->handleCALL(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg());
    _s->node->setReg(Reg);

}
  break;
  case 49:
{



    LLVM_DEBUG(llvm::outs() << "brg action: reg: BRG_CALL(mem, args)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    mem_action(_s->kids[0],Builder);
    args_action(_s->kids[1],Builder, 0);
    remniw::AsmOperand::RegOp Reg =
        Builder->handleCALL(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandMem());
    _s->node->setReg(Reg);

}
  break;
  }
}


void imm_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder)
{
  struct burm_state *_t;
  int _ern=burm_decode_imm[_s->rule.burm_imm];
  NODEPTR *_children;
  if(_s->rule.burm_imm==0)
    NO_ACTION(imm);
  switch(_ern){
  case 1:
{



    LLVM_DEBUG(llvm::outs() << "brg action: imm: BRG_CONST\n";);

}
  break;
  }
}


void mem_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder)
{
  struct burm_state *_t;
  int _ern=burm_decode_mem[_s->rule.burm_mem];
  NODEPTR *_children;
  if(_s->rule.burm_mem==0)
    NO_ACTION(mem);
  switch(_ern){
  case 3:
{



    LLVM_DEBUG(llvm::outs() << "brg action: mem: BRG_ALLOCA\n";);

}
  break;
  case 16:
{



    // For GetElementPtrInst, if all indices are constant we can calculate the MemOp
    // offset directly
    LLVM_DEBUG(llvm::outs() << "brg action: mem: BRG_GETELEMENTPTR(mem, imm)\n";);
    mem_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    remniw::AsmOperand::MemOp Mem = Builder->handleGETELEMENTPTR(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandMem(), _s->kids[1]->node->getAsAsmOperandImm());
    _s->node->setMem(Mem);

}
  break;
  case 17:
{



    LLVM_DEBUG(llvm::outs() << "brg action: mem: BRG_GETELEMENTPTR(mem, reg)\n";);
    mem_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    remniw::AsmOperand::MemOp Mem = Builder->handleGETELEMENTPTR(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandMem(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setMem(Mem);

}
  break;
  case 18:
{



    LLVM_DEBUG(llvm::outs() << "brg action: mem: BRG_GETELEMENTPTR(reg, imm)\n";);
    reg_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    remniw::AsmOperand::MemOp Mem = Builder->handleGETELEMENTPTR(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(), _s->kids[1]->node->getAsAsmOperandImm());
    _s->node->setMem(Mem);

}
  break;
  case 19:
{



    LLVM_DEBUG(llvm::outs() << "brg action: mem: BRG_GETELEMENTPTR(reg, reg)\n";);
    reg_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    auto *GEP = llvm::cast<llvm::GetElementPtrInst>(_s->node->getInstruction());
    remniw::AsmOperand::MemOp Mem = Builder->handleGETELEMENTPTR(
        _s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(), _s->kids[1]->node->getAsAsmOperandReg());
    _s->node->setMem(Mem);

}
  break;
  }
}


void label_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder)
{
  struct burm_state *_t;
  int _ern=burm_decode_label[_s->rule.burm_label];
  NODEPTR *_children;
  if(_s->rule.burm_label==0)
    NO_ACTION(label);
  switch(_ern){
  case 0:
{



    LLVM_DEBUG(llvm::outs() << "brg action: label: BRG_LABEL\n";);

}
  break;
  }
}


void cond_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder)
{
  struct burm_state *_t;
  int _ern=burm_decode_cond[_s->rule.burm_cond];
  NODEPTR *_children;
  if(_s->rule.burm_cond==0)
    NO_ACTION(cond);
  switch(_ern){
  case 20:
{



    LLVM_DEBUG(llvm::outs() << "brg action: cond: BRG_ICMP(reg, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    Builder->handleICMP(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(),
                        _s->kids[1]->node->getAsAsmOperandReg());

}
  break;
  case 21:
{



    LLVM_DEBUG(llvm::outs() << "brg action: cond: BRG_ICMP(reg, imm)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    reg_action(_s->kids[0],Builder);
    imm_action(_s->kids[1],Builder);
    Builder->handleICMP(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandReg(),
                        _s->kids[1]->node->getAsAsmOperandImm());

}
  break;
  case 22:
{



    LLVM_DEBUG(llvm::outs() << "brg action: cond: BRG_ICMP(imm, reg)\n";);
    if (_s->node->isActionExecuted())
        return;
    _s->node->setActionExecuted();
    imm_action(_s->kids[0],Builder);
    reg_action(_s->kids[1],Builder);
    Builder->handleICMP(_s->node->getInstruction(), _s->kids[0]->node->getAsAsmOperandImm(),
                        _s->kids[1]->node->getAsAsmOperandReg());

}
  break;
  }
}


void arg_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder, unsigned ArgNo)
{
  struct burm_state *_t;
  int _ern=burm_decode_arg[_s->rule.burm_arg];
  NODEPTR *_children;
  if(_s->rule.burm_arg==0)
    NO_ACTION(arg);
  switch(_ern){
  case 41:
{



    LLVM_DEBUG(llvm::outs() << "brg action: arg: reg\n";);
    reg_action(_s,Builder);
    Builder->handleARG(ArgNo, _s->node->getAsAsmOperandReg());

}
  break;
  case 42:
{



    LLVM_DEBUG(llvm::outs() << "brg action: arg: imm\n";);
    imm_action(_s,Builder);
    Builder->handleARG(ArgNo, _s->node->getAsAsmOperandImm());

}
  break;
  case 43:
{



    LLVM_DEBUG(llvm::outs() << "brg action: arg: mem\n";);
    mem_action(_s,Builder);
    Builder->handleARG(ArgNo, _s->node->getAsAsmOperandMem());

}
  break;
  case 44:
{



    LLVM_DEBUG(llvm::outs() << "brg action: arg: label\n";);
    label_action(_s,Builder);
    Builder->handleARG(ArgNo, _s->node->getAsAsmOperandLabel());

}
  break;
  }
}


void args_action(struct burm_state *_s, 

remniw::AsmBuilderPtr Builder, unsigned ArgNo)
{
  struct burm_state *_t;
  int _ern=burm_decode_args[_s->rule.burm_args];
  NODEPTR *_children;
  if(_s->rule.burm_args==0)
    NO_ACTION(args);
  switch(_ern){
  case 45:
{



    LLVM_DEBUG(llvm::outs() << "brg action: args: BRG_ARGS(BRG_UNDEF, BRG_UNDEF)\n";);

}
  break;
  case 46:
{



    LLVM_DEBUG(llvm::outs() << "brg action: args: BRG_ARGS(arg, args)\n";);
    arg_action(_s->kids[0],Builder, ArgNo);
    args_action(_s->kids[1],Builder, ArgNo + 1);

}
  break;
  }
}
static void burm_closure_reg(struct burm_state *, COST);
static void burm_closure_imm(struct burm_state *, COST);
static void burm_closure_mem(struct burm_state *, COST);
static void burm_closure_label(struct burm_state *, COST);

static void burm_closure_reg(struct burm_state *s, COST c) {
  if(burm_cost_code(&c,41,s) && COST_LESS(c,s->cost[burm_arg_NT])) {
burm_trace(burm_np, 41, c);     s->cost[burm_arg_NT] = c ;
    s->rule.burm_arg = 1;
  }
  if(burm_cost_code(&c,6,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 6, c);     s->cost[burm_stmt_NT] = c ;
    s->rule.burm_stmt = 1;
  }
}

static void burm_closure_imm(struct burm_state *s, COST c) {
  if(burm_cost_code(&c,42,s) && COST_LESS(c,s->cost[burm_arg_NT])) {
burm_trace(burm_np, 42, c);     s->cost[burm_arg_NT] = c ;
    s->rule.burm_arg = 2;
  }
}

static void burm_closure_mem(struct burm_state *s, COST c) {
  if(burm_cost_code(&c,43,s) && COST_LESS(c,s->cost[burm_arg_NT])) {
burm_trace(burm_np, 43, c);     s->cost[burm_arg_NT] = c ;
    s->rule.burm_arg = 3;
  }
}

static void burm_closure_label(struct burm_state *s, COST c) {
  if(burm_cost_code(&c,44,s) && COST_LESS(c,s->cost[burm_arg_NT])) {
burm_trace(burm_np, 44, c);     s->cost[burm_arg_NT] = c ;
    s->rule.burm_arg = 4;
  }
}

struct burm_state *burm_alloc_state(NODEPTR u,int op,int arity)
{
  struct burm_state *p, **k;
  p = (struct burm_state *)ALLOC(sizeof *p);
  burm_assert(p, PANIC("1:ALLOC returned NULL in burm_alloc_state\n"));
    burm_np = u;
  p->op = op;
  p->node = u;
  if(arity){
    k=(struct burm_state **)ALLOC(arity*sizeof (struct burm_state *));
    burm_assert(k, PANIC("2:ALLOC returned NULL in burm_alloc_state\n"));
    p->kids=k;
  }else
    p->kids=0;
  p->rule.burm_stmt =
  p->rule.burm_reg =
  p->rule.burm_imm =
  p->rule.burm_mem =
  p->rule.burm_label =
  p->rule.burm_cond =
  p->rule.burm_arg =
  p->rule.burm_args =
    0;
  p->cost[1] =
  p->cost[2] =
  p->cost[3] =
  p->cost[4] =
  p->cost[5] =
  p->cost[6] =
  p->cost[7] =
  p->cost[8] =
    COST_INFINITY;
  return p;
}
struct burm_state *burm_label1(NODEPTR u) {
  int op, arity, i, immed_matched=0;
  COST c=COST_ZERO;
  struct burm_state *s,**k;
  NODEPTR *children;
  op=OP_LABEL(u);
  arity=burm_arity[op];
  switch(op){
  case 0:   /* BRG_UNDEF */
#ifdef LEAF_TRAP
    if(s=LEAF_TRAP(u,op))
      return s;
#endif
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=0;
    break;
  case 1:   /* BRG_CONST */
#ifdef LEAF_TRAP
    if(s=LEAF_TRAP(u,op))
      return s;
#endif
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=0;
    {      /* imm: BRG_CONST */
      if(burm_cost_code(&c,1,s) && COST_LESS(c,s->cost[burm_imm_NT])) {
burm_trace(burm_np, 1, c);         s->cost[burm_imm_NT] = c ;
        s->rule.burm_imm = 1;
        burm_closure_imm(s, c );
      }
    }
    break;
  case 2:   /* BRG_LABEL */
#ifdef LEAF_TRAP
    if(s=LEAF_TRAP(u,op))
      return s;
#endif
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=0;
    {      /* label: BRG_LABEL */
      if(burm_cost_code(&c,0,s) && COST_LESS(c,s->cost[burm_label_NT])) {
burm_trace(burm_np, 0, c);         s->cost[burm_label_NT] = c ;
        s->rule.burm_label = 1;
        burm_closure_label(s, c );
      }
    }
    break;
  case 3:   /* BRG_ARGS */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* args: BRG_ARGS(arg,args) */
      k[0]->rule.burm_arg && 
      k[1]->rule.burm_args
    ) {
      if(burm_cost_code(&c,46,s) && COST_LESS(c,s->cost[burm_args_NT])) {
burm_trace(burm_np, 46, c);         s->cost[burm_args_NT] = c ;
        s->rule.burm_args = 2;
      }
    }
    if (   /* args: BRG_ARGS(BRG_UNDEF,BRG_UNDEF) */
      k[0]->op == 0 &&  /* BRG_UNDEF */
      k[1]->op == 0 /* BRG_UNDEF */
    ) {
      if(burm_cost_code(&c,45,s) && COST_LESS(c,s->cost[burm_args_NT])) {
burm_trace(burm_np, 45, c);         s->cost[burm_args_NT] = c ;
        s->rule.burm_args = 1;
      }
    }
    break;
  case 4:   /* BRG_REG */
#ifdef LEAF_TRAP
    if(s=LEAF_TRAP(u,op))
      return s;
#endif
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=0;
    {      /* reg: BRG_REG */
      if(burm_cost_code(&c,2,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 2, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 1;
        burm_closure_reg(s, c );
      }
    }
    break;
  case 5:   /* BRG_RET */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* stmt: BRG_RET(imm) */
      k[0]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,9,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 9, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 4;
      }
    }
    if (   /* stmt: BRG_RET(BRG_UNDEF) */
      k[0]->op == 0 /* BRG_UNDEF */
    ) {
      if(burm_cost_code(&c,8,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 8, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 3;
      }
    }
    if (   /* stmt: BRG_RET(reg) */
      k[0]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,7,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 7, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 2;
      }
    }
    break;
  case 6:   /* BRG_BR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* stmt: BRG_BR(label,BRG_UNDEF,BRG_UNDEF) */
      k[0]->rule.burm_label && 
      k[1]->op == 0 &&  /* BRG_UNDEF */
      k[2]->op == 0 /* BRG_UNDEF */
    ) {
      if(burm_cost_code(&c,24,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 24, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 12;
      }
    }
    if (   /* stmt: BRG_BR(cond,label,label) */
      k[0]->rule.burm_cond && 
      k[1]->rule.burm_label && 
      k[2]->rule.burm_label
    ) {
      if(burm_cost_code(&c,23,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 23, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 11;
      }
    }
    break;
  case 7:   /* BRG_SWITCH */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 8:   /* BRG_INDIRECTBR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 9:   /* BRG_INVOKE */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 10:   /* BRG_RESUME */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 11:   /* BRG_UNREACHABLE */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 12:   /* BRG_CLEANUPRET */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 13:   /* BRG_CATCHRET */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 14:   /* BRG_CATCHSWITCH */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 15:   /* BRG_CALLBR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 16:   /* BRG_FNEG */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 17:   /* BRG_ADD */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* reg: BRG_ADD(imm,imm) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,28,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 28, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 7;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_ADD(imm,reg) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,27,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 27, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 6;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_ADD(reg,imm) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,26,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 26, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 5;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_ADD(reg,reg) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,25,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 25, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 4;
        burm_closure_reg(s, c );
      }
    }
    break;
  case 18:   /* BRG_FADD */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 19:   /* BRG_SUB */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* reg: BRG_SUB(imm,imm) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,32,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 32, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 11;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_SUB(imm,reg) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,31,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 31, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 10;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_SUB(reg,imm) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,30,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 30, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 9;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_SUB(reg,reg) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,29,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 29, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 8;
        burm_closure_reg(s, c );
      }
    }
    break;
  case 20:   /* BRG_FSUB */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 21:   /* BRG_MUL */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* reg: BRG_MUL(imm,imm) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,36,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 36, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 15;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_MUL(imm,reg) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,35,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 35, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 14;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_MUL(reg,imm) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,34,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 34, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 13;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_MUL(reg,reg) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,33,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 33, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 12;
        burm_closure_reg(s, c );
      }
    }
    break;
  case 22:   /* BRG_FMUL */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 23:   /* BRG_UDIV */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 24:   /* BRG_SDIV */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* reg: BRG_SDIV(imm,imm) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,40,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 40, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 19;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_SDIV(imm,reg) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,39,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 39, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 18;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_SDIV(reg,imm) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,38,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 38, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 17;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_SDIV(reg,reg) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,37,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 37, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 16;
        burm_closure_reg(s, c );
      }
    }
    break;
  case 25:   /* BRG_FDIV */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 26:   /* BRG_UREM */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 27:   /* BRG_SREM */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 28:   /* BRG_FREM */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 29:   /* BRG_SHL */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 30:   /* BRG_LSHR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 31:   /* BRG_ASHR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 32:   /* BRG_AND */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 33:   /* BRG_OR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 34:   /* BRG_XOR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 35:   /* BRG_ALLOCA */
#ifdef LEAF_TRAP
    if(s=LEAF_TRAP(u,op))
      return s;
#endif
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=0;
    {      /* mem: BRG_ALLOCA */
      if(burm_cost_code(&c,3,s) && COST_LESS(c,s->cost[burm_mem_NT])) {
burm_trace(burm_np, 3, c);         s->cost[burm_mem_NT] = c ;
        s->rule.burm_mem = 1;
        burm_closure_mem(s, c );
      }
    }
    break;
  case 36:   /* BRG_LOAD */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* reg: BRG_LOAD(reg) */
      k[0]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,5,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 5, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 3;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_LOAD(mem) */
      k[0]->rule.burm_mem
    ) {
      if(burm_cost_code(&c,4,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 4, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 2;
        burm_closure_reg(s, c );
      }
    }
    break;
  case 37:   /* BRG_STORE */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* stmt: BRG_STORE(label,mem) */
      k[0]->rule.burm_label && 
      k[1]->rule.burm_mem
    ) {
      if(burm_cost_code(&c,15,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 15, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 10;
      }
    }
    if (   /* stmt: BRG_STORE(mem,mem) */
      k[0]->rule.burm_mem && 
      k[1]->rule.burm_mem
    ) {
      if(burm_cost_code(&c,14,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 14, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 9;
      }
    }
    if (   /* stmt: BRG_STORE(imm,mem) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_mem
    ) {
      if(burm_cost_code(&c,13,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 13, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 8;
      }
    }
    if (   /* stmt: BRG_STORE(imm,reg) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,12,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 12, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 7;
      }
    }
    if (   /* stmt: BRG_STORE(reg,reg) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,11,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 11, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 6;
      }
    }
    if (   /* stmt: BRG_STORE(reg,mem) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_mem
    ) {
      if(burm_cost_code(&c,10,s) && COST_LESS(c,s->cost[burm_stmt_NT])) {
burm_trace(burm_np, 10, c);         s->cost[burm_stmt_NT] = c ;
        s->rule.burm_stmt = 5;
      }
    }
    break;
  case 38:   /* BRG_GETELEMENTPTR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* mem: BRG_GETELEMENTPTR(reg,reg) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,19,s) && COST_LESS(c,s->cost[burm_mem_NT])) {
burm_trace(burm_np, 19, c);         s->cost[burm_mem_NT] = c ;
        s->rule.burm_mem = 5;
        burm_closure_mem(s, c );
      }
    }
    if (   /* mem: BRG_GETELEMENTPTR(reg,imm) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,18,s) && COST_LESS(c,s->cost[burm_mem_NT])) {
burm_trace(burm_np, 18, c);         s->cost[burm_mem_NT] = c ;
        s->rule.burm_mem = 4;
        burm_closure_mem(s, c );
      }
    }
    if (   /* mem: BRG_GETELEMENTPTR(mem,reg) */
      k[0]->rule.burm_mem && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,17,s) && COST_LESS(c,s->cost[burm_mem_NT])) {
burm_trace(burm_np, 17, c);         s->cost[burm_mem_NT] = c ;
        s->rule.burm_mem = 3;
        burm_closure_mem(s, c );
      }
    }
    if (   /* mem: BRG_GETELEMENTPTR(mem,imm) */
      k[0]->rule.burm_mem && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,16,s) && COST_LESS(c,s->cost[burm_mem_NT])) {
burm_trace(burm_np, 16, c);         s->cost[burm_mem_NT] = c ;
        s->rule.burm_mem = 2;
        burm_closure_mem(s, c );
      }
    }
    break;
  case 39:   /* BRG_FENCE */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 40:   /* BRG_ATOMICCMPXCHG */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 41:   /* BRG_ATOMICRMW */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 42:   /* BRG_TRUNC */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 43:   /* BRG_ZEXT */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 44:   /* BRG_SEXT */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 45:   /* BRG_FPTOUI */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 46:   /* BRG_FPTOSI */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 47:   /* BRG_UITOFP */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 48:   /* BRG_SITOFP */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 49:   /* BRG_FPTRUNC */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 50:   /* BRG_FPEXT */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 51:   /* BRG_PTRTOINT */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 52:   /* BRG_INTTOPTR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 53:   /* BRG_BITCAST */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 54:   /* BRG_ADDRSPACECAST */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 55:   /* BRG_CLEANUPPAD */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 56:   /* BRG_CATCHPAD */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 57:   /* BRG_ICMP */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* cond: BRG_ICMP(imm,reg) */
      k[0]->rule.burm_imm && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,22,s) && COST_LESS(c,s->cost[burm_cond_NT])) {
burm_trace(burm_np, 22, c);         s->cost[burm_cond_NT] = c ;
        s->rule.burm_cond = 3;
      }
    }
    if (   /* cond: BRG_ICMP(reg,imm) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_imm
    ) {
      if(burm_cost_code(&c,21,s) && COST_LESS(c,s->cost[burm_cond_NT])) {
burm_trace(burm_np, 21, c);         s->cost[burm_cond_NT] = c ;
        s->rule.burm_cond = 2;
      }
    }
    if (   /* cond: BRG_ICMP(reg,reg) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_reg
    ) {
      if(burm_cost_code(&c,20,s) && COST_LESS(c,s->cost[burm_cond_NT])) {
burm_trace(burm_np, 20, c);         s->cost[burm_cond_NT] = c ;
        s->rule.burm_cond = 1;
      }
    }
    break;
  case 58:   /* BRG_FCMP */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 59:   /* BRG_PHI */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 60:   /* BRG_CALL */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    if (   /* reg: BRG_CALL(mem,args) */
      k[0]->rule.burm_mem && 
      k[1]->rule.burm_args
    ) {
      if(burm_cost_code(&c,49,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 49, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 22;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_CALL(reg,args) */
      k[0]->rule.burm_reg && 
      k[1]->rule.burm_args
    ) {
      if(burm_cost_code(&c,48,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 48, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 21;
        burm_closure_reg(s, c );
      }
    }
    if (   /* reg: BRG_CALL(label,args) */
      k[0]->rule.burm_label && 
      k[1]->rule.burm_args
    ) {
      if(burm_cost_code(&c,47,s) && COST_LESS(c,s->cost[burm_reg_NT])) {
burm_trace(burm_np, 47, c);         s->cost[burm_reg_NT] = c ;
        s->rule.burm_reg = 20;
        burm_closure_reg(s, c );
      }
    }
    break;
  case 61:   /* BRG_SELECT */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 62:   /* BRG_USEROP1 */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 63:   /* BRG_USEROP2 */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 64:   /* BRG_VAARG */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 65:   /* BRG_EXTRACTELEMENT */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 66:   /* BRG_INSERTELEMENT */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 67:   /* BRG_SHUFFLEVECTOR */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 68:   /* BRG_EXTRACTVALUE */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 69:   /* BRG_INSERTVALUE */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 70:   /* BRG_LANDINGPAD */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  case 71:   /* BRG_FREEZE */
    s=burm_alloc_state(u,op,arity);
    SET_STATE(u,s);
    k=s->kids;
    children=GET_KIDS(u);
    for(i=0;i<arity;i++)
      k[i]=burm_label1(children[i]);
    break;
  default:
    burm_assert(0, PANIC("Bad operator %d in burm_state\n", op));
  }
  return s;
}

struct burm_state *burm_label(NODEPTR p) {
  burm_label1(p);
  return ((struct burm_state *)STATE_LABEL(p))->rule.burm_stmt ? STATE_LABEL(p) : 0;
}

void burm_free(struct burm_state *s)
{
  int i,arity=burm_arity[s->op];
  if(s->kids==0)
    free(s);
  else {
    for(i=0;i<arity;i++)
      burm_free(s->kids[i]);
    free(s->kids);free(s);
  }
}
struct burm_state *burm_immed(struct burm_state *s,int n)
{
  NODEPTR *children = GET_KIDS(s->node);
  if(s->kids[n])
    return s->kids[n];
  else
  return s->kids[n]=burm_label1(children[n]);
}
int burm_op_label(NODEPTR p) {
  burm_assert(p, PANIC("NULL tree in burm_op_label\n"));
  return OP_LABEL(p);
}

struct burm_state *burm_state_label(NODEPTR p) {
  burm_assert(p, PANIC("NULL tree in burm_state_label\n"));
  return STATE_LABEL(p);
}

NODEPTR burm_child(NODEPTR p, int index) {
  NODEPTR *kids;
  burm_assert(p, PANIC("NULL tree in burm_child\n"));
  kids=GET_KIDS(p);
  burm_assert((0<=index && index<burm_arity[OP_LABEL(p)]),
    PANIC("Bad index %d in burm_child\n", index));

  return kids[index];
}
NODEPTR *burm_kids(NODEPTR p, int eruleno, NODEPTR kids[]) {
  burm_assert(p, PANIC("NULL tree in burm_kids\n"));
  burm_assert(kids, PANIC("NULL kids in burm_kids\n"));
  switch (eruleno) {
  case 45: /* args: BRG_ARGS(BRG_UNDEF,BRG_UNDEF) */
  case 8: /* stmt: BRG_RET(BRG_UNDEF) */
  case 3: /* mem: BRG_ALLOCA */
  case 2: /* reg: BRG_REG */
  case 1: /* imm: BRG_CONST */
  case 0: /* label: BRG_LABEL */
    break;
  case 24: /* stmt: BRG_BR(label,BRG_UNDEF,BRG_UNDEF) */
  case 9: /* stmt: BRG_RET(imm) */
  case 7: /* stmt: BRG_RET(reg) */
  case 5: /* reg: BRG_LOAD(reg) */
  case 4: /* reg: BRG_LOAD(mem) */
    kids[0] = burm_child(p,0);
    break;
  case 44: /* arg: label */
  case 43: /* arg: mem */
  case 42: /* arg: imm */
  case 41: /* arg: reg */
  case 6: /* stmt: reg */
    kids[0] = p;
    break;
  case 49: /* reg: BRG_CALL(mem,args) */
  case 48: /* reg: BRG_CALL(reg,args) */
  case 47: /* reg: BRG_CALL(label,args) */
  case 46: /* args: BRG_ARGS(arg,args) */
  case 40: /* reg: BRG_SDIV(imm,imm) */
  case 39: /* reg: BRG_SDIV(imm,reg) */
  case 38: /* reg: BRG_SDIV(reg,imm) */
  case 37: /* reg: BRG_SDIV(reg,reg) */
  case 36: /* reg: BRG_MUL(imm,imm) */
  case 35: /* reg: BRG_MUL(imm,reg) */
  case 34: /* reg: BRG_MUL(reg,imm) */
  case 33: /* reg: BRG_MUL(reg,reg) */
  case 32: /* reg: BRG_SUB(imm,imm) */
  case 31: /* reg: BRG_SUB(imm,reg) */
  case 30: /* reg: BRG_SUB(reg,imm) */
  case 29: /* reg: BRG_SUB(reg,reg) */
  case 28: /* reg: BRG_ADD(imm,imm) */
  case 27: /* reg: BRG_ADD(imm,reg) */
  case 26: /* reg: BRG_ADD(reg,imm) */
  case 25: /* reg: BRG_ADD(reg,reg) */
  case 22: /* cond: BRG_ICMP(imm,reg) */
  case 21: /* cond: BRG_ICMP(reg,imm) */
  case 20: /* cond: BRG_ICMP(reg,reg) */
  case 19: /* mem: BRG_GETELEMENTPTR(reg,reg) */
  case 18: /* mem: BRG_GETELEMENTPTR(reg,imm) */
  case 17: /* mem: BRG_GETELEMENTPTR(mem,reg) */
  case 16: /* mem: BRG_GETELEMENTPTR(mem,imm) */
  case 15: /* stmt: BRG_STORE(label,mem) */
  case 14: /* stmt: BRG_STORE(mem,mem) */
  case 13: /* stmt: BRG_STORE(imm,mem) */
  case 12: /* stmt: BRG_STORE(imm,reg) */
  case 11: /* stmt: BRG_STORE(reg,reg) */
  case 10: /* stmt: BRG_STORE(reg,mem) */
    kids[0] = burm_child(p,0);
    kids[1] = burm_child(p,1);
    break;
  case 23: /* stmt: BRG_BR(cond,label,label) */
    kids[0] = burm_child(p,0);
    kids[1] = burm_child(p,1);
    kids[2] = burm_child(p,2);
    break;
  default:
    burm_assert(0, PANIC("Bad external rule number %d in burm_kids\n", eruleno));
  }
  return kids;
}

void dumpCover(NODEPTR p, int goalnt, int indent)
{
  int eruleno = burm_rule(STATE_LABEL(p), goalnt);
  short *nts = burm_nts[eruleno];
  NODEPTR kids[10];
  int i;

  std::cerr << "\t\t";
  for (i = 0; i < indent; i++)
    std::cerr << " ";
  std::cerr << burm_string[eruleno] << "\n";
  burm_kids(p, eruleno, kids);
  for (i = 0; nts[i]; i++)
    dumpCover(kids[i], nts[i], indent + 1);
}


#pragma GCC diagnostic pop



/* burm_trace - print trace message for matching p */
static void burm_trace(NODEPTR p, int eruleno, COST cost) { }

static void gen(NODEPTR p, remniw::AsmBuilder *Builder) {
    if (burm_label(p) == 0) {
        LLVM_DEBUG(llvm::outs() << "gen: " << "no cover\n";);
    } else {
        stmt_action(p->getState(), Builder);
    }
    burm_free(p->getState());
}


static void printDebugTree(BrgTreeNode *p, int indent=0) {
    if (!p)
        return;
    LLVM_DEBUG({
        if (indent) llvm::outs() << "|";
        int i = 0;
        for (; i < indent - 4; ++i) llvm::outs() << " ";
        if (indent - 4 > 0) llvm::outs() << "|";
        for (; i < indent; ++i) llvm::outs() << "-";
        llvm::outs() << "+ op:" << p->getOp() << ", Kind:" << p->getNodeKindString() << "\n";
        for(auto *kid: p->getKidsVector())
            printDebugTree(kid, (indent+4));
    });
}

namespace remniw {

void AsmBuilder::buildAsmFunction(const BrgFunction *BrgFunc) {
    CurrentFunction = new AsmFunction(BrgFunc->FuncName, BrgFunc->StackSizeInBytes);
    AsmFunctions.push_back(CurrentFunction);

    for (auto* RootNode: BrgFunc->Insts) {
        printDebugTree(RootNode);
        // FIXME: Label Instruction
        if (RootNode->getOp() == BrgTerm::Label)
            handleLABEL(AsmOperand::createLabel(RootNode->getLabel()));
        gen(RootNode, this);
    }
}

}  // namespace remniw
