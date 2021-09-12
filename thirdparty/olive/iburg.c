/*
   FILE: iburg.c

   Copyright (c) 1997 Princeton University

   All rights reserved.

   This software is to be used for non-commercial purposes only,
   unless authorized permission to do otherwise is obtained.
   For more information, contact spam@ee.princeton.edu
*/

#include <assert.h>
#include "centerline.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "x_array.h"
#include "code.h"
#include "tree.h"
#include "iburg.h"

static char rcsid[] = "$Id: iburg.c,v 1.20 1993/07/21 11:39:27 tjiang Exp $";
static char *prefix = "burm";
int Lflag = 1;
static int Iflag = 1, Tflag = 1;
static int ntnumber = 0;
static Nonterm start = 0;
static Term terms;
static Nonterm nts;
static Code *sem_values;
static Rule rules;
static int nrules;

static char *stringf(char *fmt, ...);
static void emitclosure(Nonterm nts);
static void emitcost(Tree t, char *v);
static void emit_nt_defs(Nonterm nts, int ntnumber);
static void emitdefs(Nonterm nts, int ntnumber);
static void emitfuncs(void);
static void emitheader(void);
static void emitkids(Rule rules, int nrules);
static void emitlabel(Nonterm start);
static void emitnts(Rule rules, int nrules);
static void emit_record(char *pre, Rule r);
static void emit_record_immediate(char *pre, Rule r);
static void emitrule(Nonterm nts);
static void emitstate(Term terms, Nonterm start, int ntnumber);
static void emitstring(Rule rules);
static void emit_numbers(Rule rules);
static void emitstruct(Nonterm nts, int ntnumber);
static void emitterms(Term terms);
static void emittest(Tree t, char *v, char *suffix);
static void emit_action_struct();
static void emit_cost_code(Rule rules, int nrules);
static void emit_action_code_headers();
static void emit_action_code();
static void emit_exec_code();
static void emit_dump_code();

int current_in_file, n_in_files;
char *in_file_names[N_IN_FILES];
char *in_file_name;
static char *in_file_base;
static char *out_file_name;
static char *def_file_name;
FILE *this_fp;

static void set_file(FILE *fp) {
    this_fp = fp;
}

int main(int argc, char *argv[]) {
    int c, i;
    Nonterm p;
    char *cp;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-I") == 0)
            Iflag = 1;
        else if (strcmp(argv[i], "-L") == 0)
            Lflag = 0;
        else if (strcmp(argv[i], "-T") == 0)
            Tflag = 1;
        else if (strncmp(argv[i], "-p", 2) == 0 && argv[i][2])
            prefix = &argv[i][2];
        else if (strncmp(argv[i], "-p", 2) == 0 && i + 1 < argc)
            prefix = argv[++i];
        else if (strcmp(argv[i], "-o") == 0) {
            /* -o outfile */
            i = i + 1;
            outfp = fopen(argv[i], "w");
            if (outfp == NULL) {
                yyerror("%s: can't write `%s'\n", argv[0], argv[i]);
                exit(1);
            }
        } else if (*argv[i] == '-' && argv[i][1]) {
            yyerror("usage: %s [-T | -I | -p prefix]... [ [ input ] output \n", argv[0]);
            exit(1);
        } else {
            /* collect input files */
            in_file_names[n_in_files++] = argv[i];
        }
    }

    /* open input files */
    current_in_file = -1;
    if (n_in_files == 0 || strcmp(in_file_names[0], "-") == 0)
        infp = stdin;
    else
        infp = open_next_file(NULL);

    if (outfp == NULL) {
        if (in_file_name == 0)
            in_file_name = "no_name.twg";
        cp = strchr(in_file_name, '/');
        if (cp == 0 || cp == in_file_name)
            cp = in_file_name;
        else
            cp += 1;
        i = strlen(cp) + 1;
        in_file_base = (char *)malloc(i);
        out_file_name = (char *)malloc(i + 4);
        def_file_name = (char *)malloc(i + 4);
        strcpy(in_file_base, cp);
        cp = strrchr(in_file_base, '.');
        if (cp)
            *cp = 0;
        sprintf(out_file_name, "%s.cpp", in_file_base);
        sprintf(def_file_name, "%s.h", in_file_base);
        outfp = fopen(out_file_name, "w");
        if (outfp == NULL) {
            yyerror("%s: can't write `%s'\n", argv[0], out_file_name);
            exit(1);
        }
        deffp = fopen(def_file_name, "w");
        if (deffp == NULL) {
            yyerror("%s: can't write `%s'\n", argv[0], def_file_name);
            exit(1);
        }
    }
    // set_file(outfp);
    /* create the definitions file */
    set_file(deffp);
    print("#ifndef __OLIVE_HEADER_INCLUDED__\n");
    print("#define __OLIVE_HEADER_INCLUDED__\n");
    print("#include <assert.h>\n");
    print("#include <iostream>\n");
    print("#include <stdlib.h>\n");
    print("#include <string>\n");
    yyparse();
    if (start)
        ckreach(start);
    emitstruct(nts, ntnumber);
    // emitstruct(nts, ntnumber);

    // print("#define __OLIVE_HEADER_INCLUDED__\n");
    set_file(outfp);
    print("#include \"%s\"\n", def_file_name);
    emit_nt_defs(nts, ntnumber);
    emit_action_code_headers();

    /* this should be done during parsing */
    for (p = nts; p; p = p->link) {
        p->argty = Type_get_arguments(p->argument_types);
        p->retty = Type_get_return(p->return_type);
    }

    emitheader();
    emitdefs(nts, ntnumber);
    emitnts(rules, nrules);
    emitterms(terms);
    if (Iflag) {
        emitstring(rules);
        emit_numbers(rules);
    }

    print("\n#pragma GCC diagnostic push\n");
    print("#pragma GCC diagnostic ignored \"-Wunused-variable\"\n\n");

    emitrule(nts);

    emit_action_struct();
    emit_cost_code(rules, nrules);
    emit_action_code_headers();
    emit_exec_code();
    emit_action_code();
    emitclosure(nts);
    for (p = nts; p; p = p->link)
        if (!p->reached)
            yyerror("can't reach non-terminal `%s'\n", p->name);
    if (start)
        emitstate(terms, start, ntnumber);

    // print("#ifdef STATE_LABEL\n");

    if (start) {
        emitlabel(start);
        set_file(deffp);
        print("\nstruct %Pstate *%Plabel(NODEPTR);\n");
        print("struct %Pstate *%Plabel1(NODEPTR);\n\n");
        print("void dumpCover(NODEPTR,int,int);\n\n");
        print("#endif\n");
        set_file(outfp);
    } else {
        set_file(deffp);
        print("#endif\n");
        set_file(outfp);
    }
    emitfuncs();
    emitkids(rules, nrules);
    emit_dump_code();

    print("\n#pragma GCC diagnostic pop\n\n");

    // print("#endif\n");

    mark_line(yylineno, in_file_name);
    if (!feof(infp))
        while ((c = get()) != EOF)
            putc(c, outfp);
    return errcnt > 0;
}

