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

#include "st-memory.h"
#include "st-types.h"
#include "st-utils.h"
#include "st-descriptor.h"
#include "st-object.h"
#include "st-behavior.h"
#include "st-array.h"
#include "st-context.h"
#include "st-method.h"
#include "st-association.h"
#include "st-byte-array.h"
#include "st-system.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

static inline st_oop    remap_oop (st_memory *om, st_oop ref);

static void      garbage_collect (st_memory *memory);

// RESERVE 256 MB worth of virtual address space
#define RESERVED_SIZE        (256 * 1024 * 1024)
#define INITIAL_COMMIT_SIZE  (128 * 1024 * 1024)

// 1 MB for mark stack 
#define MARK_STACK_SIZE      (1 * 1024 * 1024)

#define BLOCK_SIZE       256
#define BLOCK_SIZE_OOPS  (BLOCK_SIZE / sizeof (st_oop))

static inline st_uint
round_pagesize (st_uint size)
{
    return ((size + st_system_pagesize () - 1) / st_system_pagesize ()) * st_system_pagesize ();
}

static void
initialize_mark_stack (struct mark_stack *ms)
{
    st_pointer result;

    result = st_system_commit_memory (NULL, MARK_STACK_SIZE);
    if (!result)
	abort ();
    
    ms->address = result;
    ms->size = MARK_STACK_SIZE;

    ms->stack = (st_oop *) ms->address;
    ms->sp = 0;
}

static void
initialize_metadata (st_memory *memory)
{
    /* The bit arrays are implemented using bytes as smallest element of storage.
     * If there are N oops in the heap, then we need ((N + 7) / 8) bytes
     * for a bit array with 32 elements. We need to reserve space for
     * two bit arrays (mark, alloc), as well as the offsets array.
     */
    st_uint   size, bits_size, offsets_size;
    st_uchar *metadata_start;

    size = memory->heap_end - memory->heap_start;
    bits_size    = ((size + 7) / 8);
    offsets_size = (size / BLOCK_SIZE_OOPS) * sizeof (st_oop *);
   
    memory->mark_bits  = st_malloc (bits_size); 
    memory->alloc_bits = st_malloc (bits_size); 
    memory->bits_size  = bits_size;

    memory->offsets      = st_malloc (offsets_size);
    memory->offsets_size = offsets_size;
}

st_memory *
st_memory_new (void)
{
    st_memory *memory;
    st_oop    *ptr;
    st_ulong   size_bits;
    st_heap   *heap;

    heap = st_heap_new (RESERVED_SIZE);
    if (!heap)
	abort ();

    if (!st_heap_grow (heap, INITIAL_COMMIT_SIZE))
	abort ();

    memory = st_new0 (st_memory);
    
    memory->heap = heap;
    memory->heap_start = (st_oop *) heap->start;
    memory->heap_end  = (st_oop *) heap->p;

    memory->roots = ptr_array_new (25);

    memory->tot_pause_time.tv_sec = 0;
    memory->tot_pause_time.tv_nsec = 0;
    memory->byte_count = 0;

    initialize_mark_stack (&memory->ms);
    initialize_metadata (memory);

    /* fixed space gets 1/5 of heap, 4/5 for the moving space */    
    ptr = memory->heap_start + ((memory->heap_end - memory->heap_start) / 5) ;
    memory->fixed_space  = st_space_new (memory->heap_start, ptr);
    memory->moving_space = st_space_new (ptr, memory->heap_end);

    return memory;
}

void
st_memory_destroy (st_memory *memory)
{
    st_free (memory);
}

void
st_memory_add_root (st_memory *memory, st_oop root)
{
    ptr_array_append (memory->roots, (st_pointer) root);
}

void
st_memory_remove_root (st_memory *memory, st_oop root)
{
    ptr_array_remove_fast (memory->roots, (st_pointer) root);
}

