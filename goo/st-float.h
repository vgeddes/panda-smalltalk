/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-float.h
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

#ifndef __ST_FLOAT_H__
#define __ST_FLOAT_H__

#include <st-types.h>
#include <st-heap-object.h>

typedef struct
{
    st_header_t header;

    double value;

} st_float_t;


st_oop_t st_float_new (double value);

INLINE double st_float_value (st_oop_t f);

INLINE void st_float_set_value (st_oop_t f, double value);

const st_vtable_t *st_float_vtable (void);


/* inline definitions */
#define ST_FLOAT(oop) ((st_float_t *) ST_POINTER (oop))

INLINE double
st_float_value (st_oop_t f)
{
    return ST_FLOAT (f)->value;
}

INLINE void
st_float_set_value (st_oop_t f, double value)
{
    ST_FLOAT (f)->value = value;
}

#endif /* __ST_FLOAT_H__ */