/* alloc - allocate nbytes or issue fatal error */
void *alloc(int nbytes) {
    void *p = calloc(1, nbytes);

    if (p == NULL) {
        yyerror("out of memory\n");
        exit(1);
    }
    return p;
}

/* stringf - format and save a string */
static char *stringf(char *fmt, ...) {
    va_list ap;
    char *s, buf[512];

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    return strcpy(alloc(strlen(buf) + 1), buf);
}

struct entry {
    union {
        char *name;
        struct term t;
        struct nonterm nt;
    } sym;
    struct entry *link;
} * table[211];
#define HASHSIZE (sizeof table / sizeof table[0])

/* hash - return hash number for str */
static unsigned hash(char *str) {
    unsigned h = 0;

    while (*str)
        h = (h << 1) + *str++;
    return h;
}

/* lookup - lookup symbol name */
void *lookup(char *name) {
    struct entry *p = table[hash(name) % HASHSIZE];

    for (; p; p = p->link)
        if (strcmp(name, p->sym.name) == 0)
            return &p->sym;
    return 0;
}

/* install - install symbol name */
static void *install(char *name) {
    struct entry *p = alloc(sizeof *p);
    int i = hash(name) % HASHSIZE;

    p->sym.name = name;
    p->link = table[i];
    table[i] = p;
    return &p->sym;
}

/* nonterm - create a new terminal id, if necessary */
Nonterm nonterm(char *id) {
    Nonterm p = lookup(id), *q = &nts;

    if (p && p->kind == NONTERM)
        return p;
    if (p && p->kind == TERM)
        yyerror("`%s' is a terminal\n", id);
    p = install(id);
    p->kind = NONTERM;
    p->number = ++ntnumber;
    p->return_type = 0;
    p->argument_types = 0;
    if (p->number == 1)
        start = p;
    while (*q && (*q)->number < p->number)
        q = &(*q)->link;
    assert(*q == 0 || (*q)->number != p->number);
    p->link = *q;
    *q = p;
    return p;
}

/* set the prototype for a nonterm */
void prototype(char *id, Code *retval, Code *args) {
    Nonterm p = nonterm(id);
    if (p->return_type || p->argument_types)
        yyerror("redeclaration of %s\n", id);
    else {
        assert(retval);
        assert(p->return_type == 0 && p->argument_types == 0);
        p->return_type = retval;
        p->argument_types = args;
    }
}

/* term - create a new terminal id with external symbol number esn */
Term term(char *id, int esn) {
    Term p = lookup(id), *q = &terms;

    if (p)
        yyerror("redefinition of terminal `%s'\n", id);
    else
        p = install(id);
    p->kind = TERM;
    p->esn = esn;
    p->arity = -1;
    p->root_of_immed = 0;
    while (*q && (*q)->esn < p->esn)
        q = &(*q)->link;
    if (*q && (*q)->esn == p->esn)
        yyerror("duplicate external symbol number `%s=%d'\n", p->name, p->esn);
    p->link = *q;
    *q = p;
    return p;
}

