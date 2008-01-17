/* 
 * st-byte-array.c
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

#include "st-byte-array.h"
#include "st-object.h"
#include "st-utils.h"
#include "st-vtable.h"

#include <string.h>

ST_DEFINE_VTABLE (st_byte_array, st_heap_object_vtable ());

static bool
byte_array_verify (st_oop_t object)
{    
    if (!st_heap_object_vtable ()->verify (object))
	return false;

    st_oop_t size = st_byte_array_size (object);
    if (!st_object_is_smi (size) || !(st_smi_value (size) > 0))
	return false;

    return true;
}

INLINE st_smi_t
round_size (st_smi_t size)
{
    /* round off number of bytes to a multiple of st_oop_t size (i.e. a multiple of 4 or 8 bytes,
     * depending on the platform).
     * 
     * we also include space for a null terminator. By doing this, we don't
     * have to allocate and initialize a new terminated string when interfacing with C string functions
     * 
     * TODO: Only include null terminator for *String subclasses of ByteArray 
     *
     */
    size += 1;

    int r = size % sizeof (st_oop_t);

    if (r == 0)
	return size;
    else
	return size - r + sizeof (st_oop_t);
}

static st_oop_t
allocate_arrayed (st_oop_t klass, st_smi_t size)
{
    g_assert (size > 0);

    st_smi_t size_rounded = round_size (size);
    st_oop_t array = st_allocate_object (ST_TYPE_SIZE (st_byte_array_t) + (size_rounded / sizeof (st_oop_t)));

    st_object_initialize_header (array, klass);
    ST_BYTE_ARRAY (array)->size = st_smi_new (size);

    memset (st_byte_array_bytes (array), 0, size_rounded);

    return array;
}

static st_oop_t
allocate (st_oop_t klass)
{
    return allocate_arrayed (klass, 0);
}

static bool
is_byte_array (void)
{
    return true;
}

static bool
is_arrayed (void)
{
    return true;
}

static bool
byte_array_equal (st_oop_t object, st_oop_t another)
{
    st_smi_t size1, size2;

    if (!st_object_is_byte_array (another))
	return false;

    size1 = st_smi_value (st_byte_array_size (object));
    size2 = st_smi_value (st_byte_array_size (another));

    if (size1 != size2)
	return false;

    return memcmp (st_byte_array_bytes (object), st_byte_array_bytes (another), size1) == 0;
}

static guint
byte_array_hash (st_oop_t object)
{
    const signed char *p = (signed char *) st_byte_array_bytes (object);
    guint32 h = *p;

    long size = st_smi_value (st_byte_array_size (object));

    if (size == 0 || !h)
	return h;

    for (st_smi_t i = 1; i < size; i++) {
	h = (h << 5) - h + *(p + i);
    }

    return h;
}

static void
st_byte_array_vtable_init (st_vtable_t * table)
{
    assert_static (sizeof (st_byte_array_t) == (sizeof (st_header_t) + sizeof (st_oop_t)));

    table->allocate = allocate;
    table->allocate_arrayed = allocate_arrayed;

    table->verify = byte_array_verify;

    table->is_byte_array = is_byte_array;
    table->is_arrayed = is_arrayed;

    table->equal = byte_array_equal;
    table->hash = byte_array_hash;
}
