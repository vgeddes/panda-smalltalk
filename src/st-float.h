/*
 * st-float.h
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

#ifndef __ST_FLOAT_H__
#define __ST_FLOAT_H__

#include <st-types.h>
#include <st-descriptor.h>
#include <st-heap-object.h>
#include <st-object.h>

#define ST_FLOAT(oop) ((struct st_float *) ST_POINTER (oop))

struct st_float
{
    struct st_header header;

    double value;

};

st_oop        st_float_new       (double value);

static inline double st_float_value     (st_oop f);

static inline void   st_float_set_value (st_oop f, double value);

static inline bool   st_float_equal     (st_oop object, st_oop other);

static inline st_uint  st_float_hash      (st_oop object);

st_descriptor *st_float_descriptor (void);

static inline double
st_float_value (st_oop f)
{
    return ST_FLOAT (f)->value;
}

static inline void
st_float_set_value (st_oop f, double value)
{
    ST_FLOAT (f)->value = value;
}

static inline bool
st_float_equal (st_oop object, st_oop other)
{
    if (st_object_class (other) != st_float_class)
	return false;

    return st_float_value (object) == st_float_value (other);
}

static inline st_uint
st_float_hash (st_oop object)
{
    return (st_uint) st_float_value (object);
}


#endif /* __ST_FLOAT_H__ */