/* tree - create & initialize a tree node with the given fields */
Tree tree(char *id, Tree_list *tl) {
    Tree t = alloc(sizeof *t);
    Term p = lookup(id);
    int i;
    int arity = 0;

    arity = tl ? Tree_list_ub(tl) : 0;
    if (arity > MAX_KIDS) {
        yyerror("too many children for terminal `%d'\n", id);
        arity = MAX_KIDS;
    }
    if (p == NULL && arity > 0) {
        yyerror("undefined terminal `%s'\n", id);
        p = term(id, -1);
    } else if (p == NULL && arity == 0)
        p = (Term)nonterm(id);
    else if (p && p->kind == NONTERM && arity > 0) {
        yyerror("`%s' is a non-terminal\n", id);
        p = term(id, -1);
    }
    if (p->kind == TERM && p->arity == -1)
        p->arity = arity;
    if (p->kind == TERM && arity != p->arity)
        yyerror("inconsistent arity for terminal `%s'\n", id);
    t->op = p;
    t->nkids = arity;
    for (i = 0; i < arity; i++)
        t->kids[i] = Tree_list_fetch(tl, i);
    return t;
}

int count_underscores(Tree pattern) {
    int i, n;
    Term q;
    n = 0;
    for (i = 0; i < pattern->nkids; i++) {
        q = pattern->kids[i]->op;
        if (q->kind == NONTERM)
            n += count_underscores(pattern->kids[i]);
        else if (q->name[0] == '_')
            n += 1;
    }
    return n;
}

/* rule - create & initialize a rule with the given fields */
Rule rule(int ln, char *id, Tree pattern, Code *cc, Code *ac) {
    Rule r = alloc(sizeof *r), *q;
    Term p = pattern->op, qq;
    int i;

    r->line_number = ln;
    r->lhs = nonterm(id);
    r->emitted = 0;
    r->packed = ++r->lhs->lhscount;
    for (q = &r->lhs->rules; *q; q = &(*q)->decode)
        ;
    *q = r;
    r->pattern = pattern;
    if (pattern->nkids) {
        r->is_immed = 1;
        for (i = 0; i < pattern->nkids; i++) {
            qq = pattern->kids[i]->op;
            if (qq->name[0] != '_') {
                r->is_immed = 0;
                break;
            }
        }
    } else
        r->is_immed = 0;
    r->ern = nrules++;
    r->cost_code = cc;
    r->action_code = ac;
    if (p->kind == TERM) {
        r->next = p->rules;
        p->rules = r;
        p->root_of_immed |= r->is_immed;
    } else if (pattern->nkids == 0) {
        Nonterm p = pattern->op;
        r->chain = p->chain;
        p->chain = r;
    }
    for (q = &rules; *q && (*q)->ern < r->ern; q = &(*q)->link)
        ;
    if (*q && (*q)->ern == r->ern)
        yyerror("duplicate external rule number `%d'\n", r->ern);
    r->link = *q;
    *q = r;
    return r;
}

/* print - formatted output */
void print(char *fmt, ...) {
    int i;
    va_list ap;

    va_start(ap, fmt);
    for (; *fmt; fmt++)
        if (*fmt == '%')
            switch (*++fmt) {
            case 'x': fprintf(this_fp, "%x", va_arg(ap, int)); break;
            case 'd': fprintf(this_fp, "%d", va_arg(ap, int)); break;
            case 's': fputs(va_arg(ap, char *), this_fp); break;
            case 'P': fprintf(this_fp, "%s_", prefix); break;
            case 'T': {
                Tree t = va_arg(ap, Tree);
                print("%S", t->op);

                if (t->nkids) {
                    putc('(', this_fp);
                    for (i = 0; i + 1 < t->nkids; i++)
                        print("%T,", t->kids[i]);
                    print("%T)", t->kids[i]);
                }
                break;
            }
            case 'R': {
                Rule r = va_arg(ap, Rule);
                print("%S: %T", r->lhs, r->pattern);
                break;
            }
            case 'S': fputs(va_arg(ap, Term)->name, this_fp); break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5': {
                int n = *fmt - '0';
                while (n-- > 0) {
                    putc(' ', this_fp);
                    putc(' ', this_fp);
                }
                break;
            }
            default: putc(*fmt, this_fp); break;
            }
        else
            putc(*fmt, this_fp);
    va_end(ap);
}

/* reach - mark all non-terminals in tree t as reachable */
static void reach(Tree t, int level) {
    int i;
    Nonterm p = t->op;

    if (p->kind == NONTERM) {
        if (!p->reached)
            ckreach(p);
    } else if (level && ((Term)p)->root_of_immed) {
        /*
          check that a internal node is not the root of an immediate rule,
          a situation that the matcher cannot handle.
          */
        yyerror("%s appears as internal node and as root of immed rule\n", p->name);
    }

    level++;
    for (i = 0; i < t->nkids; i++)
        reach(t->kids[i], level);
}

/* ckreach - mark all non-terminals reachable from p */
void ckreach(Nonterm p) {
    Rule r;

    p->reached = 1;
    for (r = p->rules; r; r = r->decode)
        reach(r->pattern, 0);
}

