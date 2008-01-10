/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-heap-object.h
 *
 * Copyright (C) 2008 Vincent Geddes <vgeddes@gnome.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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


INLINE st_oop_t st_heap_object_class (st_oop_t object);

INLINE void st_heap_object_set_class (st_oop_t object, st_oop_t klass);

INLINE int st_heap_object_hash (st_oop_t object);

INLINE st_oop_t st_heap_object_mark (st_oop_t object);

INLINE void st_heap_object_set_mark (st_oop_t object, st_oop_t mark);

INLINE st_oop_t *st_heap_object_instvars (st_oop_t object);

st_vtable_t *st_heap_object_vtable (void);


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

INLINE st_oop_t *
st_heap_object_instvars (st_oop_t object)
{
    return ((st_heap_object_t *) ST_POINTER (object))->instvars;
}

#endif /* __ST_HEAP_OBJECT_H__ */
