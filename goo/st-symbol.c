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
string_new (st_oop klass, const char *bytes)
{
    int len = strlen (bytes);

    st_oop string = st_object_new_arrayed (klass, len);

    guchar *data = st_byte_array_bytes (string);

    memcpy (data, bytes, len);

    return string;
}

st_oop
st_string_new (const char *bytes)
{
    return string_new (st_string_class, bytes);
}

st_oop
st_symbol_new (const char *bytes)
{
    st_oop element = st_set_like (st_symbol_table, st_string_new (bytes));

    if (element == st_nil) {

	st_oop symbol = string_new (st_symbol_class, bytes);

	st_set_add (st_symbol_table, symbol);

	return symbol;
    }

    return element;
}

st_oop
st_character_new (gunichar unichar)
{
    st_oop ch = st_object_new (st_character_class);
    
    st_heap_object_body (ch)[0] = st_smi_new (unichar);

    return ch;    
}

gunichar
st_character_value (st_oop character)
{ 
    return st_smi_value (st_heap_object_body (character)[0]);
}

bool
st_character_equal (st_oop object, st_oop other)
{ 
    if (st_object_class (other) != st_character_class)
	return false;

    return st_character_value (object) == st_character_value (other);
}

guint
st_character_hash (st_oop character)
{ 
    return (guint) st_character_value (character);
}
