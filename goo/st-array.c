/* 
 * st-array.c
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

#include "st-array.h"
#include "st-universe.h"
#include "st-utils.h"
#include "st-vtable.h"

ST_DEFINE_VTABLE (st_array, st_heap_object_vtable ());

static bool
is_array (void)
{
    return true;
}

static bool
is_arrayed (void)
{
    return true;
}

static bool
array_verify (st_oop object)
{
    if (!st_heap_object_vtable ()->verify (object))
	return false;
   
    // variable size
    st_oop size = st_array_size (object);
    if (!(st_object_is_smi (size) && (st_smi_value (size) > 0))) {
	g_debug ("GABA: %li\n", st_smi_value (size));
	return false;

    }

    for (st_smi i = 1; i <= st_smi_value (size); i++) {

	st_oop el = st_array_at (object, i);

	if ((!st_object_is_smi (el)) && (!st_object_is_heap (el)))
	    return false;
    }

    return true;
}

static st_oop
allocate_arrayed (st_oop klass, st_smi size)
{
    g_assert (size > 0);

    st_oop array = st_allocate_object (ST_TYPE_SIZE (STArray) + size);

    st_object_initialize_header (array, klass);
    ST_ARRAY (array)->size = st_smi_new (size);

    st_oop *elements = st_array_element (array, 1);
    for (st_smi i = 0; i < size; i++)
	elements[i] = st_nil;

    return array;
}

static st_oop
allocate (st_oop klass)
{
    return allocate_arrayed (klass, 0);
}

static void
st_array_vtable_init (STVTable * table)
{
    table->allocate = allocate;
    table->allocate_arrayed = allocate_arrayed;

    table->is_array = is_array;
    table->is_arrayed = is_arrayed;
    table->verify = array_verify;
}