static void emitcase(Term p, int ntnumber) {
    Rule r;
    int i;
    int non_immed = 0;
    int immed = 0;
    char buf[100];

    print("%1case %d:   /* %S */\n", p->esn, p);
    if (p->arity == 0) {
        /* a leaf node */
        print("#ifdef LEAF_TRAP\n"
              "%2if(s=LEAF_TRAP(u,op))\n"
              "%3return s;\n"
              "#endif\n"
              "%2s=%Palloc_state(u,op,arity);\n"
              "%2SET_STATE(u,s);\n"
              "%2k=0;\n");
        for (r = p->rules; r; r = r->next) {
            assert(r->is_immed == 0);
            /* emit the test */
            print("%2{%1    /* %R */\n", r);
            /* emit cost and record match */
            emit_record("      ", r);
            print("%2}\n");
        }
        print("%2break;\n");
    } else {
        print("%2s=%Palloc_state(u,op,arity);\n"
              "%2SET_STATE(u,s);\n"
              "%2k=s->kids;\n");
        /*
          Generate code for immediate rules.
          Also count the number of immediate and non_immediate rules
          */
        for (r = p->rules; r; r = r->next) {
            if (r->is_immed == 0) {
                non_immed++;
                continue;
            }
            print("%4/*immediate rule: %R */\n", r);
            emit_record_immediate("    ", r);
            immed++;
        }

        if (immed > 0)
            print("%2if(immed_matched){\n"
                  "%3for(i=0;i<arity;i++)k[i]=0;\n"
                  "%3return s;\n"
                  "%2}\n");

        /*
          We need to label the children if there are non_immediate
          rules and if this terminal symbol isn't the root of any
          rule.  In the later case, the symbol may still be nested
          within other rules.
          */
        if (non_immed > 0 || p->rules == 0)
            print("%2children=GET_KIDS(u);\n"
                  "%2for(i=0;i<arity;i++)\n"
                  "%3k[i]=%Plabel1(children[i]);\n");

        for (r = p->rules; r; r = r->next) {
            if (r->is_immed)
                continue;
            /* emit the test */
            print("%2if (%1 /* %R */\n", r);
            for (i = 0; i + 1 < p->arity; i++) {
                sprintf(buf, "k[%d]", i);
                emittest(r->pattern->kids[i], buf, " && ");
            }
            sprintf(buf, "k[%d]", i);
            emittest(r->pattern->kids[i], buf, "");
            print("%2) {\n");
            /* emit cost and record match */
            emit_record("      ", r);
            print("%2}\n");
        }
        print("%2break;\n");
    }
}

/* emitclosure - emit the closure functions */
static void emitclosure(Nonterm nts) {
    Nonterm p;

    for (p = nts; p; p = p->link)
        if (p->chain)
            print("static void %Pclosure_%S(struct %Pstate *, COST);\n", p);
    print("\n");
    for (p = nts; p; p = p->link)
        if (p->chain) {
            Rule r;
            print("static void %Pclosure_%S(struct %Pstate *s, COST c) {\n", p);
            for (r = p->chain; r; r = r->chain)
                emit_record("  ", r);
            print("}\n\n");
        }
}

/* emitcost - emit cost computation for tree t */
static void emitcost(Tree t, char *v) {
    Nonterm p = t->op;
    int i;

    /* Steve -- Is this function really necessary? */
    if (p->kind == TERM) {
        for (i = 0; i < t->nkids; i++)
            emitcost(t->kids[i], stringf("%s->kids[%d]", v, i));
    } else
        print("%s->cost[%P%S_NT] + ", v, p);
}

/* emitdefs - emit non-terminal defines and data structures */
static void emit_nt_defs(Nonterm nts, int ntnumber) {
    Nonterm p;
    for (p = nts; p; p = p->link)
        print("#define %P%S_NT %d\n", p, p->number);
    print("extern int %Pmax_nt;\n");
}

static void emitdefs(Nonterm nts, int ntnumber) {
    Nonterm p;

    emit_nt_defs(nts, ntnumber);
    print("int %Pmax_nt = %d;\n\n", ntnumber);
    if (Iflag) {
        print("std::string %Pntname[] = {\n%1\"\",\n");
        for (p = nts; p; p = p->link)
            print("%1\"%S\",\n", p);
        print("%1\"\"\n};\n\n");
    }
}

/* emitfuncs - emit functions to access node fields */
static void emitfuncs(void) {
    print("void %Pfree(struct %Pstate *s)\n{\n"
          "%1int i,arity=%Parity[s->op];\n"
          "%1if(s->kids==0)\n"
          "%2free(s);\n"
          "%1else {\n"
          "%2for(i=0;i<arity;i++)\n"
          "%3%Pfree(s->kids[i]);\n"
          "%2free(s->kids);free(s);\n"
          "%1}\n"
          "}\n");
    print("struct %Pstate *%Pimmed(struct %Pstate *s,int n)\n"
          "{\n"
          "%1NODEPTR *children = GET_KIDS(s->node);\n"
          "%1if(s->kids[n])\n"
          "%2return s->kids[n];\n"
          "%1else\n"
          "%1return s->kids[n]=burm_label1(children[n]);\n"
          "}\n");
    print("int %Pop_label(NODEPTR p) {\n"
          "%1%Passert(p, PANIC(\"NULL tree in %Pop_label\\n\"));\n"
          "%1return OP_LABEL(p);\n}\n\n");
    print("struct %Pstate *%Pstate_label(NODEPTR p) {\n"
          "%1%Passert(p, PANIC(\"NULL tree in %Pstate_label\\n\"));\n"
          "%1return STATE_LABEL(p);\n}\n\n");
    print("NODEPTR %Pchild(NODEPTR p, int index) {\n"
          "%1NODEPTR *kids;\n"
          "%1%Passert(p, PANIC(\"NULL tree in %Pchild\\n\"));\n"
          "%1kids=GET_KIDS(p);\n"
          "%1%Passert((0<=index && index<%Parity[OP_LABEL(p)]),\n"
          "%2PANIC(\"Bad index %%d in %Pchild\\n\", index));\n\n"
          "%1return kids[index];\n"
          "}\n");
}

