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
allocate_arrayed (st_oop klass, st_smi size)
{
    g_assert (size >= 0);

    st_oop array = st_allocate_object (ST_TYPE_SIZE (STArray) + size);

    st_heap_object_initialize_header (array, klass);
    ST_ARRAY (array)->size = st_smi_new (size);    

    st_oop *elements = st_array_element (array, 1);
    for (st_smi i = 0; i < size; i++)
	elements[i] = st_nil;

    return array;
}

static st_oop
allocate (st_oop klass)
{
    return allocate_arrayed (klass, 0);
}

const STDescriptor *
st_array_descriptor (void)
{
    static const STDescriptor __descriptor =
	{ .allocate         = allocate,
	  .allocate_arrayed = allocate_arrayed,
	};

    return & __descriptor;
}
