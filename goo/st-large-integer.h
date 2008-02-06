/*
 * st-large-integer.h
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

#ifndef __ST_LARGE_INTEGER__
#define __ST_LARGE_INTEGER__

#include <tommath.h>
#include <st-heap-object.h>

typedef struct
{
    STHeader header;

    mp_int value;

} STLargeInteger;


st_oop st_large_integer_new (mp_int * value);

st_oop st_large_integer_new_from_string (const char *string, guint radix);

char *st_large_integer_to_string (st_oop integer, guint radix);

INLINE mp_int *st_large_integer_value (st_oop integer);

const STVTable *st_large_integer_vtable (void);


/* inline definitions */
#define ST_LARGE_INTEGER(oop) ((STLargeInteger *) ST_POINTER (oop))

INLINE mp_int *
st_large_integer_value (st_oop integer)
{
    return & ST_LARGE_INTEGER (integer)->value;
}

#endif /* __ST_LARGE_INTEGER */
