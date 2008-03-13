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

#include <glib.h>
#include <stdbool.h>
#include <stdint.h>
#include <config.h>

#define ST_TAG_SIZE 2

/* Check host data model
 */
#if (SIZEOF_VOID_P == 4 && SIZEOF_INT == 4)
#  define ST_HOST32         1
#  define ST_HOST64         0
#  define ST_BITS_PER_WORD  32
#elif (SIZEOF_VOID_P == 8)
#  define ST_HOST32         0
#  define ST_HOST64         1
#  define ST_BITS_PER_WORD  64
#else
#  error platform not supported
#endif

#define ST_SMALL_INTEGER_MIN  (-ST_SMALL_INTEGER_MAX - 1)
#define ST_SMALL_INTEGER_MAX  536870911

enum {
    ST_SMI_TAG,
    ST_POINTER_TAG,
    ST_MARK_TAG,
    ST_TAG_UNUSED,
};

/* basic oop pointer:
 * integral type wide enough to hold a C pointer.
 * Can either point to a heap object or contain a smi or mark immediate.
 */
typedef uintptr_t st_oop;

/* signed integral type which can hold the value of a smi immediate
 */
#if ST_HOST32 == 1
typedef int32_t st_smi;
#else
typedef int64_t st_smi;
#endif

/* glib already defines `inline' in a portable manner
 */
#ifdef INLINE
#  undef INLINE
#endif
#define INLINE static inline

#define ST_POINTER(oop)          ((STHeader *) ((oop) - ST_POINTER_TAG))
#define ST_OOP(ptr)              (((st_oop) (ptr)) + ST_POINTER_TAG)

#endif /* __ST_TYPES_H__ */
