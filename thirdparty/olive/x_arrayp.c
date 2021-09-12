/*
   FILE: x_arrayp.c

   Copyright (c) 1997 Princeton University

   All rights reserved.

   This software is to be used for non-commercial purposes only,
   unless authorized permission to do otherwise is obtained.
   For more information, contact spam@ee.princeton.edu
*/

/* Extendible Arrays (implementation) */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "x_array.h"

X_arrayp *X_arrayp_create(int sz) {
    X_arrayp *xa = (X_arrayp *)malloc(sizeof(X_arrayp));
    xa->frozen = 0;
    xa->size = sz;
    xa->hi = 0;
    xa->data = (void **)malloc(sz * sizeof(void *));
    return xa;
}

void X_arrayp_destroy(X_arrayp *xa) {
    free(xa->data);
    free(xa);
}

void *X_arrayp_fetch(X_arrayp *xa, int i) {
    assert(0 <= i && i < xa->hi);
    return xa->data[i];
}

void X_arrayp_set(X_arrayp *xa, int i, void *v) {
    assert(0 <= i && i < xa->hi);
    xa->data[i] = v;
}

void X_arrayp_grow(X_arrayp *xa, int nhi) {
    X_arrayp_expand(xa, nhi);
    xa->hi = nhi;
}

void X_arrayp_expand(X_arrayp *xa, int nsz) {
    int i;
    void **temp, **dst, **src;
    if (nsz > xa->size) {
        i = xa->hi;
        src = xa->data;
        temp = dst = (void **)malloc(nsz * sizeof(void *));
        xa->size = nsz;
        assert(!xa->frozen);
        while (i > 0) {
            *dst++ = *src++;
            i--;
        }
        free(xa->data);
        xa->data = temp;
    }
}

int X_arrayp_extend(X_arrayp *xa, void *e) {
    if (xa->hi == xa->size)
        X_arrayp_expand(xa, xa->size ? 2 * xa->size : 1);
    xa->data[xa->hi] = e;
    xa->hi++;
    return xa->hi - 1;
}

int X_arrayp_ub(X_arrayp *xa) {
    return xa->hi;
}

void X_arrayp_grab_from(X_arrayp *xa, X_arrayp *r) {
    free(xa->data);
    xa->size = r->size;
    xa->hi = r->hi;
    xa->data = r->data;
    r->size = 0;
    r->hi = 0;
    r->data = 0;
}

void X_arrayp_freeze(X_arrayp *xa) {
    xa->frozen = 1;
}
