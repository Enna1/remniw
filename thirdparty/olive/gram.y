%{
/*
   FILE: gram.y
  
   Copyright (c) 1997 Princeton University

   All rights reserved.

   This software is to be used for non-commercial purposes only,
   unless authorized permission to do otherwise is obtained.  
   For more information, contact spam@ee.princeton.edu
*/

#include <stdio.h>
#include <assert.h>
#include "x_array.h"
#include "code.h"
#include "tree.h"
#include "iburg.h"

static char rcsid[] = "$Id: gram.y,v 1.8 1993/06/14 00:16:39 tjiang Exp $";

int rule_start_line;
int number_of_terminals;
int yylex(void);

%}
%union {
  int n;
  char *string;
  Tree tree;
  Code *code;
  Tree_list *tlist;
}
%term TERMINAL
%term START
%term PPERCENT
%term DECLARE
%term UNION

%token  <string>        ID
%token  <n>             INT
%type   <string>        lhs
%type   <string>        rule_header
%type   <tree>          tree
%token  <code>          CODE
%token  <code>          TYPE
%type   <tlist>         tlist
%%
spec  : decls PPERCENT rules
  | decls
  ;

decls : /* lambda */
  | decls decl
  ;

decl  : TERMINAL  blist
  | DECLARE TYPE lhs TYPE ';'
    {
      prototype($3,$2,$4);
    }
  | DECLARE TYPE lhs ';'
    {
      prototype($3,$2,0);
    }
  | START lhs
    {
    if (nonterm($2)->number != 1)
      yyerror("redeclaration of the start symbol\n");
    }
  | UNION CODE ';'
    {
      set_values($2);
    }
  | error   { yyerrok; }
  ;

blist : /* lambda */
  | blist ID '=' INT  { term($2, number_of_terminals++); }
  | blist ID          { term($2, number_of_terminals++); }
  ;

rules : rules rule
  | rule
  ;

rule_header:
  lhs { rule_start_line=yylineno; }
  ;

rule  : rule_header ':' tree CODE '=' CODE ';'
    { rule(rule_start_line,$1,$3,$4,$6);}
  | rule_header ':' tree '=' CODE ';'
    { rule(rule_start_line,$1,$3,0,$5);}
  | error ';'
    { yyerror("unrecognizable rule"); }
  ;

lhs   : ID  { nonterm($$ = $1); }
  ;

tree  : ID {
    $$ = tree($1, 0);
  }
  | ID '(' tlist ')' {
    $$ = tree($1,$3);
    Tree_list_destroy($3);
  };

tlist :  tree {
    Tree_list *tl=Tree_list_create(2);
    Tree_list_extend(tl,$1);
    $$ = tl;
  }
  | tlist ',' tree {
    Tree_list_extend($1,$3);
    $$ = $1;
  }
  ;

%%
#include "centerline.h"
#include <ctype.h>
#include <string.h>

int errcnt = 0;
FILE *infp = NULL, *outfp = NULL, *deffp = NULL;
static char buf[BUFSIZ], *bp = buf;
int yylineno = 0;
static int ppercent = 0;

int get(void) {
    if (*bp == 0) {
        if (fgets(buf, sizeof buf, infp) == NULL)
            return EOF;
        bp = buf;
        yylineno++;
        while (buf[0] == '%' && buf[1] == '{' && buf[2] == '\n') {
            /*if(!Lflag)*/
            /*fprintf(deffp,"# line %d \"%s\"\n", yylineno,in_file_name);*/
            for (;;) {
                if (fgets(buf, sizeof buf, infp) == NULL) {
                    yywarn("unterminated %{...%}\n");
                    return EOF;
                }
                yylineno++;
                if (strcmp(buf, "%}\n") == 0)
                    break;
                fputs(buf, deffp);
            }
            if (fgets(buf, sizeof buf, infp) == NULL)
                return EOF;
            yylineno++;
        }
    }
    return *bp++;
}

void yyerror(char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    if (yylineno > 0)
        fprintf(stderr, "\"%s\", line %d: ", in_file_name, yylineno);
    vfprintf(stderr, fmt, ap);
    if (fmt[strlen(fmt) - 1] != '\n')
        fprintf(stderr, "\n");
    errcnt++;
}

void yyerror0(int fn, int lineno, char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    if (lineno > 0)
        fprintf(stderr, "\"%s\", line %d: ", fn >= 0 ? in_file_names[fn] : in_file_name,
                lineno);
    vfprintf(stderr, fmt, ap);
    if (fmt[strlen(fmt) - 1] != '\n')
        fprintf(stderr, "\n");
    errcnt++;
}

