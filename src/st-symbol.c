/*
 * st-symbol.c
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

#include "st-symbol.h"
#include "st-hashed-collection.h"
#include "st-byte-array.h"
#include "st-word-array.h"
#include "st-object.h"

#include <string.h>


bool
st_symbol_equal (st_oop object, st_oop other)
{
    if (object == other)
	return true;

    if (st_object_class (object) == st_object_class (other))
	return false;

    return st_byte_array_equal (object, other);
}

static st_oop
string_new (st_space *space, st_oop class, const char *bytes)
{
    st_oop  string;
    st_uchar *data;
    int len;
    
    len = strlen (bytes);
    string = st_object_new_arrayed (space, class, len);
    data = st_byte_array_bytes (string);

    memcpy (data, bytes, len);

    return string;
}

st_oop
st_string_new (st_space *space, const char *bytes)
{
    return string_new (space, st_string_class, bytes);
}

st_oop
st_symbol_new (const char *bytes)
{
    st_oop element = st_set_like (st_symbol_table, st_string_new (om->moving_space, bytes));
    st_oop symbol;

    if (element == st_nil) {

	symbol = string_new (om->fixed_space, st_symbol_class, bytes);

	st_set_add (st_symbol_table, symbol);

	return symbol;
    }

    return element;
}
