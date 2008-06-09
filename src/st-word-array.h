/* 
 * st-array.h
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

#ifndef __ST_WORD_ARRAY_H__
#define __ST_WORD_ARRAY_H__

#include <st-types.h>
#include <st-object.h>
#include <glib.h>

#define ST_WORD_ARRAY(oop) ((struct st_word_array *) ST_POINTER (oop))

struct st_word_array
{
    struct st_arrayed_object header;

    st_uint elements[];
};

st_descriptor *st_word_array_descriptor (void);

INLINE st_uint *
st_word_array_elements (st_oop object)
{
    return ST_WORD_ARRAY (object)->elements;
}

INLINE st_uint
st_word_array_at (st_oop object, st_smi i)
{
    st_assert (1 <= i && i <= st_arrayed_object_size (object));

    return ST_WORD_ARRAY (object)->elements[i - 1];
}

INLINE void
st_word_array_at_put (st_oop object, st_smi i, st_uint value)
{
    st_assert (1 <= i && i <= st_arrayed_object_size (object));

    ST_WORD_ARRAY (object)->elements[i - 1] = value;
}

#endif /* __ST_WORD_ARRAY_H__ */
