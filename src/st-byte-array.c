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

static st_oop
allocate_arrayed (st_space *space,
		  st_oop    class,
		  st_smi    size)
{
    st_uint size_oops;
    st_oop  array;

    st_assert (size >= 0);

    /* add 1 byte for NULL terminator. Allows toll-free bridging with C string function */
    size_oops = ST_ROUNDED_UP_OOPS (size + 1);
    
    array = st_space_allocate_object (space, class, ST_TYPE_SIZE (struct st_byte_array) + size_oops);
    ST_ARRAYED_OBJECT (array)->size = st_smi_new (size);
    
    memset (st_byte_array_bytes (array), 0, ST_OOPS_TO_BYTES (size_oops));

    return array;
}

bool
st_byte_array_equal (st_oop object, st_oop other)
{
    st_smi size, size_other;

    if (st_object_class (other) != st_byte_array_class &&
	st_object_class (other) != st_string_class &&
	st_object_class (other) != st_symbol_class) 
	return false;

    size = st_smi_value (ST_ARRAYED_OBJECT (object)->size);
    size_other = st_smi_value (ST_ARRAYED_OBJECT (other)->size);

    if (size != size_other)
	return false;

    return memcmp (st_byte_array_bytes (object), st_byte_array_bytes (other), size) == 0;
}

st_uint
st_byte_array_hash (st_oop object)
{
    const signed char *p = (signed char *) st_byte_array_bytes (object);
    st_uint h = *p;

    long size = st_smi_value (ST_ARRAYED_OBJECT (object)->size);

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

    size = st_smi_value (ST_ARRAYED_OBJECT (object)->size);

    copy = allocate_arrayed (om->moving_space, st_object_class (object), size);
    
    memcpy (st_byte_array_bytes (copy),
	    st_byte_array_bytes (object),
	    size);
	    
    return copy;
}

static st_uint
byte_array_size (st_oop object)
{
    st_uint size;

    size = st_smi_value (st_arrayed_object_size (object));
    return (sizeof (struct st_arrayed_object) / sizeof (st_oop)) + ST_ROUNDED_UP_OOPS (size + 1);
}

static void
byte_array_contents (st_oop object, struct contents *contents)
{
    contents->oops = NULL;
    contents->size = 0;
}

st_descriptor *
st_byte_array_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = allocate_arrayed,
	  .copy             = byte_array_copy,
	  .size             = byte_array_size,
	  .contents         = byte_array_contents,
	};

    return & __descriptor;
}
