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

INLINE st_oop    remap_oop (st_object_memory *om, st_oop ref);

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
#define HEAP_SIZE (500 * 1024  * 1024)

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

    /* fixed space gets 1/3 of heap */
    moving_start = om->heap_start + ((om->heap_end - om->heap_start) / 3);

    om->fixed_space  = st_space_new (om->heap_start, moving_start);
    om->moving_space = st_space_new (moving_start, om->heap_end);

    om->alloc_count = 0;

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

    if (om->alloc_count > ST_COLLECTION_INTERVAL) {
	garbage_collect (om);
	om->alloc_count = 0;
    }

    chunk = space->water_level;
    space->water_level += size;

    space->object_count++;
    om->alloc_count++;

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

    space->object_count = 0;
    
    return space;
}

#define POINTER(oop)       ((st_oop *) ST_POINTER (oop))
#define GET_MARK_BIT(oop)  (get_bit (om->mark_bits,  POINTER (oop) - om->heap_start))
#define SET_MARK_BIT(oop)  (set_bit (om->mark_bits,  POINTER (oop) - om->heap_start))
#define GET_ALLOC_BIT(oop) (get_bit (om->alloc_bits, POINTER (oop) - om->heap_start))
#define SET_ALLOC_BIT(oop) (set_bit (om->alloc_bits, POINTER (oop) - om->heap_start))

#define BLOCK_SIZE       256
#define BLOCK_SIZE_OOPS  (BLOCK_SIZE / sizeof (st_oop))

st_uint objects_freed = 0;

INLINE bool
get_bit (st_uchar *bits, st_ulong index)
{
    return (bits[index >> 3] >> (index & 0x7)) & 1;
}

INLINE void
set_bit (st_uchar *bits, st_ulong index)
{
    bits[index >> 3] |= 1 << (index & 0x7);
}

INLINE void
clear_bits (char *bits, st_ulong size)
{
    memset (bits, 0, size);
}

INLINE void
clear_offsets (st_oop **offsets, st_ulong size)
{
    memset (offsets, 0, sizeof (st_oop *) * size);
}

INLINE st_ulong
get_block_index (st_object_memory *om, st_oop object)
{
    return (POINTER (object) - om->heap_start) / BLOCK_SIZE_OOPS;
}

INLINE st_ulong
bit_index (st_object_memory *om, st_oop object)
{
    return POINTER (object) - om->heap_start;
}

INLINE st_uint
compute_ordinal_number (st_object_memory *om, st_oop ref)
{
    st_ulong b, k;
    st_uint i, ordinal = 0;

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

INLINE st_oop
remap_oop (st_object_memory *om, st_oop ref)
{
    st_ulong b;
    st_uint ordinal, i, j;
    st_oop *offset;

    if (!st_object_is_heap (ref))
	return ref;

    if (POINTER (ref) < om->fixed_space->top)
	return ref;
  
    ordinal = compute_ordinal_number (om, ref); 
    offset = om->offsets[get_block_index (om, ref)];
    b = bit_index (om, ST_OOP (offset));

    for (i = 0, j = 0;; i++) {
	if (get_bit (om->alloc_bits, b + i)) {
	    if (++j == ordinal) {
		return ST_OOP (offset + i);
	    }
	}    
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
    st_uint size;

    /* remap fixed space */
    p = om->fixed_space->bottom;
    while (p < om->fixed_space->water_level) {

	st_assert (st_object_is_mark (*p));

	size = st_object_size (ST_OOP (p));
	st_object_contents (ST_OOP (p), &contents);

	for (st_uint i=0; i < contents.size; i++)
	    contents.oops[i] = remap_oop (om, contents.oops[i]);
	p += size;
    }

    st_oop object;

    /* remap moving space */
    p = om->moving_space->bottom;
    while (p < om->moving_space->water_level) {

	st_assert (st_object_is_mark (*p));

	size = st_object_size (ST_OOP (p));
	st_object_contents (ST_OOP (p), &contents);

	for (st_uint i=0; i < contents.size; i++) {

	    contents.oops[i] = remap_oop (om, contents.oops[i]);
	
	}

	object = ST_OOP (p);
	if (st_heap_object_class (object) == st_method_context_class) {
	    printf ("%lu sender: %lu; %s >> %s\n", object, ST_CONTEXT_PART (object)->sender,
		    st_object_printString (ST_METHOD_CONTEXT (object)->receiver),
		    st_byte_array_bytes (ST_METHOD (ST_METHOD_CONTEXT (object)->method)->selector));
	}
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
    st_space *space;
    st_uint size;
    long block;

    space = om->moving_space;
    p = space->bottom;
    
    block = -1;
    while (GET_MARK_BIT (ST_OOP (p))) {
	SET_ALLOC_BIT (ST_OOP (p));
	if (block < (long) ((p - om->heap_start) / BLOCK_SIZE_OOPS)) {
	    block = (p - om->heap_start) / BLOCK_SIZE_OOPS;
	    om->offsets[block] = p;
	}
	p += st_object_size (ST_OOP (p));
    }
    to = p;

    while (!GET_MARK_BIT (ST_OOP (p)))
	p += st_object_size (ST_OOP (p));
    from = p;

    st_assert (to < from);

    while (from < space->water_level) {
	st_assert (st_object_is_mark (*from));

	if (GET_MARK_BIT (ST_OOP (from))) {

	    SET_ALLOC_BIT (ST_OOP (to));
	    size = st_object_size (ST_OOP (from));
	    st_oops_move (to, from, size);
	    
	    if (block < (long) ((from - om->heap_start) / BLOCK_SIZE_OOPS)) {
		block = (from - om->heap_start) / BLOCK_SIZE_OOPS;
		om->offsets[block] = to;
	    }

	    to += size;
	    from += size;
	} else {
	    from += st_object_size (ST_OOP (from));
	    objects_freed++;
	}
    }

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

	if (GET_MARK_BIT (object))
	    continue;

	SET_MARK_BIT (object);

	om->ms.stack[om->ms.sp++] = st_heap_object_class (object);	
	st_object_contents (object, &contents);
	for (st_smi i=0; i < contents.size; i++)
	    om->ms.stack[om->ms.sp++] = contents.oops[i];

	count++;
    }
}

static void
remap_processor_state (st_object_memory *om, st_processor *processor)
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
    struct timeval before, after;
    double elapsed;
    st_oop context;

    gettimeofday (&before, NULL);

    initialize_metadata (om);
    context = proc->context;
    
    st_object_memory_add_root (om, context);
    mark (om);
    compact (om);
    remap (om);
    remap_processor_state (om, proc);
    st_object_memory_remove_root (om, context);

    //  print_backtrace (proc);

    gettimeofday (&after, NULL);
    elapsed = after.tv_sec - before.tv_sec + (after.tv_usec - before.tv_usec) / 1.e6;
    printf ("time %.9f seconds;\n", elapsed);
}