st_oop
st_space_allocate_chunk (st_space *space, st_uint size)
{
    st_oop *chunk;

    st_assert (size >= 2);

    if (space == memory->moving_space
	&& memory->byte_count > ST_COLLECTION_THRESHOLD) {
	garbage_collect (memory);
	memory->byte_count = 0;
    }

   if (space == memory->moving_space &&
       (space->water_level + size) >= space->top) {
       abort ();
    }

    chunk = space->water_level;
    space->water_level += size;

    memory->byte_count += (size * sizeof (st_oop));

    memory->bytes_allocated += (size * sizeof (st_oop));

    return ST_OOP (chunk);
}

st_oop
st_space_allocate_object (st_space *space,
			  st_oop    class,
			  st_uint   size)
{
    st_oop chunk;
    
    chunk = st_space_allocate_chunk (space, size);

    ST_HEADER (chunk)->mark = 0 | ST_MARK_TAG;
    ST_HEADER (chunk)->hash  = st_smi_new (st_current_hash++);
    ST_HEADER (chunk)->class = class;
    st_object_set_format (chunk, st_smi_value (ST_BEHAVIOR (class)->format));

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
bit_index (st_memory *memory, st_oop object)
{
    return detag (object) - memory->heap_start;
}

static inline bool
get_mark_bit (st_memory *memory, st_oop object)
{
    return get_bit (memory->mark_bits,  bit_index (memory, object));
}

static inline void
set_mark_bit (st_memory *memory, st_oop object)
{
    set_bit (memory->mark_bits,  bit_index (memory, object));
}

static inline bool
get_alloc_bit (st_memory *memory, st_oop object)
{
    return get_bit (memory->alloc_bits,  bit_index (memory, object));
}

static inline void
set_alloc_bit (st_memory *memory, st_oop object)
{
    set_bit (memory->alloc_bits,  bit_index (memory, object));
}

static inline void
clear_offsets (st_oop **offsets, st_ulong size)
{
    memset (offsets, 0, sizeof (st_oop *) * size);
}

static inline st_ulong
get_block_index (st_memory *memory, st_oop object)
{
    return (detag (object) - memory->heap_start) / BLOCK_SIZE_OOPS;
}

static inline st_uint
compute_ordinal_number (st_memory *memory, st_oop ref)
{
    st_uint i, ordinal = 0;
    st_ulong b, k;

    b = bit_index (memory, ref) & ~(BLOCK_SIZE_OOPS - 1);
    k = bit_index (memory, ref);
    
    for (i = 0; i < BLOCK_SIZE_OOPS; i++) {
	if (get_bit (memory->mark_bits, b + i)) {
	    ordinal++;
	    if ((b + i) == k)
		return ordinal;
	}
    }

    st_assert_not_reached ();
    return 0;
}

static inline st_oop
remap_oop (st_memory *memory, st_oop ref)
{
    st_ulong b;
    st_uint ordinal, i = 0, j = 0;
    st_oop *offset;

    if (!st_object_is_heap (ref))
	return ref;

    if (detag (ref) < memory->fixed_space->top)
	return ref;
  
    ordinal = compute_ordinal_number (memory, ref); 
    offset  = memory->offsets[get_block_index (memory, ref)];
    b = bit_index (memory, tag (offset));

    while (true) {
	if (get_bit (memory->alloc_bits, b + i)) {
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
remap (st_memory *memory)
{
    struct contents contents;
    st_space *space;
    st_oop *p;
    st_uint size, i;

    /* remap fixed space */
    p = memory->fixed_space->bottom;
    while (p < memory->fixed_space->water_level) {

	size = st_object_size (tag (p));
	st_object_contents (tag (p), &contents);
	
	for (i = 0; i < contents.size; i++)
	    contents.oops[i] = remap_oop (memory, contents.oops[i]);
	
	p += size;
    }
    
    st_oop method;

    /* remap moving space */
    p = memory->moving_space->bottom;
    while (p < memory->moving_space->water_level) {

	size = st_object_size (tag (p));
	st_object_contents (tag (p), &contents);

	for (i = 0; i < contents.size; i++) {
	    contents.oops[i] = remap_oop (memory, contents.oops[i]);
	}
	p += size;
    }

}

static void
compact (st_memory *memory)
{
    st_oop *p, *from, *to;
    st_space *space = memory->moving_space;
    st_uint  size;
    long block = -1;

    p = space->bottom;
    
    while (get_mark_bit (memory, tag (p))) {
	set_alloc_bit (memory, tag (p));
	if (block <  (long) get_block_index (memory, tag (p))) {
	    block = get_block_index (memory, tag (p));
	    memory->offsets[block] = p;
	}
	p += st_object_size (tag (p));
    }
    to = p;

    while (!get_mark_bit (memory, tag (p)) && p < space->water_level) {
	st_assert (st_object_is_mark (*p));
	p += st_object_size (tag (p));
    }
    from = p;

    if (from == to && p >= space->water_level) {
	goto out;
    }

    st_assert (to < from);
    st_assert (p < space->water_level);

    while (from < space->water_level) {

	if (get_mark_bit (memory, tag (from))) {

	    set_alloc_bit (memory, tag (to));
	    size = st_object_size (tag (from));
	    st_oops_move (to, from, size);
	    
	    if (block < (long) get_block_index (memory, tag (from))) {
		block = get_block_index (memory, tag (from));
		memory->offsets[block] = to;
	    }
	    to += size;
	    from += size;
	} else {
	    from += st_object_size (tag (from));
	}
    }

out:

    memory->bytes_collected = (space->water_level - to) * sizeof (st_oop);
    memory->bytes_allocated -= memory->bytes_collected;

    space->water_level = to; 
}

static void
add_roots_to_stack (st_memory *memory, struct mark_stack *ms) 
{
    st_uint i = 0;

    for (; i < memory->roots->length; i++)
	ms->stack[i] = (st_oop) ptr_array_get_index (memory->roots, i);
    ms->sp += i;

    ms->stack[memory->ms.sp++] = proc->context;
}

static void
mark (st_memory *memory)
{
    struct contents contents;
    st_oop  object;
    st_oop *oops;
    st_smi  size;

    add_roots_to_stack (memory, &(memory->ms));

    st_oop *stack = memory->ms.stack;
    st_uint sp = memory->ms.sp;

    while (sp > 0) {
	
	object = stack[--sp];
	
	if (!st_object_is_heap (object)) 
	    continue;

	if (get_mark_bit (memory, object))
	    continue;

	set_mark_bit (memory, object);

	stack[sp++] = ST_HEADER (object)->class;	
	st_object_contents (object, &contents);
	for (st_smi i=0; i < contents.size; i++) {
	    if (contents.oops[i] != st_nil)
		stack[sp++] = contents.oops[i];
	}
    }
}

static void
synchronize_processor (st_memory *memory, st_processor *processor)
{
    st_oop context;
    st_oop home;

    context = remap_oop (memory, processor->context);

    st_assert (st_object_is_mark (ST_HEADER (context)->mark));

    if (ST_HEADER (context)->class == st_block_context_class) {

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

    processor->message_receiver = remap_oop (memory, processor->message_receiver);
}

static void
clear_metadata (st_memory *memory)
{
    memset (memory->mark_bits, 0, memory->bits_size);
    memset (memory->alloc_bits, 0, memory->bits_size);
    memset (memory->offsets, 0, memory->offsets_size);
}

static void
garbage_collect (st_memory *memory)
{
    struct timespec start, end, interval;
    st_oop context;

    clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &start);

    clear_metadata (memory);

    mark (memory);
    compact (memory);
    remap (memory);
    synchronize_processor (memory, proc);

    clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &end);
    st_timespec_difference (&start, &end, &interval);
    st_timespec_add (&memory->tot_pause_time, &interval ,&memory->tot_pause_time);

    if (st_verbose_mode ()) {
	fprintf (stderr, "** gc: %uK->%uK (%uK), %.6fs\n",
		 (memory->bytes_collected + memory->bytes_allocated) / 1024,
		 memory->bytes_allocated / 1024,
		 memory->bytes_collected / 1024,
		 st_timespec_to_double_seconds (&interval));
    }
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