/* emitheader - emit initial definitions */
static void emitheader(void) {
    print(
        "#ifndef ALLOC\n#define ALLOC(n) malloc(n)\n#endif\n\n"
        "#ifndef %Passert\n#define %Passert(x,y) if (!(x)) {  y; abort(); }\n#endif\n\n");
    if (Tflag)
        print("static NODEPTR %Pnp;\n\n");
}

/* computekids - compute paths to kids in tree t */
static char *computekids(Tree t, char *v, char *bp, int *ip) {
    Term p = t->op;
    int i;

    if (p->kind == NONTERM) {
        sprintf(bp, "    kids[%d] = %s;\n", (*ip)++, v);
        bp += strlen(bp);
    } else if (p->arity > 0) {
        assert(p->arity == t->nkids);
        for (i = 0; i < t->nkids; i++)
            bp =
                computekids(t->kids[i], stringf("%s_child(%s,%d)", prefix, v, i), bp, ip);
    }
    return bp;
}

/* emitkids - emit burm_kids */
static void emitkids(Rule rules, int nrules) {
    int i;
    Rule r, *rc = alloc((nrules + 1) * sizeof *rc);
    char **str = alloc((nrules + 1) * sizeof *str);

    for (i = 0, r = rules; r; r = r->link) {
        int j = 0;
        char buf[1024], *bp = buf;
        *computekids(r->pattern, "p", bp, &j) = 0;
        for (j = 0; str[j] && strcmp(str[j], buf); j++)
            ;
        if (str[j] == NULL)
            str[j] = strcpy(alloc(strlen(buf) + 1), buf);
        r->kids = rc[j];
        rc[j] = r;
    }
    print("NODEPTR *%Pkids(NODEPTR p, int eruleno, NODEPTR kids[]) {\n"
          "%1%Passert(p, PANIC(\"NULL tree in %Pkids\\n\"));\n"
          "%1%Passert(kids, PANIC(\"NULL kids in %Pkids\\n\"));\n"
          "%1switch (eruleno) {\n");
    for (i = 0; (r = rc[i]); i++) {
        for (; r; r = r->kids)
            print("%1case %d: /* %R */\n", r->ern, r);
        print("%s%2break;\n", str[i]);
    }
    print("%1default:\n%2%Passert(0, PANIC(\"Bad external rule number %%d in "
          "%Pkids\\n\", eruleno));\n%1}\n%1return kids;\n}\n\n");
}

/* emitlabel - emit the labelling functions */
static void emitlabel(Nonterm start) {
    print("struct %Pstate *%Plabel(NODEPTR p) {\n%1%Plabel1(p);\n"
          "%1return ((struct %Pstate *)STATE_LABEL(p))->rule.%P%S ? STATE_LABEL(p) : 0;\n"
          "}\n\n",
          start);
}

/* computents - fill in bp with burm_nts vector for tree t */
static char *computents(Tree t, char *bp) {
    int i;
    if (t) {
        Nonterm p = t->op;
        if (p->kind == NONTERM) {
            sprintf(bp, "%s_%s_NT, ", prefix, p->name);
            bp += strlen(bp);
        } else
            for (i = 0; i < t->nkids; i++)
                bp = computents(t->kids[i], bp);
    }
    return bp;
}

/* emitnts - emit burm_nts ragged array */
static void emitnts(Rule rules, int nrules) {
    Rule r;
    int i, j, *nts = alloc(nrules * sizeof *nts);
    char **str = alloc(nrules * sizeof *str);

    for (i = 0, r = rules; r; r = r->link) {
        char buf[1024];
        *computents(r->pattern, buf) = 0;
        for (j = 0; str[j] && strcmp(str[j], buf); j++)
            ;
        if (str[j] == NULL) {
            print("static short %Pnts_%d[] = { %s0 };\n", j, buf);
            str[j] = strcpy(alloc(strlen(buf) + 1), buf);
        }
        nts[i++] = j;
    }
    print("\nshort *%Pnts[] = {\n");
    for (i = j = 0, r = rules; r; r = r->link) {
        for (; j < r->ern; j++)
            print("%10,%1/* %d */\n", j);
        print("%1%Pnts_%d,%1/* %d */\n", nts[i++], j++);
    }
    print("};\n\n");
}

/* emit_record - emit code that tests for a winning match of rule r */
static void emit_record(char *pre, Rule r) {
    int i;
    char buf[100];

    assert(!r->is_immed);
    print("%sif(%Pcost_code(&c,%d,s) && COST_LESS(c,s->cost[%P%S_NT])) {\n", pre, r->ern,
          r->lhs);
    // print("%s%1delete s->cost[%P%S_NT].mset.getset();\n", pre, r->lhs);
    /* emit cost code */
    if (Tflag)
        print("%Ptrace(%Pnp, %d, c); ", r->ern, r->lhs);
    print("%s%1s->cost[%P%S_NT] = c ;\n%s%1s->rule.%P%S = %d;\n", pre, r->lhs, pre,
          r->lhs, r->packed);
    if (r->lhs->chain)
        print("%s%1%Pclosure_%S(s, c );\n", pre, r->lhs);
    print("%s}\n", pre);
    // print("%s} else\n", pre);
    // print("%s%1delete c.mset.getset();\n", pre);
}

