/*
   FILE: code.h

   Copyright (c) 1997 Princeton University

   All rights reserved.

   This software is to be used for non-commercial purposes only,
   unless authorized permission to do otherwise is obtained.
   For more information, contact spam@ee.princeton.edu
*/

/*
%include "x_array.h"
*/

typedef struct Code_s {
    X_arrayc *c_code;
    int file_number;
    int line_number;
} Code;

#define Code_fetch(c, i)     X_arrayc_fetch(c->c_code, i)
#define Code_set(c, i, v)    X_arrayc_set(c->c_code, i, v)
#define Code_grow(c, i)      X_arrayc_grow(c->c_code, i)
#define Code_expand(c, i)    X_arrayc_expand(c->c_code, i)
#define Code_append(c, i)    X_arrayc_extend(c->c_code, i)
#define Code_extend(c, i)    X_arrayc_extend(c->c_code, i)
#define Code_ub(c)           X_arrayc_ub(c->c_code)
#define Code_grab_from(c, r) X_arrayc_grab_from(c->c_code, r)
#define Code_freeze(c)       X_arrayc_freeze(c->c_code)

Code *Code_create(int sz, int fn, int ln);
void Code_destroy(Code *c);
void Code_append_string(Code *, char *);
char *Code_get_type(Code *c);
/*void Code_print(Code *,Rule t,char *);*/

typedef X_arrayp Type;

#define Type_create(sz)   X_arrayp_create(sz)
#define Type_extend(t, p) X_arrayp_extend(t, p)
#define Type_fetch(t, i)  X_arrayp_fetch(t, i)
#define Type_ub(t)        X_arrayp_ub(t)
#define Type_grow(t)      X_arrayp_grow(t, 5)

Type *Type_get_arguments(Code *code);
Type *Type_get_return(Code *code);
