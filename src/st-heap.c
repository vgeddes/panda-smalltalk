/*
 * st-heap.c
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


#include "st-heap.h"
#include "st-utils.h"
#include "st-system.h"


#define PAGE_SIZE (st_system_pagesize ())

static inline st_uint
round_pagesize (st_uint size)
{
    return ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
}

st_heap *
st_heap_new (st_uint reserved_size)
{
    /* Create a new heap with a reserved address space.
     * Returns NULL if address space could not be reserved
     */
    st_pointer result;
    st_heap   *heap;
    st_uint    size;

    st_assert (reserved_size > 0);
    size = round_pagesize (reserved_size);

    result = st_system_reserve_memory (NULL, size);
    if (result == NULL)
	return NULL;

    heap = st_new0 (st_heap);

    heap->start = result;
    heap->end = result + size;
    heap->p = result;
    
    return heap;
}

bool
st_heap_grow (st_heap *heap, st_uint grow_size)
{
    /* Grows the heap by the specified amount (in bytes).
     * The primitive will not succeed if the heap runs out
     * of reserved address space.
     */
    st_pointer result;
    st_uint    size;

    st_assert (grow_size > 0);
    size = round_pagesize (grow_size);

    if ((heap->p + size) >= heap->end)
	return false;

    result = st_system_commit_memory (heap->p, size);
    if (result == NULL)
	return false;

    heap->p += size;

    return true;
}

bool
st_heap_shrink (st_heap *heap, st_uint shrink_size)
{
    /* Shrinks the heap by the specified amount (in bytes).
     */
    st_pointer result;
    st_uint    size;
    
    st_assert (shrink_size > 0);
    size = round_pagesize (shrink_size);
    
    if ((heap->p - size) < heap->start)
	return false;
    
    result = st_system_decommit_memory (heap->p - size, size);
    if (result == NULL)
	return false;

    heap->p -= size;

    return true;
}

void
st_heap_destroy (st_heap *heap)
{
    st_system_release_memory (heap->start, heap->end - heap->start);
    st_free (heap);
}
