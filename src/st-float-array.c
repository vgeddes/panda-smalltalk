/*
 * st-float-array.c
 *
 * Copyright (C) 2008 Vincent Geddes
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

#include "st-float-array.h"
#include "st-object.h"

static st_oop
allocate_arrayed (st_oop class, st_smi size)
{
    st_oop  object;
    double *elements;

    st_assert (size >= 0);

    object = st_allocate_object (ST_TYPE_SIZE (struct st_float_array) + size);
    st_heap_object_initialize_header (object, class);
    ST_ARRAYED_OBJECT (object)->size = st_smi_new (size);

    elements = ST_FLOAT_ARRAY (object)->elements;
    for (st_smi i = 0; i < size; i++)
	elements[i] = (double) 0;

    return object;
}

static st_oop
float_array_copy (st_oop object)
{
    st_oop copy;
    st_smi size;
    
    size = st_smi_value (st_arrayed_object_size (object));

    copy = allocate_arrayed (st_object_class (object), size);

    memcpy (st_float_array_elements (copy),
	    st_float_array_elements (object),
	    sizeof (double) * size);

    return copy;
}

st_descriptor *
st_float_array_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = allocate_arrayed,
	  .copy             = float_array_copy,
	};

    return & __descriptor;
}
