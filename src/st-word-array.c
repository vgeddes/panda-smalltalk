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

#include "st-word-array.h"
#include "st-universe.h"
#include "st-utils.h"
#include "st-descriptor.h"

static st_smi
round_size (st_smi size)
{    
#if ST_HOST64 == 1

    st_assert (sizeof (st_oop) == (2 * sizeof (st_uint)));

    int r = size % 2;

    if (r == 0)
	return size;
    else
	return size - r + 2;

#else

    st_assert (sizeof (st_oop) == sizeof (st_uint));

    return size;

#endif
}


static st_oop
allocate_arrayed (st_space *space, st_oop class, st_smi size)
{
    st_oop  array;
    st_smi  size_rounded;
    st_uint  *elements;

    st_assert (size >= 0);

    size_rounded = round_size (size);
    array = st_space_allocate_object (space, class, ST_TYPE_SIZE (struct st_word_array) + size_rounded);

    ST_ARRAYED_OBJECT (array)->size = st_smi_new (size);
    elements = st_word_array_elements (array);
    for (st_smi i = 0; i < size_rounded; i++)
	elements[i] = 0;

    return array;
}

static st_oop
word_array_copy (st_oop object)
{
    st_oop copy;
    st_smi size;
    
    size = st_smi_value (st_arrayed_object_size (object));

    copy = allocate_arrayed (om->moving_space, st_object_class (object), size);

    memcpy (st_word_array_elements (copy),
	    st_word_array_elements (object),
	    sizeof (st_uint) * size);

    return copy;
}

static st_uint
word_array_size (st_oop object)
{
    return (sizeof (struct st_arrayed_object) / sizeof (st_oop)) + (2 * st_smi_value (st_arrayed_object_size (object)));
}

static void
word_array_contents (st_oop object, struct contents *contents)
{
    contents->oops = NULL;
    contents->size = 0;
}

st_descriptor *
st_word_array_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = allocate_arrayed,
	  .copy             = word_array_copy,
	  .size             = word_array_size,
	  .contents         = word_array_contents,
	};
    return & __descriptor;
}

