/*
 * st-object-memory.c
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

#include "st-object-memory.h"
#include "st-types.h"
#include "st-utils.h"
#include "st-descriptor.h"
#include "st-heap-object.h"
#include "st-object.h"
#include "st-array.h"
#include "st-context.h"
#include "st-method.h"
#include "st-association.h"
#include "st-byte-array.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

static inline st_oop    remap_oop (st_object_memory *om, st_oop ref);

static void      garbage_collect (st_object_memory *om);


// adjust reserved size to be a integral multiple of the page size
static st_uint
adjust_size (int reserved_size)
{
    int page_size = getpagesize ();

    if (reserved_size % page_size != 0) {
	return (reserved_size / page_size) * page_size + page_size;
    }

    return reserved_size;
}

static bool
map_size (st_object_memory *om, st_uint size)
{
    void *start;

    st_uint adjusted_size = adjust_size (size);

    start = mmap (NULL, adjusted_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if ((intptr_t) start == -1) {
	fprintf (stderr, strerror (errno));
	return false;
    }
    
    om->heap_start = (st_oop *) start;
    om->heap_end = ((st_oop *) start) + adjusted_size / SIZEOF_VOID_P;
    
    return true;
}

bool
st_object_memory_reserve (st_object_memory *om, st_uint size)
{
    return map_size (om, size);
}

// RESERVE 500 MB worth of virtual address space
#define HEAP_SIZE (100 * 1024  * 1024)

st_object_memory *
st_object_memory_new (void)
{
    st_object_memory *om;
    st_oop *moving_start;
    st_ulong size_bits;

    om = st_new0 (st_object_memory);
    
    om->roots = ptr_array_new (25);
    
    if (!st_object_memory_reserve (om, HEAP_SIZE))
	abort ();

    /* fixed space gets 1/5 of heap */
    moving_start = om->heap_start + ((om->heap_end - om->heap_start) / 5 );

    om->fixed_space  = st_space_new (om->heap_start, moving_start);
    om->moving_space = st_space_new (moving_start, om->heap_end);

    om->byte_count = 0;

    om->tot_pause_time.tv_sec = 0;
    om->tot_pause_time.tv_nsec = 0;

    return om;
}

void
st_object_memory_destroy (st_object_memory *om)
{
    st_free (om);
}

void
st_object_memory_add_root (st_object_memory *om, st_oop root)
{
    ptr_array_append (om->roots, (st_pointer) root);
}

void
st_object_memory_remove_root (st_object_memory *om, st_oop root)
{
    ptr_array_remove_fast (om->roots, (st_pointer) root);
}

st_oop
st_space_allocate_chunk (st_space *space, st_uint size)
{
    st_oop *chunk;

    st_assert (size >= 2);

    if (space == om->moving_space && om->byte_count > ST_COLLECTION_THRESHOLD) {
	garbage_collect (om);
	om->byte_count = 0;
    }

    chunk = space->water_level;
    space->water_level += size;

    om->byte_count += (size * sizeof (st_oop));

    om->bytes_allocated += (size * sizeof (st_oop));

    return ST_OOP (chunk);
}

st_oop
st_space_allocate_object (st_space *space,
			  st_oop    class,
			  st_uint   size)
{
    st_oop chunk;
    
    chunk = st_space_allocate_chunk (space, size);

    ST_POINTER (chunk)->header = 0 | ST_MARK_TAG;
    ST_POINTER (chunk)->hash  = st_current_hash++;
    ST_POINTER (chunk)->class = class;

    st_heap_object_set_format (chunk, st_smi_value (ST_BEHAVIOR (class)->format));
    st_heap_object_set_marked (chunk, false);

    return chunk;
}

st_space *
st_space_new (st_oop *bottom, st_oop *top)
{
    st_space *space;

    space = st_new0 (st_space);

    space->bottom = bottom;
    space->top = top;
    space->water_level = bottom;
    
    return space;
}


#define BLOCK_SIZE       256
#define BLOCK_SIZE_OOPS  (BLOCK_SIZE / sizeof (st_oop))

st_uint objects_freed = 0;

static inline st_oop
tag (st_oop *ptr)
{
    return ST_OOP (ptr);
}

static inline st_oop *
detag (st_oop oop)
{
    return (st_oop *) ST_POINTER (oop);
}

static inline bool
get_bit (st_uchar *bits, st_ulong index)
{
    return (bits[index >> 3] >> (index & 0x7)) & 1;
}

