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
#include <st-descriptor.h>

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
    st_size_bits       = 8,
    st_format_bits     = 6,
    st_tag_bits        = 2,

    st_tag_shift       = 0,
    st_format_shift    = st_tag_bits + st_tag_shift,
    st_size_shift      = st_format_bits + st_format_shift,
    st_unused_shift    = st_size_bits + st_size_shift,

    st_format_mask            = ST_NTH_MASK (st_format_bits),
    st_format_mask_in_place   = st_format_mask << st_format_shift,
    st_size_mask              = ST_NTH_MASK (st_size_bits),
    st_size_mask_in_place     = st_size_mask << st_size_shift,
    st_unused_mask            = ST_NTH_MASK (st_unused_bits),
    st_unused_mask_in_place   = st_unused_mask << st_unused_shift,
};

bool           st_object_equal       (st_oop object, st_oop other);
st_uint        st_object_hash        (st_oop object);
char          *st_object_printString (st_oop object);
st_descriptor *st_object_descriptor (void);


static inline void
st_object_set_format (st_oop object, st_format format)
{
    ST_HEADER (object)->mark = (ST_HEADER (object)->mark & ~st_format_mask_in_place) | (format << st_format_shift); 
}

static inline st_format
st_object_format (st_oop object)
{
    return (ST_HEADER (object)->mark >> st_format_shift) & st_format_mask;
}

static inline st_uint
st_object_instance_size (st_oop object)
{
    return (ST_HEADER (object)->mark >> st_size_shift) & st_size_mask;
}

static inline st_uint
st_object_set_instance_size (st_oop object, st_uint size)
{
    st_assert (size <= 255);
    ST_HEADER (object)->mark = (ST_HEADER (object)->mark & ~st_size_mask_in_place) | (size << st_size_shift); 
}

void st_object_initialize_header (st_oop object, st_oop class);

static inline st_oop
st_arrayed_object_size (st_oop object)
{
    return ST_ARRAYED_OBJECT (object)->size;
}

static inline st_descriptor *
st_descriptor_for_object (st_oop object)
{  
    return st_descriptors[st_object_format (object)];
}

static inline st_uint
st_object_size (st_oop object)
{
    return st_descriptor_for_object (object)->size (object);
}

static inline void
st_object_contents (st_oop object, st_oop **oops, st_uint *size)
{
    st_descriptor_for_object (object)->contents (object, oops, size);
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
	return st_smi_class;

    if (ST_UNLIKELY (st_object_is_character (object)))
	return st_character_class;

    return ST_HEADER (object)->class;
}

static inline bool
st_object_is_symbol (st_oop object)
{
    return st_object_class (object) == st_symbol_class;
}

static inline bool
st_object_is_string (st_oop object)
{
    return st_object_class (object) == st_string_class;
}

static inline bool
st_object_is_array (st_oop object)
{
    return st_object_class (object) == st_array_class;
}

static inline bool
st_object_is_byte_array (st_oop object)
{
    return st_object_class (object) == st_byte_array_class;
}

static inline bool
st_object_is_float (st_oop object)
{
    return st_object_class (object) == st_float_class;
}

#endif /* __ST_OBJECT_H__ */
