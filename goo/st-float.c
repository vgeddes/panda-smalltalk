/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-float.c
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

#include <st-float.h>
#include <st-object.h>

#include <st-mini.h>


ST_DEFINE_VTABLE (st_float, st_heap_object_vtable ())

     static bool is_float (void)
{
    return true;
}

static st_oop_t
float_allocate (st_oop_t klass)
{
    int object_size = sizeof (st_float_t) / sizeof (st_oop_t);

    st_oop_t f = st_allocate_object (object_size);

    st_object_initialize_header (f, st_float_class);

    st_float_set_value (f, 0.0);

    return f;
}

static void
st_float_vtable_init (st_vtable_t * table)
{
    assert_static (sizeof (st_float_t) == (sizeof (st_header_t) + sizeof (double)));

    table->is_float = is_float;
    table->allocate = float_allocate;
}

st_oop_t
st_float_new (double value)
{
    st_oop_t f = st_object_new (st_float_class);

    st_float_set_value (f, value);

    return f;
}
