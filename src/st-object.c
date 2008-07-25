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
    ST_OBJECT_CLASS (object) = class;
    st_object_set_format (object, st_smi_value (ST_BEHAVIOR_FORMAT (class)));
    st_object_set_instance_size (object, st_smi_value (ST_BEHAVIOR_INSTANCE_SIZE (class)));
}

bool
st_object_equal (st_oop object, st_oop other)
{
    if (st_object_class (object) == ST_SMI_CLASS)
	return st_smi_equal (object, other);

    if (st_object_class (object) == ST_CHARACTER_CLASS)
	return st_character_equal (object, other);

    if (ST_OBJECT_CLASS (object) == ST_FLOAT_CLASS)
	return st_float_equal (object, other);

    if (ST_OBJECT_CLASS (object) == ST_ASSOCIATION_CLASS)
	return st_association_equal (object, other);

    if (ST_OBJECT_CLASS (object) == ST_SYMBOL_CLASS)
	return st_symbol_equal (object, other);
    
    if (ST_OBJECT_CLASS (object) == ST_BYTE_ARRAY_CLASS ||
	ST_OBJECT_CLASS (object) == ST_STRING_CLASS)
	return st_byte_array_equal (object, other);
    
    return object == other;    
}

st_uint
st_object_hash (st_oop object)
{
    if (st_object_class (object) == ST_SMI_CLASS)
	return st_smi_hash (object);

    if (st_object_class (object) == ST_BYTE_ARRAY_CLASS ||
	st_object_class (object) == ST_STRING_CLASS     ||
	st_object_class (object) == ST_SYMBOL_CLASS)
	return st_byte_array_hash (object);

    if (st_object_class (object) == ST_FLOAT_CLASS)
	return st_float_hash (object);

    if (st_object_class (object) == ST_CHARACTER_CLASS)
	return st_character_hash (object);

    if (st_object_class (object) == ST_ASSOCIATION_CLASS)
	return st_association_hash (object);
    
    return st_identity_ht_hash (memory->ht, object);
}

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
	fields[i] = ST_NIL;

    return object;
}

st_oop
st_handle_allocate (st_oop class)
{
    st_oop *fields;
    st_oop  object;

    object = st_memory_allocate (ST_SIZE_OOPS (struct st_handle));
    st_object_initialize_header (object, class);

    return object;
}