static Code *current_code;

#define SAVE(c) Code_append(current_code, c)

static void scan_quoted(char delim) {
    int startline = yylineno;
    int c;
    SAVE(delim);
    while ((c = get()) != EOF) {
        SAVE(c);
        switch (c) {
        case '\\': SAVE(get()); break;
        default:
            if (c == delim)
                return;
        }
    }
    yyerror("unterminated %c on line %d", delim, startline);
}

static void scan_comment(void) {
    int startline = yylineno;
    int c;
    while ((c = get()) != EOF) {
        SAVE(c);
        if (c == '*') {
            c = get();
            SAVE(c);
            if (c == '/')
                return;
        }
    }
    yyerror("unterminated comment on line %d", startline);
}

static void scan_till_brace(void) {
    int startline = yylineno;
    int c;
    while ((c = get()) != EOF)
        switch (c) {
        case '}': return;
        case '{':
            SAVE(c);
            scan_till_brace();
            SAVE('}');
            break;
        case '/':
            SAVE(c);
            c = get();
            SAVE(c);
            if (c == '*')
                scan_comment();
            break;
        case '\'':
        case '"': scan_quoted(c); break;
        default: SAVE(c); break;
        }
    yyerror("{ on line %d has no closing }", startline);
}

static void scan_code(void) {
    assert(current_code == NULL);
    current_code = Code_create(100, current_in_file, yylineno);
    scan_till_brace();
}

static void scan_type(void) {
    int startline = yylineno;
    int c;

    assert(current_code == 0);
    current_code = Code_create(10, current_in_file, yylineno);

    while ((c = get()) != EOF)
        switch (c) {
        case '>': return;
        default: SAVE(c); break;
        }

    yyerror("< on line %d has no closing >", startline);
}

#undef SAVE

int yylex(void) {
    int c;

    while ((c = get()) != EOF) {
        switch (c) {
        case '#':
            /* a comment */
            do {
                c = get();
            } while (c != EOF && c != '\n');
            continue;
        case ' ':
        case '\f':
        case '\t':
        case '\n': continue;
        case '(':
        case ')':
        case ',':
        case ';':
        case '=':
        case ':': return c;
        case '_':
            yylval.string = alloc(2);
            yylval.string[0] = '_';
            yylval.string[1] = 0;
            return ID;
        case '<':
            scan_type();
            yylval.code = current_code;
            current_code = 0;
            return TYPE;
        case '{':
            scan_code();
            yylval.code = current_code;
            current_code = 0;
            return CODE;
        }
        if (c == '%' && *bp == '%') {
            bp++;
            return ppercent++ ? 0 : PPERCENT;
        } else if (c == '%' && strncmp(bp, "term", 4) == 0 && isspace(bp[4])) {
            bp += 4;
            return TERMINAL;
        } else if (c == '%' && strncmp(bp, "start", 5) == 0 && isspace(bp[5])) {
            bp += 5;
            return START;
        } else if (c == '%' && strncmp(bp, "union", 5) == 0 && isspace(bp[5])) {
            bp += 5;
            return UNION;
        } else if (c == '%' && strncmp(bp, "declare", 7) == 0 &&
                   (isspace(bp[7]) || bp[7] == '<')) {
            bp += 7;
            return DECLARE;
        } else if (isdigit(c)) {
            int n = 0;
            do {
                n = 10 * n + (c - '0');
                c = get();
            } while (isdigit(c));
            if (n > 32767)
                yyerror("integer %d greater than 32767\n", n);
            bp--;
            yylval.n = n;
            return INT;
        } else if (isalpha(c)) {
            char *p = bp - 1;
            while (isalpha(c) || isdigit(c) || c == '_')
                c = get();
            bp--;
            yylval.string = alloc(bp - p + 1);
            strncpy(yylval.string, p, bp - p);
            yylval.string[bp - p] = 0;
            return ID;
        } else if (isprint(c))
            yyerror("illegal character `%c'\n", c);
        else
            yyerror("illegal character `\0%o'\n", c);
    }
    /* automatically chain to the next file */
    infp = open_next_file(infp);
    if (infp == NULL)
        return 0;
    else
        return yylex();
}

void yywarn(char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    if (yylineno > 0)
        fprintf(stderr, "line %d: ", yylineno);
    fprintf(stderr, "warning: ");
    vfprintf(stderr, fmt, ap);
}
