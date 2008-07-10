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
#include "st-universe.h"
#include "st-behavior.h"
#include "st-small-integer.h"
#include "st-association.h"
#include "st-float.h"
#include "st-array.h"
#include "st-array.h"
#include "st-symbol.h"
#include "st-object.h"
#include "st-character.h"
#include "st-unicode.h"

void
st_object_initialize_header (st_oop object, st_oop class)
{
    ST_OBJECT_MARK (object)  = 0 | ST_MARK_TAG;
    ST_OBJECT_HASH (object)  = st_smi_new (st_current_hash++);
    ST_OBJECT_CLASS (object) = class;
    st_object_set_format (object, st_smi_value (ST_BEHAVIOR_FORMAT (class)));
    st_object_set_instance_size (object, st_smi_value (ST_BEHAVIOR_INSTANCE_SIZE (class)));
}

bool
st_object_equal (st_oop object, st_oop other)
{
    if (st_object_class (object) == st_smi_class)
	return st_smi_equal (object, other);

   if (st_object_class (object) == st_character_class)
	return st_character_equal (object, other);

    if (ST_OBJECT_CLASS (object) == st_float_class)
	return st_float_equal (object, other);

    if (ST_OBJECT_CLASS (object) == st_association_class)
	return st_association_equal (object, other);

    if (ST_OBJECT_CLASS (object) == st_symbol_class)
	return st_symbol_equal (object, other);
    
    if (ST_OBJECT_CLASS (object) == st_byte_array_class ||
	ST_OBJECT_CLASS (object) == st_string_class)
	return st_byte_array_equal (object, other);
    
    return object == other;    
}

st_uint
st_object_hash (st_oop object)
{
    if (st_object_class (object) == st_smi_class)
	return st_smi_hash (object);

    if (st_object_class (object) == st_byte_array_class ||
	st_object_class (object) == st_string_class     ||
	st_object_class (object) == st_symbol_class)
	return st_byte_array_hash (object);

    if (st_object_class (object) == st_float_class)
	return st_float_hash (object);

    if (st_object_class (object) == st_character_class)
	return st_character_hash (object);

    if (st_object_class (object) == st_association_class)
	return st_association_hash (object);
    
    return st_smi_value (ST_HEADER (object)->hash);
}

char *
st_object_printString (st_oop object)
{
    char *class_name;
    char *string;

    class_name = (char *) st_byte_array_bytes (ST_CLASS (st_object_class (object))->name);
    
    // SmallInteger
    if (st_object_is_smi (object))
	string = st_strdup_printf ("%li", st_smi_value (object));

    // Float
    else if (st_object_class (object) == st_float_class)
	string = st_strdup_printf ("%g", st_float_value (object));

    // ByteString
    else if (st_object_is_string (object))
	string = st_strdup_printf ("'%s'", (char *) st_byte_array_bytes (object));

    // ByteSymbol
    else if (st_object_is_symbol (object))
	string = st_strdup_printf ("#%s", (char *) st_byte_array_bytes (object));

    // Character
    else if (st_object_class (object) == st_character_class) {
	char outbuf[6] = { 0 };
	st_unichar_to_utf8 (st_character_value (object), outbuf);
	string = st_strdup_printf ("$%s", outbuf);

    // Other
    } else
	string = st_strdup_printf ("%s", class_name);

    return string;
}

int st_current_hash = 1;

st_oop
st_object_allocate (st_oop class)
{
    st_oop *fields;
    st_uint instance_size;
    st_oop  object;

    instance_size = st_smi_value (ST_BEHAVIOR_INSTANCE_SIZE (class));
    object = st_memory_allocate (ST_SIZE_OOPS (struct st_header) + instance_size);
    st_object_initialize_header (object, class);

    fields = ST_OBJECT_FIELDS (object);
    for (st_uint i = 0; i < instance_size; i++)
	fields[i] = st_nil;

    return object;
}
