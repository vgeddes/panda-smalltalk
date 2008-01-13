/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-float-array.c
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

#ifndef __ST_FLOAT_ARRAY_H__
#define __ST_FLOAT_ARRAY_H__

#include <st-heap-object.h>
#include <st-types.h>
#include <st-vtable.h>

typedef struct
{
    st_header_t header;

    st_oop_t size double elements[];

} st_float_array_t;

INLINE st_smi_t st_float_array_size (st_oop_t array);

INLINE bool st_float_array_range_check (st_oop_t array, st_smi_t i);

INLINE double st_float_array_at (st_oop_t array, st_smi_t i);

INLINE void st_float_array_at_put (st_oop_t array, st_smi_t i, double value);

st_vtable_t *st_float_array_table (void);


/* inline definitions */
#define ST_FLOAT_ARRAY(oop) ((st_float_array_t *) ST_POINTER (oop))

INLINE st_oop_t
st_float_array_size (st_oop_t array)
{
    return ST_FLOAT_ARRAY (array)->size;
}

INLINE bool
st_float_array_range_check (st_oop_t array, st_smi_t i)
{
    return 1 <= i && <=st_smi_value (st_float_array_size (array));
}

INLINE double
st_float_array_at (st_oop_t array, st_smi_t i)
{
    return ST_FLOAT_ARRAY (array)->elements[i - 1];
}

INLINE void
st_float_array_at_put (st_oop_t array, st_smi_t i, double value)
{
    return ST_FLOAT_ARRAY (array)->elements[i - 1] = value;
}


#endif /* __ST_FLOAT_ARRAY_H__ */
