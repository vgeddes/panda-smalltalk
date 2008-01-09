/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/* 
 * st-association.h
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
 
#ifndef __ST_ASSOCIATION_H__
#define __ST_ASSOCIATION_H__

#include <st-heap-object.h>
#include <st-types.h>
#include <st-mini.h>

typedef struct
{
    st_header_t header;
	
    st_oop_t     key;
    st_oop_t     value;
	
} st_association_t;

st_oop_t         st_association_new       (st_oop_t key, st_oop_t value);

INLINE st_oop_t  st_association_key       (st_oop_t assoc);

INLINE st_oop_t  st_association_value     (st_oop_t assoc);

INLINE void      st_association_set_key   (st_oop_t assoc, st_oop_t key);

INLINE void      st_association_set_value (st_oop_t assoc, st_oop_t value);

st_vtable_t     *st_association_vtable    (void);

/* inline definitions */
#define _ST_ASSOCIATION(oop) ((st_association_t *) ST_POINTER (oop))

INLINE st_oop_t
st_association_key (st_oop_t assoc)
{
    return _ST_ASSOCIATION (assoc)->key;
}

INLINE st_oop_t
st_association_value (st_oop_t assoc)
{
    return _ST_ASSOCIATION (assoc)->value;
}

INLINE void
st_association_set_key (st_oop_t assoc, st_oop_t key)
{
    _ST_ASSOCIATION (assoc)->key = key;
}

INLINE void
st_association_set_value (st_oop_t assoc, st_oop_t value)
{
    _ST_ASSOCIATION (assoc)->value = value;
}

#endif /* __ST_ASSOCIATION_H__ */
