/*
   FILE: code.c

   Copyright (c) 1997 Princeton University

   All rights reserved.

   This software is to be used for non-commercial purposes only,
   unless authorized permission to do otherwise is obtained.
   For more information, contact spam@ee.princeton.edu
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "x_array.h"
#include "code.h"
#include "tree.h"
#include "iburg.h"

Code *Code_create(int sz, int fn, int ln) {
    char buf[100];
    Code *c = (Code *)malloc(sizeof(Code));
    c->c_code = X_arrayc_create(sz);
    c->file_number = fn;
    c->line_number = ln;
    if (fn >= 0) {
        if (Lflag == 0)
            sprintf(buf, "\n# line %d \"%s\"\n", ln, in_file_names[fn]);
        else
            sprintf(buf, "\n\n");
        Code_append_string(c, buf);
    }
    return c;
}

void Code_destroy(Code *c) {
    X_arrayc_destroy(c->c_code);
    free(c);
}

void Code_append_string(Code *cd, char *s) {
    while (*s)
        Code_append(cd, *s++);
}

/* generate code for $n embedded in C code */
static Nonterm selected; /* either a Term or a Nonterm */

int build_state_ref(Tree t, int n, char *buf) {
    int j;
    assert(n > 0);
    n--;
    if (n) {
        for (j = 0; j < t->nkids; j++) {
            sprintf(buf, "->kids[%d]", j);
            n = build_state_ref(t->kids[j], n, buf + strlen(buf));
            if (n == 0)
                return 0;
        }
        return n;
    } else {
        selected = t->op;
        return 0;
    }
}

#define MAX    1000
#define INC(s) (line_within_code += (*s == '\n'), s++)
#define SKIP_BLANKS(s)                                                                   \
    for (; *s;) {                                                                        \
        if (*s != ' ' && *s != '\t')                                                     \
            break;                                                                       \
        INC(s);                                                                          \
    }

#define SKIP_UNTIL(s, delim)                                                             \
    while (*s && strchr(delim, *s) == 0)                                                 \
    INC(s)

static int line_within_code;
static int ref_number;
static char dollar_ref[MAX];

static char *parse_dollar_ref(char *s, Rule r) {
    int n;

    /* extract ref_number */
    ref_number = 0;
    SKIP_BLANKS(s);
    while (isdigit(*s))
        ref_number = ref_number * 10 + ((*INC(s)) - '0');
    SKIP_BLANKS(s);

    /* map to a burm_state access */
    dollar_ref[0] = 0;
    if (ref_number)
        build_state_ref(r->pattern, ref_number, dollar_ref);
    else
        selected = r->lhs;
    assert(strlen(dollar_ref) < MAX);
    return s;
}

static void print_node_reference(Rule r, char *prefix) {
    if (r->is_immed == 0 || ref_number == 1)
        print("%s%s->node", prefix, dollar_ref);
    else
        print("_children[%d]", ref_number - 2);
}

static void print_cost_reference(Rule r, char *prefix) {
    if (ref_number != 0)
        print("%s%s->cost[%P%S_NT]", prefix, dollar_ref, selected);
    else
        print("(*_c)");
}

static void print_value_reference(Rule r, char *prefix) {
    if (ref_number != 0)
        print("%s%s->values", prefix, dollar_ref);
    else
        print("%s->values", prefix);
}

static void print_state_reference(Rule r, char *prefix) {
    if (ref_number != 0)
        print("%s%s", prefix, dollar_ref);
    else
        print("%s", prefix);
}

static char *print_action_ref(char *s) {
    char bf[] = "";
    while (*s != ']')
        strncat(bf, s++, 1);
    print("%s->st,%s->nt,", bf, bf);
    return ++s;
}

static char *print_arguments(char *s) {
    char bf[] = "";
    while (*s != ')')
        strncat(bf, s++, 1);
    if (strlen(bf))
        print("%s", bf);
    else
        print("_");
    return ++s;
}

#define CHECK(cond, label)                                                               \
    if (!(cond))                                                                         \
    goto label

