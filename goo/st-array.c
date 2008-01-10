/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
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

#include <st-array.h>
#include <st-universe.h>
#include <st-utils.h>
#include <st-mini.h>

ST_DEFINE_VTABLE (st_array, st_heap_object_vtable ())

     static bool is_array (void)
{
    return true;
}

static bool
is_arrayed (void)
{
    return true;
}

static bool
array_verify (st_oop_t object)
{
    // verify object header
    if (!st_object_super_virtual (object)->verify (object)) {
	printf ("A ");
	return false;


    }
    // variable size
    st_oop_t size = st_array_size (object);
    if (!(st_object_is_smi (size) && (st_smi_value (size) > 0))) {
	g_debug ("GABA: %i\n", st_smi_value (size));
	return false;

    }

    for (st_smi_t i = 1; i <= st_smi_value (size); i++) {

	st_oop_t el = st_array_at (object, i);

	if ((!st_object_is_smi (el)) && (!st_object_is_heap (el)))
	    return false;
    }

    return true;
}

static st_oop_t
allocate_arrayed (st_oop_t klass, st_smi_t size)
{
    g_assert (size > 0);

    st_oop_t array = st_allocate_object (sizeof (st_array_t) / sizeof (st_oop_t) + size);

    st_object_initialize_header (array, klass);
    _ST_ARRAY (array)->size = st_smi_new (size);

    st_oop_t *elements = st_array_element (array, 1);
    for (st_smi_t i = 0; i < size; i++)
	elements[i] = st_nil;

    return array;
}

static st_oop_t
allocate (st_oop_t klass)
{
    return allocate_arrayed (klass, 0);
}

static void
st_array_vtable_init (st_vtable_t * table)
{
    table->allocate = allocate;
    table->allocate_arrayed = allocate_arrayed;

    table->is_array = is_array;
    table->is_arrayed = is_arrayed;
    table->verify = array_verify;
}
