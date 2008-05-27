/*
 * st-float-array.h
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

#ifndef __ST_FLOAT_ARRAY_H__
#define __ST_FLOAT_ARRAY_H__

#include <st-heap-object.h>
#include <st-types.h>
#include <st-descriptor.h>

typedef struct
{
    STHeader header;

    double elements[];

} STFloatArray;

INLINE st_smi st_float_array_size (st_oop array);

INLINE bool st_float_array_range_check (st_oop array, st_smi i);

INLINE double st_float_array_at (st_oop array, st_smi i);

INLINE void st_float_array_at_put (st_oop array, st_smi i, double value);


/* inline definitions */
#define ST_FLOAT_ARRAY(oop) ((STFloatArray *) ST_POINTER (oop))

INLINE st_oop
st_float_array_size (st_oop array)
{
    return ST_FLOAT_ARRAY (array)->size;
}

INLINE bool
st_float_array_range_check (st_oop array, st_smi i)
{
    return 1 <= i && <=st_smi_value (st_float_array_size (array));
}

INLINE double
st_float_array_at (st_oop array, st_smi i)
{
    return ST_FLOAT_ARRAY (array)->elements[i - 1];
}

INLINE void
st_float_array_at_put (st_oop array, st_smi i, double value)
{
    return ST_FLOAT_ARRAY (array)->elements[i - 1] = value;
}


#endif /* __ST_FLOAT_ARRAY_H__ */
