/*
 * st-float.c
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

#include "st-float.h"
#include "st-object.h"

static st_oop
allocate (st_oop class)
{
    st_oop f = st_allocate_object (ST_TYPE_SIZE (struct st_float));

    st_heap_object_initialize_header (f, st_float_class);

    st_float_set_value (f, 0.0);

    return f;
}

st_oop
st_float_new (double value)
{
    st_oop f = st_object_new (st_float_class);

    st_float_set_value (f, value);
    
    return f;
}

static st_oop
float_copy (st_oop object)
{
    return st_float_new (st_float_value (object));
}

st_descriptor *
st_float_descriptor (void)
{
    assert_static (sizeof (struct st_float) == (sizeof (struct st_header) + sizeof (double)));
    
    static st_descriptor __descriptor =
	{ .allocate         = allocate,
	  .allocate_arrayed = NULL,
	  .copy             = float_copy
	};
    
    return & __descriptor;
}
