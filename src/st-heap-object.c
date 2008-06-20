/*
 * st-heap-object.c
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

#include "st-heap-object.h"
#include "st-small-integer.h"
#include "st-byte-array.h"
#include "st-object.h"
#include "st-descriptor.h"
#include "st-object-memory.h"


#define HEADER(object) (ST_POINTER (object)->header)

int st_current_hash = 1;

st_uint
st_heap_object_hash (st_oop object)
{
    return (st_uint) ST_POINTER (object)->hash;   
}

void
st_heap_object_set_hash (st_oop object, int value)
{
    ST_POINTER (object)->hash = (st_smi) value;
}

st_uint
st_heap_object_format (st_oop object)
{
    return (HEADER (object) >> st_format_shift) & st_format_mask;
}

void
st_heap_object_set_format (st_oop object, st_uint format)
{
    HEADER (object) = (HEADER (object) & ~st_format_mask_in_place) | (format << st_format_shift);   
}

bool
st_heap_object_marked (st_oop object)
{
    return (HEADER (object) >> st_mark_shift) & st_mark_mask;
}

void
st_heap_object_set_marked (st_oop object, bool mark)
{
    HEADER (object) = (HEADER (object) & ~st_mark_mask_in_place) | ((mark ? 1 : 0) << st_mark_shift);
}

void
st_heap_object_initialize_header (st_oop object, st_oop class)
{
    /* header */
    HEADER (object) = 0 | ST_MARK_TAG;
    st_heap_object_set_format (object, st_smi_value (ST_BEHAVIOR (class)->format));
    st_heap_object_set_marked (object, false);
    st_heap_object_set_hash (object, st_current_hash++);
    st_heap_object_class (object) = class;

    st_assert (st_heap_object_format (object) == (st_smi_value (ST_BEHAVIOR (class)->format)));
}

void
st_heap_object_initialize_body (st_oop object, st_smi instance_size)
{
    st_oop *instvars = st_heap_object_body (object);

    for (st_smi i = 0; i < instance_size; i++)
	instvars[i] = st_nil;
}

void
st_object_verify (st_oop object)
{
    st_assert (st_heap_object_class (object) != st_nil);
    st_assert (st_heap_object_hash (object) != 0);
}

void
st_heap_object_install_forwarding_pointer (st_oop object, st_oop pointer)
{
    ST_POINTER (object)->hash = pointer;
}

st_oop
st_heap_object_forwarding_pointer (st_oop object)
{
    return ST_POINTER (object)->hash;
}

static st_oop
allocate (st_space *space, st_oop class)
{
    st_smi instance_size;
    st_oop object;

    instance_size = st_smi_value (ST_BEHAVIOR (class)->instance_size);
    object = st_space_allocate_object (space, ST_TYPE_SIZE (struct st_header) + instance_size);

    st_heap_object_initialize_header (object, class);
    st_heap_object_initialize_body (object, instance_size);

    return object;
}

static st_oop
object_copy (st_oop object)
{
    st_oop class;
    st_oop copy;
    st_smi instance_size;

    class = st_heap_object_class (object);
    instance_size = st_smi_value (ST_BEHAVIOR (class)->instance_size);
    copy = st_object_new (om->moving_space, class);

    st_oops_copy (st_heap_object_body (copy),
		  st_heap_object_body (object),
		  instance_size);

    return copy;
}

static st_uint
object_size (st_oop object)
{
    return (sizeof (struct st_header) / sizeof (st_oop)) + st_smi_value (ST_BEHAVIOR (st_heap_object_class (object))->instance_size);
}

static void
object_contents (st_oop object, struct contents *contents)
{
    contents->oops = st_heap_object_body (object);
    contents->size = st_smi_value (ST_BEHAVIOR (st_heap_object_class (object))->instance_size);
}

st_descriptor *
st_heap_object_descriptor (void)
{
    static st_descriptor __descriptor =
	{ .allocate         = allocate,
	  .allocate_arrayed = NULL,
	  .copy             = object_copy,
	  .size             = object_size,
	  .contents         = object_contents,
	};

    return & __descriptor;
}