static inline void
set_bit (st_uchar *bits, st_ulong index)
{
    bits[index >> 3] |= 1 << (index & 0x7);
}

static inline st_ulong
bit_index (st_object_memory *om, st_oop object)
{
    return detag (object) - om->heap_start;
}

static inline bool
get_mark_bit (st_object_memory *om, st_oop object)
{
    return get_bit (om->mark_bits,  bit_index (om, object));
}

static inline void
set_mark_bit (st_object_memory *om, st_oop object)
{
    set_bit (om->mark_bits,  bit_index (om, object));
}

static inline bool
get_alloc_bit (st_object_memory *om, st_oop object)
{
    return get_bit (om->alloc_bits,  bit_index (om, object));
}

static inline void
set_alloc_bit (st_object_memory *om, st_oop object)
{
    set_bit (om->alloc_bits,  bit_index (om, object));
}

static inline void
clear_bits (char *bits, st_ulong size)
{
    memset (bits, 0, size);
}

static inline void
clear_offsets (st_oop **offsets, st_ulong size)
{
    memset (offsets, 0, sizeof (st_oop *) * size);
}

static inline st_ulong
get_block_index (st_object_memory *om, st_oop object)
{
    return (detag (object) - om->heap_start) / BLOCK_SIZE_OOPS;
}

static inline st_uint
compute_ordinal_number (st_object_memory *om, st_oop ref)
{
    st_uint i, ordinal = 0;
    st_ulong b, k;

    b = bit_index (om, ref) & ~(BLOCK_SIZE_OOPS - 1);
    k = bit_index (om, ref);
    
    for (i = 0; i < BLOCK_SIZE_OOPS; i++) {
	if (get_bit (om->mark_bits, b + i)) {
	    ordinal++;
	    if ((b + i) == k)
		return ordinal;
	}
    }

    st_assert_not_reached ();
    return 0;
}

static inline st_oop
remap_oop (st_object_memory *om, st_oop ref)
{
    st_ulong b;
    st_uint ordinal, i = 0, j = 0;
    st_oop *offset;

    if (!st_object_is_heap (ref))
	return ref;

    if (detag (ref) < om->fixed_space->top)
	return ref;
  
    ordinal = compute_ordinal_number (om, ref); 
    offset  = om->offsets[get_block_index (om, ref)];
    b = bit_index (om, tag (offset));

    while (true) {
	if (get_bit (om->alloc_bits, b + i)) {
	    if (++j == ordinal) {
		return tag (offset + i);
	    }
	}
	i++;
    }

    st_assert_not_reached ();
    return 0;
}

static void
remap (st_object_memory *om)
{
    struct contents contents;
    st_space *space;
    st_oop *p;
    st_uint size, i;

    /* remap fixed space */
    p = om->fixed_space->bottom;
    while (p < om->fixed_space->water_level) {

	size = st_object_size (tag (p));
	st_object_contents (tag (p), &contents);

	for (i = 0; i < contents.size; i++)
	    contents.oops[i] = remap_oop (om, contents.oops[i]);

	p += size;
    }

    /* remap moving space */
    p = om->moving_space->bottom;
    while (p < om->moving_space->water_level) {

	size = st_object_size (tag (p));
	st_object_contents (tag (p), &contents);

	for (i = 0; i < contents.size; i++)
	    contents.oops[i] = remap_oop (om, contents.oops[i]);
	
	p += size;
    }

}

static void
initialize_metadata (st_object_memory *om)
{
    st_ulong bit_vector_size, size;
    st_ulong block_count; 

    size = om->moving_space->water_level - om->heap_start;
    bit_vector_size = (size / 8) + 1;
    block_count = size / BLOCK_SIZE_OOPS;

    om->mark_bits  = (st_uchar *) om->moving_space->water_level;
    clear_bits (om->mark_bits, bit_vector_size);
    
    om->alloc_bits = om->mark_bits + bit_vector_size;
    clear_bits (om->alloc_bits, bit_vector_size);

    om->offsets = (st_oop **) (om->alloc_bits + bit_vector_size);
    clear_offsets (om->offsets, block_count);

    om->ms.stack = (st_oop *) (om->offsets + block_count);
    om->ms.sp = 0;
}

