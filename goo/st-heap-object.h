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

/* Every heap-allocated object starts with this layout */
/* format of mark oop
 * [ gc: 9 | non-pointer: 1 | readonly: 1 | hasBeenHashed : 1 | hasHashField: 2 | format: 6 ]
 *
 * gc:          book-keeping for garbage collection
 * unused: 	not used yet (haven't implemented GC)
 * non-pointer:	object body contains native C types
 * readonly:	object cannot be modified
 * 
 */

typedef struct
{
    st_smi header;
    st_smi hash;
    st_oop klass;
    
    st_oop fields[];
} STHeader;


extern int st_current_hash;

enum
{
    st_unused_bits     = 9,
    st_nonpointer_bits = 1,
    st_readonly_bits   = 1,
    st_format_bits     = 6,

    st_format_shift     = 0,
    st_readonly_shift   = st_format_bits + st_format_shift,
    st_nonpointer_shift = st_readonly_bits + st_readonly_shift,
    st_unused_shift     = st_nonpointer_bits + st_nonpointer_shift,

    st_format_mask              = ST_NTH_MASK (st_format_bits),
    st_format_mask_in_place     = st_format_mask << st_format_shift,
    st_readonly_mask            = ST_NTH_MASK (st_readonly_bits),
    st_readonly_mask_in_place   = st_readonly_mask << st_readonly_shift,
    st_nonpointer_mask          = ST_NTH_MASK (st_nonpointer_bits),
    st_nonpointer_mask_in_place = st_nonpointer_mask << st_nonpointer_shift,
    st_unused_mask              = ST_NTH_MASK (st_unused_bits),
    st_unused_mask_in_place     = st_unused_mask << st_unused_shift,
};

#define  st_heap_object_format(oop) (ST_POINTER (oop)->header & st_format_mask)
#define  st_heap_object_class(oop)  (ST_POINTER (oop)->klass)
#define  st_heap_object_body(oop)   (ST_POINTER (oop)->fields)

void     st_heap_object_initialize_header (st_oop object, st_oop klass);

void     st_heap_object_initialize_body   (st_oop object, st_smi instance_size);

guint    st_heap_object_hash              (st_oop object);

void     st_heap_object_set_hash          (st_oop object, int hash);

void     st_heap_object_set_format        (st_oop object, guint format);

bool     st_heap_object_readonly          (st_oop object);

void     st_heap_object_set_readonly      (st_oop object, bool readonly);

bool     st_heap_object_nonpointer        (st_oop object);

void     st_heap_object_set_nonpointer    (st_oop object, bool nonpointer);

const STDescriptor *st_heap_object_descriptor (void) G_GNUC_CONST;

#endif /* __ST_HEAP_OBJECT_H__ */
