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
#include <st-universe.h>

#define  ST_HEADER(oop)           ((struct st_header *) ST_POINTER (oop))

/* Every heap-allocated object starts with this header word */
/* format of mark oop
 * [ unused: 16 | instance-size: 8 | format: 6 | tag: 2 ]
 *
 *
 * format:      object format
 * mark:        object contains a forwarding pointer
 * unused: 	not used yet (haven't implemented GC)
 * 
 */
struct st_header
{
    st_oop mark;
    st_oop class;
    st_oop fields[];
};

#define _ST_OBJECT_SET_BITFIELD(bitfield, field, value) 	  		\
    (((bitfield) & ~(_ST_OBJECT_##field##_MASK << _ST_OBJECT_##field##_SHIFT)) \
     | (((value) & _ST_OBJECT_##field##_MASK) << _ST_OBJECT_##field##_SHIFT))

#define _ST_OBJECT_GET_BITFIELD(bitfield, field)			\
    (((bitfield) >> _ST_OBJECT_##field##_SHIFT) & _ST_OBJECT_##field##_MASK)

enum
{
    _ST_OBJECT_UNUSED_BITS     = 15,
    _ST_OBJECT_HASH_BITS       = 1,
    _ST_OBJECT_LARGE_BITS      = 1,
    _ST_OBJECT_SIZE_BITS       = 8,
    _ST_OBJECT_FORMAT_BITS     = 6,

    _ST_OBJECT_FORMAT_SHIFT    =  ST_TAG_SIZE,
    _ST_OBJECT_SIZE_SHIFT      = _ST_OBJECT_FORMAT_BITS + _ST_OBJECT_FORMAT_SHIFT,
    _ST_OBJECT_LARGE_SHIFT     = _ST_OBJECT_SIZE_BITS   + _ST_OBJECT_SIZE_SHIFT,
    _ST_OBJECT_HASH_SHIFT      = _ST_OBJECT_LARGE_BITS  + _ST_OBJECT_LARGE_SHIFT,
    _ST_OBJECT_UNUSED_SHIFT    = _ST_OBJECT_HASH_BITS   + _ST_OBJECT_HASH_SHIFT,

    _ST_OBJECT_FORMAT_MASK     = ST_NTH_MASK (_ST_OBJECT_FORMAT_BITS),
    _ST_OBJECT_SIZE_MASK       = ST_NTH_MASK (_ST_OBJECT_SIZE_BITS),
    _ST_OBJECT_LARGE_MASK      = ST_NTH_MASK (_ST_OBJECT_LARGE_BITS),
    _ST_OBJECT_HASH_MASK       = ST_NTH_MASK (_ST_OBJECT_HASH_BITS),
    _ST_OBJECT_UNUSED_MASK     = ST_NTH_MASK (_ST_OBJECT_UNUSED_BITS),
};

/* Make sure to update all cased code in VM when adding a new format */
typedef enum st_format
{
    ST_FORMAT_OBJECT,
    ST_FORMAT_FLOAT,
    ST_FORMAT_LARGE_INTEGER,
    ST_FORMAT_HANDLE,
    ST_FORMAT_ARRAY,
    ST_FORMAT_BYTE_ARRAY,
    ST_FORMAT_FLOAT_ARRAY,
    ST_FORMAT_INTEGER_ARRAY,
    ST_FORMAT_WORD_ARRAY,
    ST_FORMAT_CONTEXT,

    ST_NUM_FORMATS
} st_format;


bool    st_object_equal       (st_oop object, st_oop other);
st_uint st_object_hash        (st_oop object);

#define ST_OBJECT_MARK(oop)   (ST_HEADER (oop)->mark)
#define ST_OBJECT_CLASS(oop)  (ST_HEADER (oop)->class)
#define ST_OBJECT_FIELDS(oop) (ST_HEADER (oop)->fields)

st_oop st_object_allocate (st_oop class);


static inline void
st_object_set_format (st_oop object, st_format format)
{
    ST_OBJECT_MARK (object) = _ST_OBJECT_SET_BITFIELD (ST_OBJECT_MARK (object), FORMAT, format);
}

static inline st_format
st_object_format (st_oop object)
{
    return _ST_OBJECT_GET_BITFIELD (ST_OBJECT_MARK (object), FORMAT);
}

static inline void
st_object_set_large_context (st_oop object, bool is_large)
{
    ST_OBJECT_MARK (object) = _ST_OBJECT_SET_BITFIELD (ST_OBJECT_MARK (object), LARGE, is_large);
}

static inline bool
st_object_large_context (st_oop object)
{
    return _ST_OBJECT_GET_BITFIELD (ST_OBJECT_MARK (object), LARGE);
}

static inline void
st_object_set_hashed (st_oop object, bool hashed)
{
    ST_OBJECT_MARK (object) = _ST_OBJECT_SET_BITFIELD (ST_OBJECT_MARK (object), HASH, hashed);
}

static inline bool
st_object_is_hashed (st_oop object)
{
    return _ST_OBJECT_GET_BITFIELD (ST_OBJECT_MARK (object), HASH);
}


static inline st_uint
st_object_instance_size (st_oop object)
{
    return _ST_OBJECT_GET_BITFIELD (ST_OBJECT_MARK (object), SIZE);
}

static inline st_uint
st_object_set_instance_size (st_oop object, st_uint size)
{
    ST_OBJECT_MARK (object) = _ST_OBJECT_SET_BITFIELD (ST_OBJECT_MARK (object), SIZE, size);
}

void st_object_initialize_header (st_oop object, st_oop class);

static inline int
st_object_tag (st_oop object)
{
    return object & st_tag_mask;
}

static inline bool
st_object_is_heap (st_oop object)
{
    return st_object_tag (object) == ST_POINTER_TAG;
}

static inline bool
st_object_is_smi (st_oop object)
{
    return st_object_tag (object) == ST_SMI_TAG;
}

static inline bool
st_object_is_character (st_oop object)
{
    return st_object_tag (object) == ST_CHARACTER_TAG;
}

static inline bool
st_object_is_mark (st_oop object)
{
    return st_object_tag (object) == ST_MARK_TAG;
}

static inline st_oop
st_object_class (st_oop object)
{
    if (ST_UNLIKELY (st_object_is_smi (object)))
	return ST_SMI_CLASS;

    if (ST_UNLIKELY (st_object_is_character (object)))
	return ST_CHARACTER_CLASS;

    return ST_OBJECT_CLASS (object);
}

static inline bool
st_object_is_symbol (st_oop object)
{
    return st_object_class (object) == ST_SYMBOL_CLASS;
}

static inline bool
st_object_is_string (st_oop object)
{
    return st_object_class (object) == ST_STRING_CLASS;
}

static inline bool
st_object_is_array (st_oop object)
{
    return st_object_class (object) == ST_ARRAY_CLASS;
}

static inline bool
st_object_is_byte_array (st_oop object)
{
    return st_object_class (object) == ST_BYTE_ARRAY_CLASS;
}

static inline bool
st_object_is_float (st_oop object)
{
    return st_object_class (object) == ST_FLOAT_CLASS;
}

#endif /* __ST_OBJECT_H__ */
