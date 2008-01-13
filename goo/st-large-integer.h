/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-large-integer.h
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

#ifndef _ST_LARGE_INTEGER__
#define _ST_LARGE_INTEGER__

#include <tommath.h>
#include <st-heap-object.h>

typedef struct
{
    st_header_t header;

    mp_int value;

} st_large_integer_t;


st_oop_t st_large_integer_new (mp_int * value);

st_oop_t st_large_integer_new_from_string (const char *string, guint radix);

char *st_large_integer_to_string (st_oop_t integer, guint radix);

INLINE mp_int *st_large_integer_value (st_oop_t integer);

const st_vtable_t *st_large_integer_vtable (void);


/* inline definitions */
#define ST_LARGE_INTEGER(oop) ((st_large_integer_t *) ST_POINTER (oop))

mp_int *
st_large_integer_value (st_oop_t integer)
{
    return &ST_LARGE_INTEGER (integer)->value;
}

#endif /* _ST_LARGE_INTEGER */
