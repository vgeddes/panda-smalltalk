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

#ifndef __ST_ARRAY_H__
#define __ST_ARRAY_H__

#include <st-types.h>
#include <st-class.h>
#include <st-object.h>
#include <glib.h>

typedef struct
{
    STHeader header;

    st_oop size;

    st_oop elements[];

} STArray;


INLINE st_oop st_array_size (st_oop object);

INLINE bool st_array_range_check (st_oop object, st_smi i);

INLINE st_oop *st_array_element (st_oop object, st_smi i);

INLINE st_oop st_array_at (st_oop object, st_smi i);

INLINE void st_array_at_put (st_oop object, st_smi i, st_oop value);

guint st_array_vtable (void);


/* inline definitions */
#define ST_ARRAY(oop) ((STArray *) ST_POINTER (oop))

INLINE st_oop
st_array_size (st_oop object)
{
    return ST_ARRAY (object)->size;
}

INLINE bool
st_array_range_check (st_oop object, st_smi i)
{
    return 1 <= i && i <= st_smi_value (st_array_size (object));
}

/* 
 * returns address of element at index i > 0
 */
INLINE st_oop *
st_array_element (st_oop object, st_smi i)
{
    /* by obtaining the element address as an offset from `&array->size',
     * we avoid the slight overhead of subtraction if we had used `&array->elements[i - 1]' instead.
     */
    return (&ST_ARRAY (object)->size) + i;
}

INLINE st_oop
st_array_at (st_oop object, st_smi i)
{
    g_assert (1 <= i && i <= st_array_size (object));

    return *st_array_element (object, i);
}

INLINE void
st_array_at_put (st_oop object, st_smi i, st_oop value)
{
    g_assert (1 <= i && i <= st_array_size (object));

    *st_array_element (object, i) = value;
}

#endif /* __ST_ARRAY_H__ */
