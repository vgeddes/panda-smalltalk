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

#include <st-object.h>
#include <st-types.h>
#include <st-descriptor.h>

#define ST_FLOAT_ARRAY(oop) ((struct st_float_array *) ST_POINTER (oop))

struct st_float_array
{
    struct st_arrayed_object header;

    double elements[];
};

st_descriptor *st_float_array_descriptor (void);

static inline double *
st_float_array_elements (st_oop array)
{
    return ST_FLOAT_ARRAY (array)->elements;
}

static inline double
st_float_array_at (st_oop array, st_smi i)
{
    return ST_FLOAT_ARRAY (array)->elements[i - 1];
}

static inline void
st_float_array_at_put (st_oop array, st_smi i, double value)
{
    ST_FLOAT_ARRAY (array)->elements[i - 1] = value;
}




#endif /* __ST_FLOAT_ARRAY_H__ */
