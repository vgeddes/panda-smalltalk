/* 
 * st-mark.h
 *
 * Copyright (C) 2008 Vincent Geddes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __ST_MARK_H__
#define __ST_MARK_H__

#include <st-types.h>
#include <st-utils.h>
#include <stdio.h>

INLINE st_oop_t  st_mark_new          (void);
INLINE int       st_mark_hash         (st_oop_t mark);
INLINE st_oop_t  st_mark_set_hash     (st_oop_t mark, int value);
INLINE bool      st_mark_readonly     (st_oop_t mark);
INLINE st_oop_t  st_mark_set_readonly (st_oop_t mark, bool readonly);



/* inline definitions */

/* format of mark oop
 * [ unused: 9 | non-pointer: 1 | readonly: 1 | hash: 20 | tag: 2 ]
 *
 * unused: 	not used yet (haven't implemented GC yet)
 * non-pointer:	object body contains native C types
 * readonly:	object cannot be modified
 * hash:	identity hash for use in Collections
 * tag:		standard oop tag. In this case, ST_MARK_TAG
 * 
 */
enum
{
	st_unused_bits     = 9,
	st_nonpointer_bits = 1,
	st_readonly_bits   = 1,
	st_hash_bits       = 20,

	st_hash_shift       = ST_TAG_SIZE,
	st_readonly_shift   = st_hash_bits + st_hash_shift,
	st_nonpointer_shift = st_readonly_bits + st_readonly_shift,
	st_unused_shift     = st_nonpointer_bits + st_nonpointer_shift,

	st_hash_mask                = ST_NTH_MASK (st_hash_bits),
	st_hash_mask_in_place       = st_hash_mask << st_hash_shift,
	st_readonly_mask            = ST_NTH_MASK (st_readonly_bits),
	st_readonly_mask_in_place   = st_readonly_mask << st_readonly_shift,
	st_nonpointer_mask          = ST_NTH_MASK (st_nonpointer_bits),
	st_nonpointer_mask_in_place = st_nonpointer_mask << st_nonpointer_shift,
	st_unused_mask              = ST_NTH_MASK (st_unused_bits),
	st_unused_mask_in_place     = st_unused_mask << st_hash_shift,
};

extern int _st_current_hash;

INLINE int
st_mark_hash (st_oop_t mark)
{
	return (mark >> st_hash_shift) & st_hash_mask;
}

INLINE st_oop_t
st_mark_set_hash (st_oop_t mark, int value)
{
	if ((value & st_hash_mask) == 0)
		value = 1;
	return (mark & ~st_hash_mask_in_place) | ((value & st_hash_mask) << st_hash_shift);
}

INLINE bool
st_mark_readonly (st_oop_t mark)
{
	return (mark >> st_readonly_shift) & st_readonly_mask;
}

INLINE st_oop_t
st_mark_set_readonly (st_oop_t mark, bool readonly)
{
	return (mark & ~st_readonly_mask_in_place) | ((readonly ? 1 : 0) << st_readonly_shift);
}

INLINE bool
st_mark_nonpointer (st_oop_t mark)
{
	return (mark >> st_nonpointer_shift) & st_nonpointer_mask;
}

INLINE st_oop_t
st_mark_set_nonpointer (st_oop_t mark, bool nonpointer)
{
	return (mark & ~st_nonpointer_mask_in_place) | ((nonpointer ? 1 : 0) << st_nonpointer_shift);
}


INLINE st_oop_t
st_mark_new (void)
{		
	st_oop_t mark = (st_oop_t) ST_MARK_TAG;
	mark = st_mark_set_hash (mark, _st_current_hash++);
	mark = st_mark_set_readonly (mark, false);
	mark = st_mark_set_nonpointer (mark, false);
	
	//printf ("%i ", st_mark_hash (mark));
	
	return mark;
}

#endif /* __ST_MARK_H__ */