static void emit_record_immediate(char *pre, Rule r) {
    int i;
    char buf[100];

    assert(r->is_immed);
    print("%sif(%Pcost_code(&c,%d,s) && COST_LESS(c,s->cost[%P%S_NT])) {\n", pre, r->ern,
          r->lhs);
    // print("%s%1delete s->cost[%P%S_NT].mset.getset();\n", pre, r->lhs);
    /* emit cost code */
    if (Tflag)
        print("%Ptrace(%Pnp, %d, c); ", r->ern, r->lhs);
    print("%s%1s->cost[%P%S_NT] = c ;\n%s%1s->rule.%P%S = %d;\n", pre, r->lhs, pre,
          r->lhs, r->packed);
    print("%s%1immed_matched=1;\n", pre);
    if (r->lhs->chain)
        print("%s%1%Pclosure_%S(s, c );\n", pre, r->lhs);
    print("%s}\n", pre);
    // print("%s} else\n", pre);
    // print("%s%1delete  c.mset.getset();\n", pre);
}

/* emitrule - emit decoding vectors and burm_rule */
static void emitrule(Nonterm nts) {
    Nonterm p;

    for (p = nts; p; p = p->link) {
        Rule r;
        print("static short %Pdecode_%S[] = {\n%1 -1,\n", p);
        for (r = p->rules; r; r = r->decode)
            print("%1%d,\n", r->ern);
        print("};\n\n");
    }
    print("int %Prule(struct %Pstate *state, int goalnt) {\n"
          "%1%Passert(goalnt >= 1 && goalnt <= %d,\n"
          "%4PANIC(\"Bad goal nonterminal %%d in %Prule\\n\", goalnt));\n"
          "%1if (!state)\n%2return 0;\n%1switch (goalnt) {\n",
          ntnumber);
    for (p = nts; p; p = p->link)
        print("%1case %P%S_NT:"
              "%1return %Pdecode_%S[((struct %Pstate *)state)->rule.%P%S];\n",
              p, p, p);
    print("%1default:\n%2%Passert(0, PANIC(\"Bad goal nonterminal %%d in %Prule\\n\", "
          "goalnt));\n%1}\n%1return 0;\n}\n\n");
}

/* emit_code_exec - emit code for $exec */
static void emit_exec_code() {
    int i = 0;
    Nonterm p;

    print("\n\n#include <stdarg.h>\n\n");

    print("void burm_exec(struct %Pstate *state, int nterm, ...) \n{"
          "\n%1va_list(ap);\n"
          "%1va_start(ap,nterm);\n\n"
          "%1%Passert(nterm >= 1 && nterm <= %d,\n"
          "%4PANIC(\"Bad nonterminal %%d in $exec\\n\", nterm));\n\n"
          "%1if (state)\n%2switch (nterm) {\n",
          ntnumber);

    for (p = nts; p; p = p->link) {
        if (strcmp(p->name, "_")) {
            print("%2case %P%S_NT:\n%3", p);
            if (p->retty) {
                print("PANIC(\"$exec cannot take non-void functions as arguments\\n\");");
                print("\n%3break;\n");
            } else {
                print("%S_action(state", p);
                if (p->argty)
                    for (i = 0; i < Type_ub(p->argty); i++)
                        print(",va_arg(ap,%s)", Type_fetch(p->argty, i));
                print(");\n%3break;\n");
            }
        }
    }
    print("%2default:\n%3PANIC(\"Bad nonterminal %%d in $exec\\n\", "
          "nterm);\n%3break;\n%2}\n%1else\n%2PANIC(\"Bad state for $exec in nonterminal "
          "%%d \\n\",nterm);\n%1va_end(ap);\n}\n\n");

    print("#define EXEC(s,n,a) ( \\\n");
    for (p = nts; p; p = p->link) {
        if (strcmp(p->name, "_")) {
            if (!p->retty) {
                print("%1(n == %P%S_NT)? ", p);
                print("%Pexec(s,n");
                if (p->argty)
                    print(",a): \\\n");
                else
                    print("): \\\n");
            }
        }
    }
    print("%1PANIC(\"Bad nonterminal %%d in $exec\\n\", n))\n\n");
}

