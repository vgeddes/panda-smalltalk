/*
 * st-byte-array.c
 *
 * Copyright (c) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#include "st-byte-array.h"
#include "st-object.h"
#include "st-utils.h"
#include "st-vtable.h"

#include <string.h>

ST_DEFINE_VTABLE (st_byte_array, st_heap_object_vtable ());


#define ST_BYTE_ARRAY(oop) ((STByteArray *) ST_POINTER (oop))


st_oop
st_byte_array_size (st_oop object)
{
    return ST_BYTE_ARRAY (object)->size;
}

guchar *
st_byte_array_bytes (st_oop object)
{
    return ST_BYTE_ARRAY (object)->bytes;
}

bool
st_byte_array_range_check (st_oop object, st_smi i)
{
    return 1 <= i && i <= st_smi_value (st_byte_array_size (object));
}

guchar
st_byte_array_at (st_oop object, st_smi i)
{
    g_assert (1 <= i && i <= st_smi_value (st_byte_array_size (object)));

    return st_byte_array_bytes (object)[i - 1];
}

void
st_byte_array_at_put (st_oop object, st_smi i, guchar value)
{
    g_assert (1 <= i && i <= st_smi_value (st_byte_array_size (object)));

    st_byte_array_bytes (object)[i - 1] = value;
}


static bool
byte_array_verify (st_oop object)
{    
    if (!st_heap_object_vtable ()->verify (object))
	return false;

    st_oop size = st_byte_array_size (object);
    if (!st_object_is_smi (size) || !(st_smi_value (size) > 0))
	return false;

    return true;
}

INLINE st_smi
round_size (st_smi size)
{
    /* round off number of bytes to a multiple of st_oop size (i.e. a multiple of 4 or 8 bytes,
     * depending on the platform).
     * 
     * we also include space for a null terminator. By doing this, we don't
     * have to allocate and initialize a new terminated string when interfacing with C string functions
     * 
     * TODO: Only include null terminator for *String subclasses of ByteArray 
     *
     */
    size += 1;

    int r = size % sizeof (st_oop);

    if (r == 0)
	return size;
    else
	return size - r + sizeof (st_oop);
}

static st_oop
allocate_arrayed (st_oop klass, st_smi size)
{
    g_assert (size > 0);

    st_smi size_rounded = round_size (size);
    st_oop array = st_allocate_object (ST_TYPE_SIZE (STByteArray) + (size_rounded / sizeof (st_oop)));

    st_object_initialize_header (array, klass);
    ST_BYTE_ARRAY (array)->size = st_smi_new (size);

    memset (st_byte_array_bytes (array), 0, size_rounded);

    return array;
}

static st_oop
allocate (st_oop klass)
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
byte_array_equal (st_oop object, st_oop another)
{
    st_smi size1, size2;

    if (!st_object_is_byte_array (another))
	return false;

    size1 = st_smi_value (st_byte_array_size (object));
    size2 = st_smi_value (st_byte_array_size (another));

    if (size1 != size2)
	return false;

    return memcmp (st_byte_array_bytes (object), st_byte_array_bytes (another), size1) == 0;
}

static guint
byte_array_hash (st_oop object)
{
    const signed char *p = (signed char *) st_byte_array_bytes (object);
    guint32 h = *p;

    long size = st_smi_value (st_byte_array_size (object));

    if (size == 0 || !h)
	return h;

    for (st_smi i = 1; i < size; i++) {
	h = (h << 5) - h + *(p + i);
    }

    return h;
}

static void
st_byte_array_vtable_init (STVTable * table)
{
    assert_static (sizeof (STByteArray) == (sizeof (STHeader) + sizeof (st_oop)));

    table->allocate = allocate;
    table->allocate_arrayed = allocate_arrayed;

    table->verify = byte_array_verify;

    table->is_byte_array = is_byte_array;
    table->is_arrayed = is_arrayed;

    table->equal = byte_array_equal;
    table->hash = byte_array_hash;
}
