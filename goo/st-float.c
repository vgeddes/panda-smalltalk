/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
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

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <st-float.h>
#include <st-object.h>

#include <st-mini.h>


ST_DEFINE_VTABLE (st_float, st_heap_object_vtable ())

static bool
is_float (void)
{
    return true;
}

static st_oop_t
float_allocate (st_oop_t klass)
{
    int object_size = sizeof (st_float_t) / sizeof (st_oop_t);

    st_oop_t f = st_allocate_object (object_size);

    st_object_initialize_header (f, st_float_class);

    st_float_set_value (f, 0.0);

    return f;
}

static void
st_float_vtable_init (st_vtable_t *table)
{
    assert_static (sizeof (st_float_t) == (sizeof (st_header_t) + sizeof (double)));

    table->is_float = is_float;
    table->allocate = float_allocate;
}

st_oop_t
st_float_new (double value)
{	
    st_oop_t f = st_object_new (st_float_class);
	
    st_float_set_value (f, value);	
	
    return f;
}

