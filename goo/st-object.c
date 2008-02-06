/*
 * st-object.c
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

#include "st-object.h"
#include "st-mark.h"
#include "st-universe.h"
#include "st-class.h"
#include "st-small-integer.h"
#include "st-heap-object.h"

int _st_current_hash = 0;

void
st_object_initialize_header (st_oop object, st_oop klass)
{
    st_heap_object_set_mark (object, st_mark_new ());
    st_heap_object_set_class (object, klass);
}

void
st_object_initialize_body (st_oop object, st_smi instance_size)
{
    st_oop *instvars = st_heap_object_instvars (object);

    for (st_smi i = 0; i < instance_size; i++)
	instvars[i] = st_nil;
}

st_oop
st_object_new (st_oop klass)
{   
    return ST_CLASS_VTABLE (klass)->allocate (klass);
}

st_oop
st_object_new_arrayed (st_oop klass, st_smi size)
{
    return ST_CLASS_VTABLE (klass)->allocate_arrayed (klass, size);
}

/* Meta table */

ST_DEFINE_VTABLE (st_object, NULL);

static bool
is_not_type (void)
{
    return false;
}

static bool
object_equal (st_oop object, st_oop another)
{
    /* identity test */
    return object == another;
}

static bool
object_verify (st_oop object)
{
    return false;
}

static char *
object_describe (st_oop object)
{
    return NULL;
}

static st_oop
allocate (st_oop klass)
{
    g_assert_not_reached ();
    return 0;
}

static st_oop
allocate_arrayed (st_oop klass, st_smi size)
{
    g_assert_not_reached ();
    return 0;
}

static void
st_object_vtable_init (STVTable *table)
{
    table->allocate = allocate;
    table->allocate_arrayed = allocate_arrayed;

    table->equal = object_equal;
    table->verify = object_verify;
    table->describe = object_describe;

    table->is_association = is_not_type;
    table->is_class = is_not_type;
    table->is_metaclass = is_not_type;
    table->is_symbol = is_not_type;
    table->is_float = is_not_type;
    table->is_set = is_not_type;
    table->is_dictionary = is_not_type;
    table->is_compiled_method = is_not_type;
    table->is_compiled_block = is_not_type;
    table->is_block_closure = is_not_type;
    table->is_method_context = is_not_type;
    table->is_block_context = is_not_type;
    table->is_arrayed = is_not_type;
    table->is_array = is_not_type;
    table->is_byte_array = is_not_type;
}
