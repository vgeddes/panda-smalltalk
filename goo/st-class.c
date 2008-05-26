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

#include "st-class.h"
#include "st-heap-object.h"
#include "st-hashed-collection.h"
#include "st-universe.h"
#include "st-symbol.h"
#include "st-object.h"
#include "st-array.h"
#include "st-byte-array.h"
#include "st-descriptor.h"


GList *
st_behavior_all_instance_variables (st_oop klass)
{
    GList *a = NULL, *b = NULL;
    st_oop names;
    st_smi size;
    
    /* superclass of Object is nil */
    if (klass == st_nil)
	return NULL;
    
    a = st_behavior_all_instance_variables (st_behavior_superclass (klass));

    names = st_behavior_instance_variables (klass);
    if (names != st_nil) {
	size = st_smi_value (st_array_size (names));
	for (st_smi i = 1; i <= size; i++)
	    b = g_list_prepend (b, (gpointer) g_strdup (st_byte_array_bytes (st_array_at (names, i))));
    }

    return g_list_concat (a, g_list_reverse (b));
}