static char *Code_print_dollar(char *s, Code *c, Rule r, char *prefix) {
    char *start;
    char name[MAX];
    Nonterm nt;

    assert(*INC(s) == '$');
    selected = 0;
    ref_number = -1;
    CHECK(r != 0 && prefix != 0, not_permitted);
    switch (*s) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        s = parse_dollar_ref(s, r);
        CHECK(selected, out_of_range);
        print_node_reference(r, prefix);
        break;
    case 's':
        CHECK(strncmp(s, "state[", 6) == 0, unrecognized);
        s = parse_dollar_ref(s + 6, r);
        CHECK(s && *INC(s) == ']', unrecognized);
        CHECK(selected, out_of_range);
        print_state_reference(r, prefix);
        break;
    case 'c':
        CHECK(strncmp(s, "cost[", 5) == 0, unrecognized);
        s = parse_dollar_ref(s + 5, r);
        CHECK(s && *INC(s) == ']', unrecognized);
        CHECK(selected, out_of_range);
        CHECK(selected->kind == NONTERM, not_a_nonterm);
        CHECK(selected->name[0] != '_', underscore);
        print_cost_reference(r, prefix);
        break;
    case 'v':
        CHECK(strncmp(s, "value[", 6) == 0, unrecognized);
        s = parse_dollar_ref(s + 6, r);
        CHECK(s && *INC(s) == ']', unrecognized);
        CHECK(selected, out_of_range);
        CHECK(selected->kind == NONTERM, not_a_nonterm);
        print_value_reference(r, prefix);
        break;
    case 'a':
        CHECK(strncmp(s, "action[", 7) == 0, unrecognized);
        s = parse_dollar_ref(s + 7, r);
        CHECK(s && *INC(s) == ']', unrecognized);
        SKIP_BLANKS(s);
        CHECK(s && *INC(s) == '(', missing_paren);
        CHECK(selected, out_of_range);
        CHECK(selected->kind == NONTERM, not_a_nonterm);
        CHECK(selected->name[0] != '_', underscore);
        CHECK(ref_number != 0, action_zero);
        if (selected->argument_types)
            print("%S_action(%s%s,", selected, prefix, dollar_ref);
        else
            print("%S_action(%s%s", selected, prefix, dollar_ref);
        break;
    case 'r': /* $rule */
        CHECK(strncmp(s, "rule[", 5) == 0, unrecognized);
        s = parse_dollar_ref(s + 5, r);
        CHECK(s && *INC(s) == ']', unrecognized);
        CHECK(selected->name[0] != '_', underscore);
        CHECK(selected, out_of_range);
        CHECK(selected->kind == NONTERM, not_a_nonterm);
        CHECK(ref_number != 0, action_zero);
        print("RULE(%P%S_NT,%s%s)", selected, prefix, dollar_ref);
        break;
    case 'e': /* $exec -- should parse arguments better */
        CHECK(strncmp(s, "exec[", 5) == 0, unrecognized);
        print("EXEC(");
        s = print_action_ref(s + 5);
        CHECK(*s++ == '(', lost_arg);
        s = print_arguments(s);
        print(")");
        break;
    case 'g':
        CHECK(strncmp(s, "getcost[", 8) == 0, unrecognized);
        s = parse_dollar_ref(s + 8, r);
        CHECK(s && *INC(s) == ']', unrecognized);
        CHECK(selected->name[0] != '_', underscore);
        CHECK(selected, out_of_range);
        CHECK(selected->kind == NONTERM, not_a_nonterm);
        print("%s%s->cost[%P%S_NT]", prefix, dollar_ref, selected);
        break;
    case 'o':
        CHECK(strncmp(s, "op[", 3) == 0, unrecognized);
        s = parse_dollar_ref(s + 3, r);
        CHECK(s && *INC(s) == ']', unrecognized);
        CHECK(selected, out_of_range);
        CHECK(selected->name[0] != '_', underscore1);
        print("%s%s->op", prefix, dollar_ref);
        break;
    case 'm':
        CHECK(strncmp(s, "match[", 6) == 0, unrecognized);
        s = parse_dollar_ref(s + 6, r);
        CHECK(s && *INC(s) == ',', unrecognized);
        SKIP_BLANKS(s);
        start = s;
        SKIP_UNTIL(s, " \t]");
        strncpy(name, start, s - start);
        name[s - start] = 0;
        SKIP_BLANKS(s);
        CHECK(s && *INC(s) == ']', unrecognized);
        nt = lookup(name);
        CHECK(nt && nt->kind == NONTERM, not_a_nonterm);
        ckreach(nt);
        CHECK(ref_number != 0, action_zero);
        CHECK(ref_number != 1, not_child);
        CHECK(selected, out_of_range);
        CHECK(selected->kind == NONTERM, not_a_nonterm);
        CHECK(selected->name[0] == '_', not_underscore);
        print("burm_immed(%s,%d)->rule.%P%s", prefix, ref_number - 2, name);
        break;
    case 'i':
        CHECK(strncmp(s, "immed[", 6) == 0, unrecognized);
        s = parse_dollar_ref(s + 6, r);
        CHECK(s && *INC(s) == ',', unrecognized);
        SKIP_BLANKS(s);
        start = s;
        SKIP_UNTIL(s, " \t]");
        strncpy(name, start, s - start);
        name[s - start] = 0;
        SKIP_BLANKS(s);
        CHECK(s && *INC(s) == ']', unrecognized);
        SKIP_BLANKS(s);
        nt = lookup(name);
        CHECK(nt && nt->kind == NONTERM, not_a_nonterm);
        ckreach(nt);
        CHECK(*INC(s) == '(', missing_paren);
        CHECK(ref_number != 0, action_zero);
        CHECK(ref_number != 1, not_child);
        CHECK(selected, out_of_range);
        CHECK(selected->kind == NONTERM, not_a_nonterm);
        CHECK(selected->name[0] == '_', not_underscore);
        if (r->is_immed)
            print("%s_action(burm_immed(%s,%d)%s", name, prefix, ref_number - 2,
                  nt->argument_types ? "," : "");
        else if (nt->argument_types)
            print("%s_action(%s%s,", name, prefix, dollar_ref);
        else
            print("%s_action(%s%s", name, prefix, dollar_ref);
        break;
    default: CHECK(0, unrecognized);
    }
    return s;
