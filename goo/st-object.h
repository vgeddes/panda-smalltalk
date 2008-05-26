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
#include <st-class.h>
#include <st-universe.h>
#include <glib.h>


st_oop        st_object_new               (st_oop klass);
st_oop        st_object_new_arrayed       (st_oop klass, st_smi size);

INLINE int    st_object_tag               (st_oop object);
INLINE bool   st_object_is_heap           (st_oop object);
INLINE bool   st_object_is_smi            (st_oop object);

INLINE st_oop st_object_class             (st_oop object);
bool          st_object_equal             (st_oop object, st_oop other);
guint         st_object_hash              (st_oop object);


char       *st_object_printString         (st_oop object);

/* inline definitions */

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

INLINE st_oop
st_object_class (st_oop object)
{
    if (G_UNLIKELY (st_object_is_smi (object)))
	return st_smi_class;

    if (G_UNLIKELY (st_object_is_character (object)))
	return st_character_class;

    return st_heap_object_class (object);
}

INLINE const STDescriptor *
st_object_descriptor (st_oop object)
{
    if (G_UNLIKELY (st_object_is_smi (object)))
	g_assert_not_reached ();

    if (G_UNLIKELY (st_object_is_smi (object)))
	g_assert_not_reached ();
    
    return st_descriptors[st_heap_object_format (object)];
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
