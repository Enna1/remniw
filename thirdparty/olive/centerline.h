/*
   FILE: centerline.h
  
   Copyright (c) 1997 Princeton University

   All rights reserved.

   This software is to be used for non-commercial purposes only,
   unless authorized permission to do otherwise is obtained.  
   For more information, contact spam@ee.princeton.edu
*/

/*

Fixup for doing var args in centerline.

*/
#ifdef __CENTERLINE__

typedef void *va_list;
#ifndef va_start
#define va_start(list, arg) list = (char *)&arg + sizeof(arg)
#endif
#ifndef __builtin_va_arg_incr
#define __builtin_va_arg_incr(list) (((list) += 1) -1)
#endif
#ifndef va_arg
#define va_arg(list, mode) ((mode *)__builtin_va_arg_incr((mode*)list))[0]
#endif
#ifndef va_end
#define va_end(list)
#endif

#else

#include <stdarg.h>

#endif
