/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/* 
 * st-heap-object.h
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __ST_HEAP_OBJECT_H__
#define __ST_HEAP_OBJECT_H__

#include <st-types.h>
#include <st-mini.h>
#include <st-mark.h>

/* Each heap object starts with this layout */

typedef struct
{
    st_oop_t mark;
    st_oop_t klass;

} st_header_t;

typedef struct
{
    st_oop_t mark;
    st_oop_t klass;

    st_oop_t instvars[];
} st_heap_object_t;


INLINE st_oop_t  st_heap_object_class       (st_oop_t object);

INLINE void      st_heap_object_set_class   (st_oop_t object, st_oop_t klass);

INLINE int       st_heap_object_hash        (st_oop_t object);

INLINE st_oop_t  st_heap_object_mark        (st_oop_t object);

INLINE void      st_heap_object_set_mark    (st_oop_t object, st_oop_t mark);

INLINE st_oop_t *st_heap_object_instvars    (st_oop_t object);

st_vtable_t     *st_heap_object_vtable      (void);


INLINE st_oop_t
st_heap_object_class (st_oop_t object)
{
    return ST_POINTER (object)->klass;
}

INLINE void
st_heap_object_set_class (st_oop_t object, st_oop_t klass)
{
    ST_POINTER (object)->klass = klass;
}

INLINE int
st_heap_object_hash (st_oop_t object)
{
    return st_mark_hash (ST_POINTER (object)->mark);
}

INLINE st_oop_t
st_heap_object_mark (st_oop_t object)
{
    return ST_POINTER (object)->mark;
}

INLINE void
st_heap_object_set_mark (st_oop_t object, st_oop_t mark)
{
   ST_POINTER (object)->mark = mark;
}

INLINE
st_oop_t *st_heap_object_instvars (st_oop_t object)
{
    return ((st_heap_object_t *) ST_POINTER (object))->instvars;
}

#endif /* __ST_HEAP_OBJECT_H__ */
