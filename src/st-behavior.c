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
#include "st-hashed-collection.h"
#include "st-universe.h"
#include "st-symbol.h"
#include "st-object.h"
#include "st-array.h"
#include "st-byte-array.h"
#include "st-descriptor.h"


st_list *
st_behavior_all_instance_variables (st_oop class)
{
    st_list *a = NULL, *b = NULL;
    st_oop names;
    st_smi size;
    
    if (class == st_nil)
	return NULL;
    
    a = st_behavior_all_instance_variables (ST_BEHAVIOR (class)->superclass);

    names = ST_BEHAVIOR (class)->instance_variables;
    if (names != st_nil) {
	size = st_smi_value (st_arrayed_object_size (names));
	for (st_smi i = 1; i <= size; i++)
	    b = st_list_prepend (b, (st_pointer) st_strdup (st_byte_array_bytes (st_array_at (names, i))));
    }

    return st_list_concat (a, st_list_reverse (b));
}
