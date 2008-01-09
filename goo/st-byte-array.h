/* 
 * st-byte-array.h
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

#ifndef __ST_BYTE_ARRAY_H__
#define __ST_BYTE_ARRAY_H__

#include <st-types.h>
#include <st-small-integer.h>
#include <st-heap-object.h>

typedef struct
{
    st_header_t header;
    
    st_oop_t size;
    
    guchar bytes[];
    
} st_byte_array_t;


INLINE st_oop_t  st_byte_array_size    (st_oop_t object);

INLINE guchar   *st_byte_array_bytes   (st_oop_t object);

INLINE guchar    st_byte_array_at      (st_oop_t object, st_smi_t i);

INLINE void      st_byte_array_at_put  (st_oop_t object, st_smi_t i, guchar value);

st_vtable_t     *st_byte_array_vtable  (void);


/* inline definitions */
#define _ST_BYTE_ARRAY(oop) ((st_byte_array_t *) ST_POINTER (oop))

INLINE st_oop_t
st_byte_array_size (st_oop_t object)
{
	return _ST_BYTE_ARRAY (object)->size;
}

INLINE guchar *
st_byte_array_bytes (st_oop_t object)
{
	return _ST_BYTE_ARRAY (object)->bytes;
}

INLINE bool
st_byte_array_range_check (st_oop_t object, st_smi_t i)
{
	return 1 <= i && i <= st_smi_value (st_byte_array_size (object));
}

INLINE guchar
st_byte_array_at (st_oop_t object, st_smi_t i)
{	
	g_assert (1 <= i && i <= st_smi_value (st_byte_array_size (object)));
	
	return st_byte_array_bytes (object)[i - 1];
}

INLINE void
st_byte_array_at_put (st_oop_t object, st_smi_t i, guchar value)
{
	g_assert (1 <= i && i <= st_smi_value (st_byte_array_size (object)));

	st_byte_array_bytes (object)[i - 1] = value;
}

st_vtable_t  *st_byte_array_vtable (void);

#endif /* __ST_BYTE_ARRAY__ */
