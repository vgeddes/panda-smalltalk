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
#include "st-descriptor.h"

#include <string.h>

#define ST_BYTE_ARRAY(oop) ((STByteArray *) ST_POINTER (oop))

guchar *
st_byte_array_bytes (st_oop object)
{
    return ST_BYTE_ARRAY (object)->bytes;
}

guchar
st_byte_array_at (st_oop object, st_smi i)
{
    g_assert (1 <= i && i <= st_smi_value (st_arrayed_object_size (object)));

    return st_byte_array_bytes (object)[i - 1];
}

void
st_byte_array_at_put (st_oop object, st_smi i, guchar value)
{
    g_assert (1 <= i && i <= st_smi_value (st_arrayed_object_size (object)));

    st_byte_array_bytes (object)[i - 1] = value;
}

INLINE st_smi
round_size (st_smi size)
{
    /* round off number of bytes to a multiple of st_oop size (i.e. a multiple of 4 or 8 bytes,
     * depending on the platform).
     * 
     * we also include space for a null terminator. By doing this, we don't
     * have to allocate and initialize a new terminated string when interfacing with C string functions.
     * 
     * TODO: Only include null terminator for String subclasses of ByteArray 
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
    g_assert (size >= 0);

    st_smi size_rounded = round_size (size);
    st_oop array = st_allocate_object (ST_TYPE_SIZE (STByteArray) + (size_rounded / sizeof (st_oop)));

    st_heap_object_initialize_header (array, klass);
    ST_ARRAYED_OBJECT (array)->size = st_smi_new (size);

    memset (st_byte_array_bytes (array), 0, size_rounded);

    return array;
}

static st_oop
allocate (st_oop klass)
{
    return allocate_arrayed (klass, 0);
}

bool
st_byte_array_equal (st_oop object, st_oop other)
{
    st_smi size, size_other;

    if (st_object_class (other) != st_byte_array_class &&
	st_object_class (other) != st_string_class &&
	st_object_class (other) != st_symbol_class) 
	return false;

    size = st_smi_value (st_arrayed_object_size (object));
    size_other = st_smi_value (st_arrayed_object_size (other));

    if (size != size_other)
	return false;

    return memcmp (st_byte_array_bytes (object), st_byte_array_bytes (other), size) == 0;
}

guint
st_byte_array_hash (st_oop object)
{
    const signed char *p = (signed char *) st_byte_array_bytes (object);
    guint32 h = *p;

    long size = st_smi_value (st_arrayed_object_size (object));

    if (size == 0 || !h)
	return h;

    for (st_smi i = 1; i < size; i++) {
	h = (h << 5) - h + *(p + i);
    }

    return h;
}

static st_oop
byte_array_copy (st_oop object)
{
    st_oop copy;
    st_smi size;

    size = st_smi_value (st_arrayed_object_size (object));

    copy = allocate_arrayed (st_object_class (object), size);
    
    memcpy (st_byte_array_bytes (copy),
	    st_byte_array_bytes (object),
	    size);
	    
    return copy;
}


const STDescriptor *
st_byte_array_descriptor (void)
{
    static const STDescriptor __descriptor =
	{ .allocate         = allocate,
	  .allocate_arrayed = allocate_arrayed,
	  .copy             = byte_array_copy
	};

    return & __descriptor;
}
