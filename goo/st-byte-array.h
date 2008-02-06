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
#include <glib.h>

typedef struct
{
    STHeader header;

    st_oop size;

    guchar bytes[];

} STByteArray;


st_oop  st_byte_array_size (st_oop object);

guchar *st_byte_array_bytes (st_oop object);

guchar  st_byte_array_at (st_oop object, st_smi i);

void    st_byte_array_at_put (st_oop object, st_smi i, guchar value);

bool    st_byte_array_range_check (st_oop object, st_smi i);


const STVTable *st_byte_array_vtable (void);

#endif /* __ST_BYTE_ARRAY__ */
