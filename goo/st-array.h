/* 
 * st-array.h
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

#ifndef __ST_ARRAY_H__
#define __ST_ARRAY_H__

#include <st-types.h>
#include <st-class.h>
#include <st-object.h>
#include <glib.h>

typedef struct
{
    st_header_t header;

    st_oop_t size;

    st_oop_t elements[];

} st_array_t;


INLINE st_oop_t st_array_size (st_oop_t object);

INLINE bool st_array_range_check (st_oop_t object, st_smi_t i);

INLINE st_oop_t *st_array_element (st_oop_t object, st_smi_t i);

INLINE st_oop_t st_array_at (st_oop_t object, st_smi_t i);

INLINE void st_array_at_put (st_oop_t object, st_smi_t i, st_oop_t value);

const st_vtable_t *st_array_vtable (void);


/* inline definitions */
#define ST_ARRAY(oop) ((st_array_t *) ST_POINTER (oop))

INLINE st_oop_t
st_array_size (st_oop_t object)
{
    return ST_ARRAY (object)->size;
}

INLINE bool
st_array_range_check (st_oop_t object, st_smi_t i)
{
    return 1 <= i && i <= st_smi_value (st_array_size (object));
}

/* 
 * returns address of element at index i > 0
 */
INLINE st_oop_t *
st_array_element (st_oop_t object, st_smi_t i)
{
    /* by obtaining the element address as an offset from `&array->size',
     * we avoid the slight overhead of subtraction if we had used `&array->elements[i - 1]' instead.
     */
    return (&ST_ARRAY (object)->size) + i;
}

INLINE st_oop_t
st_array_at (st_oop_t object, st_smi_t i)
{
    g_assert (1 <= i && i <= st_array_size (object));

    return *st_array_element (object, i);
}

INLINE void
st_array_at_put (st_oop_t object, st_smi_t i, st_oop_t value)
{
    g_assert (1 <= i && i <= st_array_size (object));

    *st_array_element (object, i) = value;
}

#endif /* __ST_ARRAY_H__ */
