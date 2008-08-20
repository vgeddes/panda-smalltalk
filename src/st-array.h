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
#include <st-object.h>

#define ST_ARRAYED_OBJECT(oop) ((struct st_arrayed_object *) st_detag_pointer (oop))
#define ST_ARRAY(oop)          ((struct st_array *) st_detag_pointer (oop))
#define ST_FLOAT_ARRAY(oop)    ((struct st_float_array *) st_detag_pointer (oop))
#define ST_WORD_ARRAY(oop)     ((struct st_word_array *) st_detag_pointer (oop))
#define ST_BYTE_ARRAY(oop)     ((struct st_byte_array *) st_detag_pointer (oop))

struct st_arrayed_object
{
    struct st_header header;
    st_oop           size;
};

struct st_array
{
    struct st_arrayed_object header;

    st_oop elements[];
};

struct st_word_array
{
    struct st_arrayed_object header;

    st_uint elements[];
};

struct st_float_array
{
    struct st_arrayed_object header;

    double elements[];
};

struct st_byte_array
{
    struct st_arrayed_object header;

    st_uchar bytes[];
};

bool    st_byte_array_equal     (st_oop object, st_oop other);
st_uint st_byte_array_hash      (st_oop object);
st_oop  st_array_allocate       (st_oop class, st_uint size);
st_oop  st_float_array_allocate (st_oop class, int size);
st_oop  st_word_array_allocate  (st_oop class, int size);
st_oop  st_byte_array_allocate  (st_oop class, int size);

static inline st_oop
st_arrayed_object_size (st_oop object)
{
    return ST_ARRAYED_OBJECT (object)->size;
}


static inline st_oop *
st_array_elements (st_oop object)
{
    return ST_ARRAY (object)->elements;
}

static inline st_oop
st_array_at (st_oop object, int i)
{
    return (ST_ARRAY (object)->elements - 1)[i];
}

static inline void
st_array_at_put (st_oop object, int i, st_oop value)
{    
    (ST_ARRAY (object)->elements - 1)[i] = value;
}

static inline st_uint *
st_word_array_elements (st_oop object)
{
    return ST_WORD_ARRAY (object)->elements;
}

static inline st_uint
st_word_array_at (st_oop object, int i)
{
    return ST_WORD_ARRAY (object)->elements[i - 1];
}

static inline void
st_word_array_at_put (st_oop object, int i, st_uint value)
{
    ST_WORD_ARRAY (object)->elements[i - 1] = value;
}

static inline st_uchar *
st_byte_array_bytes (st_oop object)
{
    return ST_BYTE_ARRAY (object)->bytes;
}

static inline st_uchar
st_byte_array_at (st_oop object, int i)
{
    return st_byte_array_bytes (object)[i - 1];
}

static inline void
st_byte_array_at_put (st_oop object, int i, st_uchar value)
{
    st_byte_array_bytes (object)[i - 1] = value;
}

static inline double *
st_float_array_elements (st_oop array)
{
    return ST_FLOAT_ARRAY (array)->elements;
}

static inline double
st_float_array_at (st_oop array, int i)
{
    return ST_FLOAT_ARRAY (array)->elements[i - 1];
}

static inline void
st_float_array_at_put (st_oop array, int i, double value)
{
    ST_FLOAT_ARRAY (array)->elements[i - 1] = value;
}

#endif /* __ST_ARRAY_H__ */
