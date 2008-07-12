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
#define  ST_ARRAYED_OBJECT(oop)   ((struct st_arrayed_object *) ST_POINTER (oop))

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
    st_oop hash;
    st_oop class;
    st_oop fields[];
};

struct st_arrayed_object
{
    struct st_header header;
    st_oop           size;
};

extern int st_current_hash;

enum
{
    st_unused_bits     = 16,
    st_object_large_bits = 1,
    st_size_bits       = 8,
    st_format_bits     = 6,
    st_tag_bits        = 2,

    st_tag_shift       = 0,
    st_format_shift    = st_tag_bits + st_tag_shift,
    st_size_shift      = st_format_bits + st_format_shift,
    st_object_large_shift     = st_size_bits + st_size_shift,
    st_unused_shift    = st_object_large_bits + st_object_large_shift,

    st_format_mask          = ST_NTH_MASK (st_format_bits),
    st_format_mask_in_place = st_format_mask << st_format_shift,
    st_size_mask            = ST_NTH_MASK (st_size_bits),
    st_size_mask_in_place   = st_size_mask << st_size_shift,
    st_object_large_mask    = ST_NTH_MASK (st_object_large_bits),
    st_object_large_mask_in_place  = st_object_large_mask << st_object_large_shift,
    st_unused_mask          = ST_NTH_MASK (st_unused_bits),
    st_unused_mask_in_place = st_unused_mask << st_unused_shift,
};

typedef enum st_format
{
    ST_FORMAT_OBJECT,
    ST_FORMAT_FLOAT,
    ST_FORMAT_LARGE_INTEGER,
    ST_FORMAT_ARRAY,
    ST_FORMAT_BYTE_ARRAY,
    ST_FORMAT_FLOAT_ARRAY,
    ST_FORMAT_INTEGER_ARRAY,
    ST_FORMAT_WORD_ARRAY,
    ST_FORMAT_CONTEXT,

    ST_NUM_FORMATS
} st_format;


bool           st_object_equal       (st_oop object, st_oop other);
st_uint        st_object_hash        (st_oop object);
char          *st_object_printString (st_oop object);

#define ST_OBJECT_MARK(oop)   (ST_HEADER (oop)->mark)
#define ST_OBJECT_HASH(oop)   (ST_HEADER (oop)->hash)
#define ST_OBJECT_CLASS(oop)  (ST_HEADER (oop)->class)
#define ST_OBJECT_FIELDS(oop) (ST_HEADER (oop)->fields)

st_oop st_object_allocate (st_oop class);

static inline void
st_object_set_format (st_oop object, st_format format)
{
    ST_OBJECT_MARK (object) = (ST_OBJECT_MARK (object) & ~st_format_mask_in_place) | (format << st_format_shift); 
}

static inline st_format
st_object_format (st_oop object)
{
    return (ST_HEADER (object)->mark >> st_format_shift) & st_format_mask;
}

static inline void
st_object_set_large_context (st_oop object, bool is_large)
{
    ST_OBJECT_MARK (object) = (ST_OBJECT_MARK (object) & ~st_object_large_mask_in_place) | (is_large << st_object_large_shift); 
}

static inline st_format
st_object_large_context (st_oop object)
{
    return (ST_OBJECT_MARK (object) >> st_object_large_shift) & st_object_large_mask;
}

static inline st_uint
st_object_instance_size (st_oop object)
{
    return (ST_OBJECT_MARK (object) >> st_size_shift) & st_size_mask;
}

static inline st_uint
st_object_set_instance_size (st_oop object, st_uint size)
{
    ST_OBJECT_MARK (object) = (ST_OBJECT_MARK (object) & ~st_size_mask_in_place) | (size << st_size_shift); 
}

void st_object_initialize_header (st_oop object, st_oop class);

static inline st_oop
st_arrayed_object_size (st_oop object)
{
    return ST_ARRAYED_OBJECT (object)->size;
}

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
