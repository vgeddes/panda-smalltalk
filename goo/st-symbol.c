/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
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
 
#include <st-symbol.h>
#include <st-hashed-collection.h>
#include <st-byte-array.h>

#include <string.h>

ST_DEFINE_VTABLE (st_symbol, st_byte_array_vtable ())

static bool
symbol_equal (st_oop_t object, st_oop_t another)
{
	if (object == another)
		return true; 
	
	if (st_object_class (object) == st_object_class (another))
		return false;
	
	// now just do a string comparison
	if (st_object_super_virtual (object)->equal (object, another))
		return true;
		
	return false;
}

static bool
is_symbol (void)
{
    return true;
}

static void
st_symbol_vtable_init (st_vtable_t *table)
{
	table->equal = symbol_equal;
	
	table->is_symbol = is_symbol;
}

static st_oop_t
string_new (st_oop_t klass, const char *bytes)
{	
	int len = strlen (bytes);
	
	st_oop_t string = st_object_new_arrayed (klass, len);
	
	guchar *data = st_byte_array_bytes (string);
	
	memcpy (data, bytes, len);
	
	return string;
}

st_oop_t
st_string_new (const char *bytes)
{
	return string_new (st_string_class, bytes);

}

st_oop_t
st_symbol_new (const char *bytes)
{
	st_oop_t element = st_set_like (st_symbol_table, st_string_new (bytes));

	if (element == st_nil) {
	
		st_oop_t symbol = string_new (st_symbol_class, bytes);
		
		st_set_add (st_symbol_table, symbol);
	
		return symbol;
	}

	return element;
}