/* emitstate - emit state function */
static void emitstate(Term terms, Nonterm start, int ntnumber) {
    int i;
    Term p;
    Nonterm q;

    /* emit the state allocation function */
    print("struct %Pstate *%Palloc_state(NODEPTR u,int op,int arity)\n{\n"
          "%1struct %Pstate *p, **k;\n"
#ifndef SYLIAO
          "%1p = (void *)ALLOC(sizeof *p);\n"
#else
          /* g++ complains about the void pointer */
          "%1p = (struct %Pstate *)ALLOC(sizeof *p);\n"
#endif
          "%1%Passert(p, PANIC(\"1:ALLOC returned NULL in %Palloc_state\\n\"));\n");
    if (Tflag)
        print("%2%Pnp = u;\n");
    print("%1p->op = op;\n"
          "%1p->node = u;\n"
          "%1if(arity){\n"
#ifndef SYLIAO
          "%2k=(void *)ALLOC(arity*sizeof (struct %Pstate *));\n"
#else
          "%2k=(struct %Pstate **)ALLOC(arity*sizeof (struct %Pstate *));\n"
    /* g++ complains about the void pointer */
#endif
          "%2%Passert(k, PANIC(\"2:ALLOC returned NULL in %Palloc_state\\n\"));\n"
          "%2p->kids=k;\n"
          "%1}else\n"
          "%2p->kids=0;\n");
    for (q = nts; q; q = q->link)
        print("%1p->rule.%P%S =\n", q);
    print("%20;\n");
    for (i = 1; i <= ntnumber; i++)
        print("%1p->cost[%d] =\n", i);
    print("%2COST_INFINITY;\n%1return p;\n}\n");

    print("struct %Pstate *%Plabel1(NODEPTR u) {\n"
          "%1int op, arity, i, immed_matched=0;\n"
          "%1COST c=COST_ZERO;\n"
          "%1struct %Pstate *s,**k;\n"
          "%1NODEPTR *children;\n"
          // "%1%Passert(sizeof (int) >= sizeof (void *),PANIC(\"implementation
          // failure\"));\n"
          "%1op=OP_LABEL(u);\n"
          "%1arity=%Parity[op];\n"
          "%1switch(op){\n");
    for (p = terms; p; p = p->link)
        emitcase(p, ntnumber);
    print("%1default:\n"
          "%2%Passert(0, PANIC(\"Bad operator %%d in %Pstate\\n\", op));\n%1}\n"
          "%1return s;\n}\n\n");
}

/* emitstring - emit array of rules and costs */
static void emitstring(Rule rules) {
    Rule r;
    int k;

    print("\nstd::string %Pstring[] = {\n");
    for (k = 0, r = rules; r; r = r->link) {
        assert(k == r->ern);
        print("%1/* %d */%1\"%R\",\n", k++, r);
    }
    print("};\n\n");
}

/*
emit_numbers - emit array of files and line_number
corresponding to the action parts of rules.
*/
static void emit_numbers(Rule rules) {
    Rule r;
    int k;

    /* print a vector of file names */
    print("\nstd::string %Pfiles[] = {\n");
    for (k = 0; k < n_in_files; k++)
        print("\"%s\",\n", in_file_names[k]);
    print("};\n");

    print("\nint %Pfile_numbers[] = {\n");
    for (k = 0, r = rules; r; r = r->link) {
        assert(k == r->ern);
        if (r->action_code)
            print("%1/* %d */%1%d,\n", k++, r->action_code->file_number);
        else
            print("%1/* %d */%1 -1,\n", k++);
    }
    print("};\n");

    print("\nint %Pline_numbers[] = {\n");
    for (k = 0, r = rules; r; r = r->link) {
        if (r->action_code)
            print("%1/* %d */%1%d,\n", k++, r->action_code->line_number);
        else
            print("%1/* %d */%1 -1,\n", k++);
    }
    print("};\n");
}

/* Set the union to hold semantic values */
void set_values(Code *u) {
    if (sem_values)
        yyerror("duplicate %union declaration ignored");
    else
        sem_values = u;
}

/* emitstruct - emit the definition of the state structure */
static void emitstruct(Nonterm nts, int ntnumber) {
    Term p;
    for (p = terms; p; p = p->link)
        print("#define %s %d\n", p->name, p->esn);
    print("\n");
    print("struct %Pstate {\n"
          "%1int op;\n"
          "%1NODEPTR node;\n"
          "%1struct %Pstate **kids;\n"
          "%1COST cost[%d];\n"
          "%1struct {\n",
          ntnumber + 1);
    for (; nts; nts = nts->link) {
        int n = 1, m = nts->lhscount;
        while (m >>= 1)
            n++;
        print("%2unsigned %P%S:%d;\n", nts, n);
    }
    if (sem_values) {
        print("%1} rule;\n"
              "%1union {\n");
        Code_print(sem_values, 0, 0);
        print("%1} value;\n"
              "};\n\n");
    } else
        print("%1} rule;\n};\n\n");
}

/* emitterms - emit terminal data structures */
static void emitterms(Term terms) {
    Term p;
    int k;

    print("char %Parity[] = {\n");
    for (k = 0, p = terms; p; p = p->link) {
        for (; k < p->esn; k++)
            print("%10,%1/* %d */\n", k);
        print("%1%d,%1/* %d=%S */\n", p->arity < 0 ? 0 : p->arity, k++, p);
    }
    print("};\n\n");
    if (Iflag) {
        print("std::string %Popname[] = {\n");
        for (k = 0, p = terms; p; p = p->link) {
            for (; k < p->esn; k++)
                print("%1/* %d */%10,\n", k);
            print("%1/* %d */%1\"%S\",\n", k++, p);
        }
        print("};\n\n");
    }
}

