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

#ifndef __ST_MEMORY__
#define __ST_MEMORY__

#include <st-types.h>
#include <st-utils.h>
#include <ptr_array.h>

/* threshold is 8 Mb or 16 Mb depending on whether system is 32 or 64 bits */
#define ST_COLLECTION_THRESHOLD (sizeof (st_oop) * 2 * 1024 * 1024)

#define RESERVED_SIZE        (1000 * 1024 * 1024)
#define INITIAL_COMMIT_SIZE  (64 * 1024 * 1024)

typedef struct st_heap
{
    st_uchar *start; /* start of reserved address space */
    st_uchar *p;     /* end of committed address space (start to p is thus writeable memory) */
    st_uchar *end;   /* end of reserved address space */
 
} st_heap;

typedef struct st_memory
{
    st_heap   *heap;

    st_oop    *start, *end;
    st_oop    *p;

    st_oop    *mark_stack;

    st_uchar  *mark_bits;
    st_uchar  *alloc_bits;
    st_uint    bits_size; /* in bytes */

    st_oop   **offsets;
    st_uint    offsets_size; /* in bytes */

    ptr_array  roots;
    st_uint    counter;

    /* statistics */
    struct timespec total_pause_time;     /* total accumulated pause time */
    st_ulong bytes_allocated;             /* current number of allocated bytes */
    st_ulong bytes_collected;             /* number of bytes collected in last compaction */

} st_memory;

st_memory *st_memory_new             (void);
void       st_memory_destroy         (void);
void       st_memory_add_root        (st_oop object);
void       st_memory_remove_root     (st_oop object);
st_oop     st_memory_allocate        (st_uint size);

st_heap  *st_heap_new       (st_uint reserved_size);
bool      st_heap_grow      (st_heap *heap, st_uint grow_size);
bool      st_heap_shrink    (st_heap *heap, st_uint shrink_size);
void      st_heap_destroy   (st_heap *heap);

#endif /* __ST_MEMORY__ */