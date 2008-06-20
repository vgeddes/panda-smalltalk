/*
 * st-byte-array.h
 *
 * Copyright (c) 2008 Vincent Geddes
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

#ifndef __ST_BYTE_ARRAY_H__
#define __ST_BYTE_ARRAY_H__

#include <st-types.h>
#include <st-small-integer.h>
#include <st-heap-object.h>

#define ST_BYTE_ARRAY(oop) ((struct st_byte_array *) ST_POINTER (oop))

struct st_byte_array
{
    struct st_arrayed_object header;

    st_uchar bytes[];
};

bool           st_byte_array_equal      (st_oop object, st_oop other);

st_uint          st_byte_array_hash       (st_oop object);

st_descriptor *st_byte_array_descriptor (void);


INLINE st_uchar *
st_byte_array_bytes (st_oop object)
{
    return ST_BYTE_ARRAY (object)->bytes;
}

INLINE st_uchar
st_byte_array_at (st_oop object, st_smi i)
{
    st_assert (1 <= i && i <= st_arrayed_object_size (object));

    return st_byte_array_bytes (object)[i - 1];
}

INLINE void
st_byte_array_at_put (st_oop object, st_smi i, st_uchar value)
{
    st_assert (1 <= i && i <= st_arrayed_object_size (object));

    st_byte_array_bytes (object)[i - 1] = value;
}



#endif /* __ST_BYTE_ARRAY__ */
