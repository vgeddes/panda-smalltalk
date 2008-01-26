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

#include "st-float-array.h"

ST_DEFINE_VTABLE (st_float, st_heap_object_vtable ());

static bool
is_arrayed (void)
{
    return true;
}

static st_oop
allocate_arrayed (st_oop klass, st_smi size)
{
    g_assert (size > 0);

    st_oop object = st_allocate_object (ST_TYPE_SIZE (st_float_array) + size);
    
    st_object_initialize_header (object, klass);

    ST_FLOAT_ARRAY (object)->size = size;

    double *elements = ST_FLOAT_ARRAY (object)->elements;

    for (st_smi i = 0; i < size; i++)
	elements[i] = (double) 0;

}

static st_oop
allocate (st_oop klass)
{
    return allocate_arrayed (klass, 0);
}

static void
st_float_vtable_init (STVTable * table)
{
    assert_static (sizeof (st_float_array) == (sizeof (STHeader) + sizeof (st_oop)));

    table->allocate_arrayed = allocate_arrayed;
    table->allocate = allocate;

    table->is_arrayed = is_arrayed;
}
