/*
 * st-heap-object.c
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

#include "st-heap-object.h"
#include "st-small-integer.h"
#include "st-byte-array.h"
#include "st-object.h"

ST_DEFINE_VTABLE (st_heap_object, st_object_vtable ());

static st_oop
heap_object_allocate (st_oop klass)
{
    st_smi instance_size = st_smi_value (st_behavior_instance_size (klass));

    st_oop object = st_allocate_object (ST_TYPE_SIZE (STHeader) + instance_size);

    st_object_initialize_header (object, klass);
    st_object_initialize_body (object, instance_size);

    return object;
}

static st_oop
heap_object_allocate_arrayed (st_oop klass, st_smi size)
{
    g_critical ("class is not arrayed, use non-arrayed allocation instead");

    return st_nil;
}

static bool
heap_object_verify (st_oop object)
{
    bool verified = true;

    st_oop klass = st_heap_object_class (object);

    verified = verified && st_object_is_heap (object);
    verified = verified && st_object_is_mark (st_heap_object_mark (object));
    verified = verified && (st_object_is_class (klass)
			    || st_object_is_metaclass (klass));

    return verified;
}

static char *
heap_object_describe (st_oop object)
{
    static const char format[] =
	"mark:\t[nonpointer: %i; frozen: %i; hash: %i]\n" "class:\t\n" "name:\t%s\n";

    /* for other object formats, delegate to other implementations */
/*    if (st_object_is_arrayed (object) &&
	st_object_is_class (object) &&
	st_object_is_metaclass (object) &&
	st_object_is_large_integer (object) &&
        st_object_is_float (object))
	return st_object_virtual (object)->describe (object);
  */

    return g_strdup (format);
   



}

static bool
heap_object_equal (st_oop object, st_oop another)
{
    return ST_POINTER (object) == ST_POINTER (another);
}

static guint
heap_object_hash (st_oop object)
{
    return st_mark_hash (st_heap_object_mark (object));
}

static void
st_heap_object_vtable_init (STVTable * table)
{
    /* ensure that STHeader is not padded */
    assert_static (sizeof (STHeader) == 2 * sizeof (st_oop));

    table->allocate = heap_object_allocate;
    table->allocate_arrayed = heap_object_allocate_arrayed;
    table->verify = heap_object_verify;
    table->describe = heap_object_describe;
    table->equal = heap_object_equal;
    table->hash = heap_object_hash;
}
