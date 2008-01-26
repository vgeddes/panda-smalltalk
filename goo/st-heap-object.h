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
#include <st-vtable.h>
#include <st-mark.h>

/* Each heap object starts with this layout */

typedef struct
{
    st_oop mark;
    st_oop klass;

} STHeader;

typedef struct
{
    st_oop mark;
    st_oop klass;

    st_oop instvars[];
} STHeapObject;


INLINE st_oop   st_heap_object_class     (st_oop object);

INLINE void     st_heap_object_set_class (st_oop object, st_oop klass);

INLINE int      st_heap_object_hash     (st_oop object);

INLINE st_oop   st_heap_object_mark     (st_oop object);

INLINE void     st_heap_object_set_mark (st_oop object, st_oop mark);

INLINE st_oop  *st_heap_object_instvars (st_oop object);

const STVTable *st_heap_object_vtable   (void);


INLINE st_oop
st_heap_object_class (st_oop object)
{
    return ST_POINTER (object)->klass;
}

INLINE void
st_heap_object_set_class (st_oop object, st_oop klass)
{
    ST_POINTER (object)->klass = klass;
}

INLINE int
st_heap_object_hash (st_oop object)
{
    return st_mark_hash (ST_POINTER (object)->mark);
}

INLINE st_oop
st_heap_object_mark (st_oop object)
{
    return ST_POINTER (object)->mark;
}

INLINE void
st_heap_object_set_mark (st_oop object, st_oop mark)
{
    ST_POINTER (object)->mark = mark;
}

INLINE st_oop *
st_heap_object_instvars (st_oop object)
{
    return ((STHeapObject *) ST_POINTER (object))->instvars;
}

#endif /* __ST_HEAP_OBJECT_H__ */
