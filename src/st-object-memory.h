/*
 * st-virtual-space.h
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

#ifndef __ST_OBJECT_MEMORY__
#define __ST_OBJECT_MEMORY__

#include <st-types.h>
#include <st-utils.h>
#include <ptr_array.h>

#define ST_COLLECTION_INTERVAL 10000

struct mark_stack {
    st_oop *stack;
    st_uint sp;
};

typedef struct st_space
{
    st_oop *top;
    st_oop *bottom;

    st_oop *water_level;

    st_uint object_count;
    
} st_space;

typedef struct st_object_memory
{
    st_oop *heap_start, *heap_end;

    st_space *moving_space;
    st_space *fixed_space;

    struct mark_stack ms;

    st_uchar   *mark_bits;
    st_uchar   *alloc_bits;
    st_oop    **offsets;
    
    ptr_array roots;

    st_uint alloc_count;

} st_object_memory;


st_object_memory *st_object_memory_new      (void);
bool              st_object_memory_reserve  (st_object_memory *om, st_uint size);
void              st_object_memory_destroy  (st_object_memory *om);

void              st_object_memory_add_root (st_object_memory *om, st_oop root);

void              begin_gc (st_object_memory *om);

st_space         *st_space_new (st_oop *bottom, st_oop *top);

st_oop            st_space_allocate_object (st_space *space, st_uint size);


#endif /* __ST_OBJECT_MEMORY__ */