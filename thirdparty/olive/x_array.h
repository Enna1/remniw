/*
   FILE: x_array.h

   Copyright (c) 1997 Princeton University

   All rights reserved.

   This software is to be used for non-commercial purposes only,
   unless authorized permission to do otherwise is obtained.
   For more information, contact spam@ee.princeton.edu
*/

#ifndef X_ARRAY
#define X_ARRAY

/*

Extendible Arrays --- Steve Tjiang
Adapted from SUIF system.

The data structures defined in this file use the doubling strategy.
We have three versions:
- one optimized for elements that fit in a char,
- one optimized for elements that fit into a (void *),
- another for variable size objects.

*/

/*

An extensible array optimized for elements of the same size as a pointer.

frozen:  A frozen array cannot be extended or moved.  If the user
has pointers into the array then the user freeze the array.

size: the number of elements which can be held in the storage pointed to
by data.

hi: An index---which should always be strictly less than size---
the first free element in data.

*/

#include <stdio.h>

/* optimized for characters */
typedef struct _X_arrayc {
    int frozen;
    int size;
    int hi;
    char *data;
} X_arrayc;

X_arrayc *X_arrayc_create(int sz);
void X_arrayc_destroy(X_arrayc *);
char X_arrayc_fetch(X_arrayc *, int);
void X_arrayc_set(X_arrayc *, int, char);
void X_arrayc_grow(X_arrayc *, int);
void X_arrayc_expand(X_arrayc *, int);
int X_arrayc_extend(X_arrayc *, char);
int X_arrayc_ub(X_arrayc *);
void X_arrayc_grab_from(X_arrayc *, X_arrayc *);
void X_arrayc_freeze(X_arrayc *);

/* optimized for void * */
typedef struct _X_arrayp {
    int frozen;
    int size;
    int hi;
    void **data;
} X_arrayp;

X_arrayp *X_arrayp_create(int sz);
void X_arrayp_destroy(X_arrayp *);
void *X_arrayp_fetch(X_arrayp *, int);
void X_arrayp_set(X_arrayp *, int, void *);
void X_arrayp_grow(X_arrayp *, int);
void X_arrayp_expand(X_arrayp *, int);
int X_arrayp_extend(X_arrayp *, void *);
int X_arrayp_ub(X_arrayp *);
void X_arrayp_grab_from(X_arrayp *, X_arrayp *);
void X_arrayp_freeze(X_arrayp *);

/*
  generalized for arbitray sized element.
  Implemented as a macro.
  Beware: User is responsible for instantiating implementation
  */
#define DECLARE_X_ARRAY(PREFIX, TYPE)                                                    \
    typedef struct _##PREFIX {                                                           \
        int frozen;                                                                      \
        int size;                                                                        \
        int hi;                                                                          \
        TYPE *data;                                                                      \
    } PREFIX;                                                                            \
                                                                                         \
    PREFIX *PREFIX##_create(int sz, int esize);                                          \
    void PREFIX##_destroy(PREFIX *);                                                     \
    TYPE PREFIX##_fetch(PREFIX *, int);                                                  \
    TYPE *PREFIX##_index(PREFIX *, int);                                                 \
    void PREFIX##_set(PREFIX *, int, char *);                                            \
    void PREFIX##_grow(PREFIX *, int);                                                   \
    void PREFIX##_expand(PREFIX *, int);                                                 \
    int PREFIX##_extend(PREFIX *, char *);                                               \
    int PREFIX##_ub(PREFIX *);                                                           \
    void PREFIX##_grab_from(PREFIX *, PREFIX *);                                         \
    void PREFIX##_freeze(PREFIX *)

/*
  The user must instantiate an implementation for a DECLARE_X_ARRAY.
  That can be done with the following code.
  #DECLARE_X_ARRAY(prefix,type)
  #define PREFIX prefix
  #define TYPE type
  #include "x_array.implem"
  */

#endif
