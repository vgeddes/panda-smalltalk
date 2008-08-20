/*
 * st-types.h
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#ifndef __ST_TYPES_H__
#define __ST_TYPES_H__

#include <stdbool.h>
#include <stdint.h>
#include <config.h>
#include <stddef.h>

/* Check host data model. We only support LP64 at the moment.
 */
#if (SIZEOF_VOID_P == 4 && SIZEOF_INT == 4)
#  define ST_HOST32             1
#  define ST_HOST64             0
#  define ST_BITS_PER_WORD     32
#  define ST_BITS_PER_INTEGER  32
#elif (SIZEOF_VOID_P == 8)
#  define ST_HOST32            0
#  define ST_HOST64            1
#  define ST_BITS_PER_WORD     64
#  define ST_BITS_PER_INTEGER  32
#else
#  error Sorry, platform not supported. Patches welcome! 
#endif

#define ST_SMALL_INTEGER_MIN  (-ST_SMALL_INTEGER_MAX - 1)
#define ST_SMALL_INTEGER_MAX  536870911

enum {
    ST_SMI_TAG,
    ST_POINTER_TAG,
    ST_CHARACTER_TAG,
    ST_MARK_TAG,
};

#define ST_TAG_SIZE 2

/* basic oop pointer:
 * integral type wide enough to hold a C pointer.
 * Can either point to a heap object or contain a smi or Character immediate.
 */
typedef uintptr_t st_oop;

typedef unsigned char    st_uchar;
typedef unsigned short   st_ushort;
typedef unsigned long    st_ulong;
typedef unsigned int     st_uint;
typedef void *           st_pointer;
typedef st_uint          st_unichar;

static inline st_oop
st_tag_pointer (st_pointer p)
{
    return ((st_oop) p) + ST_POINTER_TAG;
}

static inline st_oop *
st_detag_pointer (st_oop oop)
{
    return (st_oop *) (oop - ST_POINTER_TAG);
}

#endif /* __ST_TYPES_H__ */
