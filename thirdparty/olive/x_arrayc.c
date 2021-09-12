/*
   FILE: x_arrayc.c

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

X_arrayc *X_arrayc_create(int sz) {
    X_arrayc *xa = (X_arrayc *)malloc(sizeof(X_arrayc));
    xa->frozen = 0;
    xa->size = sz;
    xa->hi = 0;
    xa->data = (char *)malloc(sz * sizeof(char));
    return xa;
}

void X_arrayc_destroy(X_arrayc *xa) {
    free(xa->data);
    free(xa);
}

char X_arrayc_fetch(X_arrayc *xa, int i) {
    assert(0 <= i && i < xa->hi);
    return xa->data[i];
}

void X_arrayc_set(X_arrayc *xa, int i, char v) {
    assert(0 <= i && i < xa->hi);
    xa->data[i] = v;
}

void X_arrayc_grow(X_arrayc *xa, int nhi) {
    X_arrayc_expand(xa, nhi);
    xa->hi = nhi;
}

void X_arrayc_expand(X_arrayc *xa, int nsz) {
    int i;
    char *temp, *dst, *src;
    if (nsz > xa->size) {
        i = xa->hi;
        src = xa->data;
        temp = dst = (char *)malloc(nsz * sizeof(char));
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

int X_arrayc_extend(X_arrayc *xa, char e) {
    if (xa->hi == xa->size)
        X_arrayc_expand(xa, xa->size ? 2 * xa->size : 1);
    xa->data[xa->hi] = e;
    xa->hi++;
    return xa->hi - 1;
}

int X_arrayc_ub(X_arrayc *xa) {
    return xa->hi;
}

void X_arrayc_grab_from(X_arrayc *xa, X_arrayc *r) {
    free(xa->data);
    xa->size = r->size;
    xa->hi = r->hi;
    xa->data = r->data;
    r->size = 0;
    r->hi = 0;
    r->data = 0;
}

void X_arrayc_freeze(X_arrayc *xa) {
    xa->frozen = 1;
}
