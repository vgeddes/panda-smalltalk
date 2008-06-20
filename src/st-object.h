/*
 * st-object.h
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

#ifndef __ST_OBJECT_H__
#define __ST_OBJECT_H__

#include <st-types.h>
#include <st-small-integer.h>
#include <st-utils.h>
#include <st-behavior.h>
#include <st-universe.h>

INLINE int    st_object_tag               (st_oop object);
INLINE bool   st_object_is_heap           (st_oop object);
INLINE bool   st_object_is_smi            (st_oop object);

INLINE st_oop st_object_class             (st_oop object);
bool          st_object_equal             (st_oop object, st_oop other);
st_uint       st_object_hash              (st_oop object);

char         *st_object_printString       (st_oop object);


INLINE st_oop
st_object_new (st_space *space, st_oop class)
{  
    return st_descriptors[st_smi_value (ST_BEHAVIOR (class)->format)]->allocate (space, class);
}

INLINE st_oop
st_object_new_arrayed (st_space *space, st_oop class, st_smi size)
{
    return st_descriptors[st_smi_value (ST_BEHAVIOR (class)->format)]->allocate_arrayed (space, class, size);
}

INLINE st_uint
st_object_size (st_oop object)
{
    return st_heap_object_descriptor_for_object (object)->size (object);
}

INLINE st_uint
st_object_copy (st_oop object)
{
    return st_heap_object_descriptor_for_object (object)->copy (object);
}

INLINE void
st_object_contents (st_oop object, struct contents *contents)
{
    st_heap_object_descriptor_for_object (object)->contents (object, contents);
}

INLINE int
st_object_tag (st_oop object)
{
    return object & st_tag_mask;
}

INLINE bool
st_object_is_heap (st_oop object)
{
    return st_object_tag (object) == ST_POINTER_TAG;
}

INLINE bool
st_object_is_smi (st_oop object)
{
    return st_object_tag (object) == ST_SMI_TAG;
}

INLINE bool
st_object_is_character (st_oop object)
{
    return st_object_tag (object) == ST_CHARACTER_TAG;
}

INLINE bool
st_object_is_mark (st_oop object)
{
    return st_object_tag (object) == ST_MARK_TAG;
}

INLINE st_oop
st_object_class (st_oop object)
{
    if (ST_UNLIKELY (st_object_is_smi (object)))
	return st_smi_class;

    if (ST_UNLIKELY (st_object_is_character (object)))
	return st_character_class;

    return st_heap_object_class (object);
}

INLINE bool
st_object_is_symbol (st_oop object)
{
    return st_object_class (object) == st_symbol_class;
}

INLINE bool
st_object_is_string (st_oop object)
{
    return st_object_class (object) == st_string_class;
}

INLINE bool
st_object_is_array (st_oop object)
{
    return st_object_class (object) == st_array_class;
}

INLINE bool
st_object_is_byte_array (st_oop object)
{
    return st_object_class (object) == st_byte_array_class;
}

INLINE bool
st_object_is_float (st_oop object)
{
    return st_object_class (object) == st_float_class;
}

#endif /* __ST_OBJECT_H__ */
