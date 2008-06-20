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


static st_oop
allocate_arrayed (st_space *space,
		  st_oop    class,
		  st_smi    size)
{
    st_oop  array;
    st_oop *elements;

    st_assert (size >= 0);

    array = st_space_allocate_object (space, ST_TYPE_SIZE (struct st_array) + size);

    st_heap_object_initialize_header (array, class);
    ST_ARRAYED_OBJECT (array)->size = st_smi_new (size);    

    elements = ST_ARRAY (array)->elements;
    for (st_smi i = 0; i < size; i++)
	elements[i] = st_nil;

    return array;
}

static st_oop
array_copy (st_oop object)
{
    st_oop copy;
    st_smi size;

    size = st_smi_value (ST_ARRAYED_OBJECT (object)->size);

    copy = st_object_new_arrayed (om->moving_space, st_object_class (object), size);

    st_oops_copy (ST_ARRAY (copy)->elements,
		  ST_ARRAY (object)->elements,
		  size);

    return copy;
}

static st_uint
array_size (st_oop object)
{
    return (sizeof (struct st_arrayed_object) / sizeof (st_oop)) + st_smi_value (st_arrayed_object_size (object));
}

static void
array_contents (st_oop object, struct contents *contents)
{
    contents->oops = st_array_elements (object);
    contents->size = st_smi_value (st_arrayed_object_size (object));
}

st_descriptor *
st_array_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = allocate_arrayed,
	  .copy             = array_copy,
	  .size             = array_size,
	  .contents         = array_contents,
	};

    return & __descriptor;
}

