/*
 * st-heap-object.c
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

#ifndef __ST_HEAP_OBJECT_H__
#define __ST_HEAP_OBJECT_H__

#include <st-types.h>
#include <st-utils.h>
#include <st-descriptor.h>
#include <st-small-integer.h>

/* Every heap-allocated object starts with this layout */
/* format of mark oop
 * [ unused: 9 | mark: 1 | format: 6 | tag: 2 ]
 *
 * format:      object format
 * mark:        object contains a forwarding pointer
 * unused: 	not used yet (haven't implemented GC)
 * 
 */

struct st_header
{
    st_oop header;
    st_smi hash;
    st_oop class;
    
    st_oop fields[];
};

struct st_arrayed_object
{
    struct st_header header;
    st_oop   size;
};

extern int st_current_hash;

enum
{
    st_unused_bits     = 25,
    st_mark_bits       = 1,
    st_format_bits     = 6,
    st_tag_bits        = 2,

    st_tag_shift       = 0,
    st_format_shift    = st_tag_bits + st_tag_shift,
    st_mark_shift      = st_format_bits + st_format_shift,
    st_unused_shift    = st_mark_bits + st_mark_shift,

    st_format_mask              = ST_NTH_MASK (st_format_bits),
    st_format_mask_in_place     = st_format_mask << st_format_shift,
    st_mark_mask                = ST_NTH_MASK (st_mark_bits),
    st_mark_mask_in_place       = st_mark_mask << st_mark_shift,
    st_unused_mask              = ST_NTH_MASK (st_unused_bits),
    st_unused_mask_in_place     = st_unused_mask << st_unused_shift,
};

#define  st_heap_object_class(oop)  (ST_POINTER (oop)->class)
#define  st_heap_object_body(oop)   (ST_POINTER (oop)->fields)

#define  ST_ARRAYED_OBJECT(oop)           ((struct st_arrayed_object *) ST_POINTER (oop))

void     st_heap_object_initialize_header (st_oop object, st_oop class);
void     st_heap_object_initialize_body   (st_oop object, st_smi instance_size);
st_uint  st_heap_object_hash              (st_oop object);
void     st_heap_object_set_hash          (st_oop object, int hash);
void       st_heap_object_set_format        (st_oop object, st_uint format);
st_uint  st_heap_object_format           (st_oop object);
bool     st_heap_object_marked          (st_oop object);
void     st_heap_object_set_marked      (st_oop object, bool mark);


void st_object_verify (st_oop object);

void     st_heap_object_install_forwarding_pointer (st_oop object, st_oop pointer);
st_oop   st_heap_object_forwarding_pointer         (st_oop object);

st_descriptor *st_heap_object_descriptor (void);


INLINE st_oop
st_arrayed_object_size (st_oop object)
{
    return ST_ARRAYED_OBJECT (object)->size;
}

INLINE st_descriptor *
st_heap_object_descriptor_for_object (st_oop object)
{  
    return st_descriptors[st_heap_object_format (object)];
}

#endif /* __ST_HEAP_OBJECT_H__ */