/* emittest - emit clause for testing a match */
static void emittest(Tree t, char *v, char *suffix) {
    Term p = t->op;
    int i;

    if (p->kind == TERM) {
        print("%3%s->op == %d%s /* %S */\n", v, p->esn, t->nkids ? " && " : suffix, p);
        if (t->nkids) {
            for (i = 0; i + 1 < t->nkids; i++)
                emittest(t->kids[i], stringf("%s->kids[%d]", v, i), "&&");
            emittest(t->kids[i], stringf("%s->kids[%d]", v, i), suffix);
        }
    } else {
        Nonterm p = t->op;
        assert(p->kind == NONTERM);
        if (p->name[0] == '_' && p->name[1] == '\0')
            /* if we have an _, it's a don't care */
            print("%31 %s\n", suffix);
        else
            print("%3%s->rule.%P%S%s\n", v, p, suffix);
    }
}

static void emit_action_struct() {
    print("\nstruct %Paction {\n%1int nt;\n%1struct %Pstate* st;\n};\n\n"
          "#ifndef RULE\n#define RULE(n,s) \\\n"
          "%2(act = (burm_action*) malloc(sizeof(struct %Paction)),"
          "act->nt=n,act->st=s,act)\n#endif\n\n");
}

static void emit_cost_code(Rule rules, int nrules) {
    Rule r;
    print("int %Pcost_code(COST *_c, int _ern,struct %Pstate *_s)\n{\n"
          "%1NODEPTR *_children;\n"
          "%1struct %Paction *act;\n"
          "%1switch(_ern){\n"
          "%1default:\n"
          "%2DEFAULT_COST;\n");
    for (r = rules; r; r = r->link)
        if (r->cost_code) {
            if (r->is_immed)
                print("%1case %d:\n"
                      "%2_children = GET_KIDS(_s->node);\n"
                      "{\n",
                      r->ern);
            else
                print("%1case %d:\n{\n", r->ern);
            Code_print(r->cost_code, r, "_s");
            print("\n}\n%1break;\n");
        }
    print("%1}\n%1return 1;\n}\n");
}

static void emit_action_code_headers() {
    Nonterm nt;
    Rule r;
    for (nt = nts; nt; nt = nt->link) {
        /* build function header */
        if (nt->name[0] == '_')
            continue;
        if (nt->return_type == 0) {
            yyerror("missing prototype for %s\n", nt->name);
            continue;
        }
        Code_print(nt->return_type, 0, 0);
        if (nt->argument_types) {
            print(" %S_action(struct %Pstate *_s, ", nt);
            Code_print(nt->argument_types, 0, 0);
        } else
            print(" %S_action(struct %Pstate *_s", nt);
        print(");\n");
    }
}

static void emit_action_code() {
    Nonterm nt;
    Rule r;
    print("struct %Pstate *%Pimmed(struct %Pstate *s,int n);\n"
          "#ifndef NO_ACTION\n"
          "#define NO_ACTION assert(0)\n"
          "#endif\n");
    for (nt = nts; nt; nt = nt->link) {
        if (nt->name[0] == '_' || nt->return_type == 0)
            continue;
        /* build function header */
        Code_print(nt->return_type, 0, 0);
        if (nt->argument_types) {
            print(" %S_action(struct %Pstate *_s, ", nt);
            Code_print(nt->argument_types, 0, 0);
        } else
            print(" %S_action(struct %Pstate *_s", nt);
        print(")\n{\n"
              "%1struct %Pstate *_t;\n"
              "%1int _ern=%Pdecode_%S[_s->rule.%P%S];\n"
              "%1NODEPTR *_children;\n"
              "%1if(_s->rule.%P%S==0)\n"
              "%2NO_ACTION(%S);\n"
              "%1switch(_ern){\n",
              nt, nt, nt, nt);
        for (r = nt->rules; r; r = r->decode)
            if (r->action_code) {
                if (r->is_immed)
                    print("%1case %d:\n"
                          "%2_children = GET_KIDS(_s->node);\n"
                          "{\n",
                          r->ern);
                else
                    print("%1case %d:\n{\n", r->ern);
                Code_print(r->action_code, r, "_s");
                print("\n}\n%1break;\n");
            }
        print("%1}\n}\n");
    }
}

/*
Open the next file in the input file sequence
Return NULL if we have reached the end of the sequence.
*/
FILE *open_next_file(FILE *fp) {
    if (fp != 0)
        fclose(fp);
    current_in_file++;
    if (current_in_file == n_in_files)
        return NULL;
    in_file_name = in_file_names[current_in_file];
    yylineno = 0;
    fp = fopen(in_file_name, "r");
    if (fp == NULL) {
        yyerror("can't read `%s'\n", in_file_name);
        exit(1);
    }
    return fp;
}

void mark_line(int ln, char *fn) {
    if (!Lflag)
        print("# line %d \"%s\"\n", yylineno, in_file_name);
}

void emit_dump_code() {
    print("void dumpCover(NODEPTR p, int goalnt, int indent)\n"
          "{\n"
          "%1int eruleno = %Prule(STATE_LABEL(p), goalnt);\n"
          "%1short *nts = %Pnts[eruleno];\n"
          "%1NODEPTR kids[10];\n"
          "%1int i;\n\n"
          "%1std::cerr << \"\\t\\t\";\n"
          "%1for (i = 0; i < indent; i++)\n"
          "%2std::cerr << \" \";\n"
          "%1std::cerr << burm_string[eruleno] << \"\\n\";\n"
          "%1%Pkids(p, eruleno, kids);\n"
          "%1for (i = 0; nts[i]; i++)\n"
          "%2dumpCover(kids[i], nts[i], indent + 1);\n"
          "}\n\n",
          "%s");
}
