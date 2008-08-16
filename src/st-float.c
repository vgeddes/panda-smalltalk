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
#include "st-behavior.h"


st_oop
st_float_allocate (st_oop class)
{
    st_uint size;
    st_oop object;

    size = ST_SIZE_OOPS (struct st_float);
    object = st_memory_allocate (size);
    if (object == 0) {
	st_memory_perform_gc ();
	class = st_memory_remap_reference (class);
	object = st_memory_allocate (size);
	st_assert (object != 0);
    }

    st_object_initialize_header (object, class);
    st_float_set_value (object, 0.0);

    return object;
}

st_oop
st_float_new (double value)
{
    st_oop object;

    object = st_object_new (ST_FLOAT_CLASS);

    st_float_set_value (object, value);
    
    return object;
}
