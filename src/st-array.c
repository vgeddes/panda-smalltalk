/*
 * st-array.c
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
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#include "st-array.h"
#include "st-universe.h"
#include "st-utils.h"
#include "st-descriptor.h"
#include "st-behavior.h"
#include <string.h>

static st_oop
array_allocate_arrayed (st_oop    class,
			st_smi    size)
{
    st_oop  array;
    st_oop *elements;

    st_assert (size >= 0);

    array = st_memory_allocate (ST_SIZE_OOPS (struct st_array) + size);
    st_object_initialize_header (array, class);

    ST_ARRAYED_OBJECT (array)->size = st_smi_new (size);
    elements = ST_ARRAY (array)->elements;
    for (st_smi i = 0; i < size; i++)
	elements[i] = st_nil;

    return array;
}

static st_uint
array_size (st_oop object)
{
    return ST_SIZE_OOPS (struct st_arrayed_object) + st_smi_value (st_arrayed_object_size (object));
}

static void
array_contents (st_oop object, st_oop **oops, st_uint *size)
{
    *oops = st_array_elements (object);
    *size = st_smi_value (st_arrayed_object_size (object));
}

st_descriptor *
st_array_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = array_allocate_arrayed,
	  .size             = array_size,
	  .contents         = array_contents,
	};

    return & __descriptor;
}

/* ByteArray */

static st_oop
byte_allocate_arrayed (st_oop    class,
		       st_smi    size)
{
    st_uint size_oops;
    st_oop  array;

    st_assert (size >= 0);

    /* add 1 byte for NULL terminator. Allows toll-free bridging with C string function */
    size_oops = ST_ROUNDED_UP_OOPS (size + 1);
    
    array = st_memory_allocate (ST_SIZE_OOPS (struct st_byte_array) + size_oops);
    st_object_initialize_header (array, class);

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

static st_uint
byte_array_size (st_oop object)
{
    st_uint size;

    size = st_smi_value (st_arrayed_object_size (object));
    return ST_SIZE_OOPS (struct st_arrayed_object) + ST_ROUNDED_UP_OOPS (size + 1);
}

static void
byte_array_contents (st_oop object, st_oop **oops, st_uint *size)
{
    *oops = NULL;
    *size = 0;
}

st_descriptor *
st_byte_array_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = byte_allocate_arrayed,
	  .size             = byte_array_size,
	  .contents         = byte_array_contents,
	};

    return & __descriptor;
}

/* WordArray */

static st_oop
word_allocate_arrayed (st_oop class, st_smi size)
{
    st_oop   array;
    st_smi   size_oops;
    st_uint *elements;

    st_assert (size >= 0);

    size_oops = size / (sizeof (st_oop) / sizeof (st_uint));

    array = st_memory_allocate (ST_SIZE_OOPS (struct st_word_array) + size_oops);
    st_object_initialize_header (array, class);

    ST_ARRAYED_OBJECT (array)->size = st_smi_new (size);
    elements = st_word_array_elements (array);
    for (st_smi i = 0; i < size; i++)
	elements[i] = 0;

    return array;
}


static st_uint
word_array_size (st_oop object)
{
    return ST_SIZE_OOPS (struct st_arrayed_object)
	+ (st_smi_value (st_arrayed_object_size (object)) / (sizeof (st_oop) / sizeof (st_uint)));
}

static void
word_array_contents (st_oop object, st_oop **oops, st_uint *size)
{
    *oops = NULL;
    *size = 0;
}

st_descriptor *
st_word_array_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = word_allocate_arrayed,
	  .size             = word_array_size,
	  .contents         = word_array_contents,
	};
    return & __descriptor;
}


/* FloatArray */

static st_oop
float_allocate_arrayed (st_oop class, st_smi size)
{
    st_oop  object;
    double *elements;
    st_smi  size_oops;

    st_assert (size >= 0);
    
    /* get actual size in oops (dependent on whether system is 64bit or 32bit) */ 
    size_oops = size * (sizeof (double) / sizeof (st_oop));

    object = st_memory_allocate (ST_SIZE_OOPS (struct st_float_array) + size_oops);
    st_object_initialize_header (object, class);

    ST_ARRAYED_OBJECT (object)->size = st_smi_new (size);

    elements = ST_FLOAT_ARRAY (object)->elements;
    for (st_smi i = 0; i < size; i++)
	elements[i] = (double) 0;

    return object;
}

static st_uint
float_array_size (st_oop object)
{
    return ST_SIZE_OOPS (struct st_arrayed_object) + (st_smi_value (st_arrayed_object_size (object)) * ST_SIZE_OOPS (double));
}

static void
float_array_contents (st_oop object, st_oop **oops, st_uint *size)
{
    *oops = NULL;
    *size = 0;
}

st_descriptor *
st_float_array_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = float_allocate_arrayed,
	  .size             = float_array_size,
	  .contents         = float_array_contents,
	};

    return & __descriptor;
}
