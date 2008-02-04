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

ST_DEFINE_VTABLE (st_float, st_heap_object_vtable ());

static bool
is_arrayed (void)
{
    return true;
}

static st_oop
allocate_arrayed (st_oop klass, st_smi size)
{
    g_assert (size > 0);

    st_oop object = st_allocate_object (ST_TYPE_SIZE (st_float_array) + size);
    
    st_object_initialize_header (object, klass);

    ST_FLOAT_ARRAY (object)->size = size;

    double *elements = ST_FLOAT_ARRAY (object)->elements;

    for (st_smi i = 0; i < size; i++)
	elements[i] = (double) 0;

}

static st_oop
allocate (st_oop klass)
{
    return allocate_arrayed (klass, 0);
}

static void
st_float_vtable_init (STVTable * table)
{
    assert_static (sizeof (st_float_array) == (sizeof (STHeader) + sizeof (st_oop)));

    table->allocate_arrayed = allocate_arrayed;
    table->allocate = allocate;

    table->is_arrayed = is_arrayed;
}