not_underscore:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "$immed[%d,%s] does not refer to '_' subtree\n", ref_number, name);
    return s;
not_a_nonterm:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "%s is not a nonterminal used in $immed\n", name);
    return s;
out_of_range:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "out-of-range $%d found in rule\n", ref_number);
    print("$out-of-range[%d]", ref_number);
    return s;
missing_paren:
    yyerror0(c->file_number, c->line_number + line_within_code, "missing parenthesis\n");
    print("$missing_paren[%d]", ref_number);
    return s;
unrecognized:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "unrecognized reference\n");
    print("$unrecognized[%d]", ref_number);
    return s;
not_child:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "$immed is legal only for children\n");
    print("$not_child");
    return s;
action_zero:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "$action[0] is illegal\n");
    print("$action_zero");
    return s;
not_permitted:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "$ references not permitted in prototypes\n");
    return s;
underscore:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "$cost/$action of a '_' subtree\n");
    return s;
underscore1:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "cannot access op for a '_' subtree\n");
    return s;
lost_arg:
    yyerror0(c->file_number, c->line_number + line_within_code,
             "unrecognized argument in $exec function\n");
    return s;
}

#undef SKIP_UNTIL
#undef SKIP_BLANKS
#undef CHECK
#undef MAX

#define PUT(c) putc(c, this_fp)

static char *Code_print_quoted(char *s) {
    char delim;
    delim = *s;
    PUT(*INC(s));
    while (*s)
        switch (*s) {
        case '\\':
            PUT(*INC(s));
            PUT(*INC(s));
            break;
        default:
            PUT(*s);
            if (*INC(s) == delim)
                return s;
        }
    assert(0);
}

static char *Code_print_comment(char *s) {
    while (*s)
        switch (*s) {
        case '*':
            PUT(*INC(s));
            if (*s == '/') {
                PUT(*INC(s));
                return s;
            }
        default: PUT(*INC(s)); break;
        }
}

static char *Code_print_block(char *s, Code *c, Rule r, char *prefix, int null_allowed) {
    line_within_code = 0;
    while (*s)
        switch (*s) {
        case '/':
            PUT(*INC(s));
            if (*s == '*') {
                PUT(*INC(s));
                s = Code_print_comment(s);
            } else
                PUT(*INC(s));
            break;
        case '{':
            PUT(*INC(s));
            s = Code_print_block(s, c, r, prefix, 0);
            PUT(*INC(s));
            break;
        case '\'':
        case '"': s = Code_print_quoted(s); break;
        case '$': s = Code_print_dollar(s, c, r, prefix); break;
        case '}': return s;
        default: PUT(*INC(s)); break;
        }
    assert(null_allowed);
}

#undef PUT

void Code_print(Code *c, Rule r, char *prefix) {
    Code_append(c, 0);
    Code_print_block(c->c_code->data, c, r, prefix, 1);
}

Type *Type_get_arguments(Code *code) {
    int i = 0;
    int nl = 0;
    int skip = 0;

    X_arrayc *buf = X_arrayc_create(20);
    Type *args = Type_create(5);

    if (!code)
        return 0;

    while (nl != 2)
        if (Code_fetch(code, i++) == '\n')
            nl++;

    for (; i < Code_ub(code); i++) {
        switch (Code_fetch(code, i)) {
        case ' ': break;
        case ',':
            X_arrayc_extend(buf, '\0');
            Type_extend(args, buf->data);
            buf = X_arrayc_create(20);
            skip = 0;
            break;
        default:
            if (!skip) {
                X_arrayc_extend(buf, Code_fetch(code, i));
                if (i != Code_ub(code) & Code_fetch(code, i + 1) == ' ')
                    skip = 1;
            }
            break;
        }
    }
    X_arrayc_extend(buf, '\0');
    Type_extend(args, buf->data);
    if (strlen(buf->data))
        return args;
    else
        return 0;
}

Type *Type_get_return(Code *rty) {
    int i = 0;
    int nl = 0;

    X_arrayc *buf = X_arrayc_create(20);
    Type *retty = Type_create(1);

    if (!rty)
        return 0;

    while (nl != 2)
        if (Code_fetch(rty, i++) == '\n')
            nl++;

    for (; i < Code_ub(rty); i++) {
        switch (Code_fetch(rty, i)) {
        case ' ': break;
        default: X_arrayc_extend(buf, Code_fetch(rty, i)); break;
        }
    }
    X_arrayc_extend(buf, '\0');
    if (strcmp(buf->data, "void")) {
        Type_extend(retty, buf->data);
        return retty;
    } else
        return 0;
}