void
compact (st_object_memory *om)
{
    st_oop *p, *from, *to;
    st_space *space = om->moving_space;
    st_uint  size;
    long block = -1;

    p = space->bottom;
    
    while (get_mark_bit (om, tag (p))) {
	set_alloc_bit (om, tag (p));
	if (block <  (long) get_block_index (om, tag (p))) {
	    block = get_block_index (om, tag (p));
	    om->offsets[block] = p;
	}
	p += st_object_size (tag (p));
    }
    to = p;

    while (!get_mark_bit (om, tag (p)) && p < space->water_level)
	p += st_object_size (tag (p));
    from = p;

    st_assert (to < from);

    while (from < space->water_level) {
	st_assert (st_object_is_mark (*from));

	if (get_mark_bit (om, tag (from))) {

	    set_alloc_bit (om, tag (to));
	    size = st_object_size (tag (from));
	    st_oops_move (to, from, size);
	    
	    if (block < (long) get_block_index (om, tag (from))) {
		block = get_block_index (om, tag (from));
		om->offsets[block] = to;
	    }
	    to += size;
	    from += size;
	} else {
	    from += st_object_size (tag (from));
	}
    }

    om->bytes_collected = (space->water_level - to) * sizeof (st_oop);
    om->bytes_allocated -= om->bytes_collected;

    space->water_level = to;
}

static void
add_roots_to_stack (st_object_memory *om, struct mark_stack *ms) 
{
    st_uint i = 0;

    for (; i < om->roots->length; i++)
	ms->stack[i] = (st_oop) ptr_array_get_index (om->roots, i);

    ms->sp += i;
}

static st_uint count = 0;

static void
mark (st_object_memory *om)
{
    struct contents contents;
    st_oop  object;
    st_oop *oops;
    st_smi  size;
    
    add_roots_to_stack (om, &(om->ms));

    while (om->ms.sp > 0) {
	
	object = om->ms.stack[--om->ms.sp];
	
	if (!st_object_is_heap (object)) 
	    continue;

	if (get_mark_bit (om, object))
	    continue;

	set_mark_bit (om, object);

	om->ms.stack[om->ms.sp++] = st_heap_object_class (object);	
	st_object_contents (object, &contents);
	for (st_smi i=0; i < contents.size; i++)
	    om->ms.stack[om->ms.sp++] = contents.oops[i];

	count++;
    }
}

static void
synchronize_processor (st_object_memory *om, st_processor *processor)
{
    st_oop context;
    st_oop home;

    context = remap_oop (om, processor->context);

    st_assert (st_object_is_mark (ST_POINTER (context)->header));

    if (st_heap_object_class (context) == st_block_context_class) {

	home = ST_BLOCK_CONTEXT (context)->home;

	processor->method   = ST_METHOD_CONTEXT (home)->method;
	processor->receiver = ST_METHOD_CONTEXT (home)->receiver;
	processor->literals = st_array_elements (ST_METHOD (processor->method)->literals);
	processor->temps    = ST_METHOD_CONTEXT_TEMPORARY_FRAME (home);
	processor->stack    = ST_BLOCK_CONTEXT (context)->stack;
    } else {
	processor->method   = ST_METHOD_CONTEXT (context)->method;
	processor->receiver = ST_METHOD_CONTEXT (context)->receiver;
	processor->literals = st_array_elements (ST_METHOD (processor->method)->literals);
	processor->temps    = ST_METHOD_CONTEXT_TEMPORARY_FRAME (context);
	processor->stack    = ST_METHOD_CONTEXT_STACK (context);
    }

    processor->context  = context;
    processor->bytecode = st_method_bytecode_bytes (processor->method);
}

static void
garbage_collect (st_object_memory *om)
{
    struct timespec start, end, interval;
    st_oop context;
    
    initialize_metadata (om);
    context = proc->context;
    
    st_object_memory_add_root (om, context);

    clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &start);

    mark (om);
    compact (om);
    remap (om);
    synchronize_processor (om, proc);

    clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &end);
    st_timespec_difference (&start, &end, &interval);
    st_timespec_add (&om->tot_pause_time, &interval ,&om->tot_pause_time);

    if (st_verbose_mode ()) {
	fprintf (stderr, "** gc: %uK->%uK (%uK), %.6fs\n",
		 (om->bytes_collected + om->bytes_allocated) / 1024,
		 om->bytes_allocated / 1024,
		 om->bytes_collected / 1024,
		 st_timespec_to_double_seconds (&interval));
    }
    
    st_object_memory_remove_root (om, context);
}


