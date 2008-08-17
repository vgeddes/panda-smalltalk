/*
 * st-class.c
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

#include "st-behavior.h"
#include "st-object.h"
#include "st-dictionary.h"
#include "st-universe.h"
#include "st-symbol.h"
#include "st-object.h"
#include "st-array.h"
#include "st-array.h"
#include "st-handle.h"


st_list *
st_behavior_all_instance_variables (st_oop class)
{
    st_list *list = NULL;
    st_oop names;
    int    size;

    if (class == ST_NIL)
	return NULL;

    names = ST_BEHAVIOR_INSTANCE_VARIABLES (class);
    if (names != ST_NIL) {
	size = st_smi_value (st_arrayed_object_size (names));
	for (int i = 1; i <= size; i++)
	    list = st_list_prepend (list, (st_pointer) st_strdup (st_byte_array_bytes (st_array_at (names, i))));
    }

    return st_list_concat (st_behavior_all_instance_variables (ST_BEHAVIOR_SUPERCLASS (class)),
			   st_list_reverse (list));
}


st_oop
st_object_new (st_oop class)
{
    switch (st_smi_value (ST_BEHAVIOR_FORMAT (class))) {
    case ST_FORMAT_OBJECT:
	return st_object_allocate (class);
    case ST_FORMAT_CONTEXT:
	/* not implemented */
	abort ();
	break;
    case ST_FORMAT_FLOAT:
	return st_float_allocate (class);
    case ST_FORMAT_LARGE_INTEGER:
	return st_large_integer_allocate (class, NULL);
    case ST_FORMAT_HANDLE:
	return st_handle_allocate (class);
    default:
	/* should not reach */
	abort ();
    }
}

st_oop
st_object_new_arrayed (st_oop class, int size)
{
    switch (st_smi_value (ST_BEHAVIOR_FORMAT (class))) {
    case ST_FORMAT_ARRAY:
	return st_array_allocate (class, size);
    case ST_FORMAT_BYTE_ARRAY:
	return st_byte_array_allocate (class, size);
    case ST_FORMAT_WORD_ARRAY:
	return st_word_array_allocate (class, size);
    case ST_FORMAT_FLOAT_ARRAY:
	return st_float_array_allocate (class, size);
    case ST_FORMAT_INTEGER_ARRAY:
	/* not implemented */
	abort ();
	break;
    default:
	/* should not reach */
	abort ();
    }
}
