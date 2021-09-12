/*
   FILE: iburg.h

   Copyright (c) 1997 Princeton University

   All rights reserved.

   This software is to be used for non-commercial purposes only,
   unless authorized permission to do otherwise is obtained.
   For more information, contact spam@ee.princeton.edu
*/

#ifndef BURG_INCLUDED
#define BURG_INCLUDED

#define SYLIAO

/* $Id: iburg.h,v 1.9 1993/07/01 10:44:57 tjiang Exp $ */
/* iburg.c: */
extern void *alloc(int nbytes);

typedef enum
{
    TERM = 1,
    NONTERM
} Kind;
typedef struct rule *Rule;
typedef struct term *Term;
struct term {          /* terminals: */
    char *name;        /* terminal name */
    Kind kind;         /* TERM */
    int esn;           /* external symbol number */
    int arity;         /* operator arity */
    Term link;         /* next terminal in esn order */
    Rule rules;        /* rules whose pattern starts with term */
    int root_of_immed; /* 1 iff terminal appears as root of immed rule */
};

typedef struct nonterm *Nonterm;
struct nonterm {          /* non-terminals: */
    char *name;           /* non-terminal name */
    Kind kind;            /* NONTERM */
    int number;           /* identifying number */
    int lhscount;         /* # times nt appears in a rule lhs */
    int reached;          /* 1 iff reached from start non-terminal */
    Rule rules;           /* rules w/non-terminal on lhs */
    Rule chain;           /* chain rules w/non-terminal on rhs */
    Nonterm link;         /* next terminal in number order */
    Code *return_type;    /* return type for actions */
    Code *argument_types; /* argument type for actions */
    Type *argty;          /* type of arguments */
    Type *retty;          /* type of return value */
};

extern void prototype(char *id, Code *retval, Code *args);
extern Nonterm nonterm(char *id);
extern Term term(char *id, int esn);

struct rule {          /* rules: */
    int line_number;   /* line number on which this rule defined */
    int emitted;       /* set to one if already emitted,
                  used to coalesce matches */
    Nonterm lhs;       /* lefthand side non-terminal */
    Tree pattern;      /* rule pattern */
    int ern;           /* external rule number */
    int packed;        /* packed external rule number */
    int is_immed;      /* is an immediate rule */
    Rule link;         /* next rule in ern order */
    Rule next;         /* next rule with same pattern root */
    Rule chain;        /* next chain rule with same rhs */
    Rule decode;       /* next rule with same lhs */
    Rule kids;         /* next rule with same burm_kids pattern */
    Code *cost_code;   /* the C code to compute the cost */
    Code *action_code; /* the C code to perform actions */
};
/* create a rule */
extern Rule rule(int ln, char *id, Tree pattern, Code *cost_code, Code *action_code);

/* gram.y: */
extern int yylineno;
extern void yyerror0(int fn, int ln, char *fmt, ...);
extern int get();
void yyerror(char *fmt, ...);
int yyparse(void);
void yywarn(char *fmt, ...);
extern int errcnt;
extern FILE *infp, *outfp, *deffp, *this_fp;
extern int Lflag;

/* globals */
extern char *in_file_name;
void Code_print(Code *, Rule t, char *);
void print(char *fmt, ...);
void ckreach(Nonterm p);
void *lookup(char *name);
void set_values(Code *u);
void mark_line(int, char *);

/* the input file list */
#define N_IN_FILES 20
extern int n_in_files, current_in_file;
extern char *in_file_names[/*N_IN_FILES*/];
extern FILE *open_next_file(FILE *);

#endif
