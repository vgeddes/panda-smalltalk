/*
 * st-small-integer.h
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

#ifndef __ST_CHARACTER_H__
#define __ST_CHARACTER_H__

#include <st-types.h>
#include <st-descriptor.h>

INLINE st_oop  st_character_new   (wchar_t wc);
INLINE wchar_t st_character_value (st_oop character);
INLINE bool    st_character_equal (st_oop m, st_oop n);
INLINE st_smi  st_character_hash  (st_oop character);

/* inline definitions */

INLINE st_oop
st_character_new (wchar_t wc)
{
    return (st_oop) ((wc << ST_TAG_SIZE) + ST_CHARACTER_TAG);
}

INLINE wchar_t
st_character_value (st_oop character)
{
    return ((wchar_t) character) >> ST_TAG_SIZE;
}

INLINE bool
st_character_equal (st_oop m, st_oop n)
{
    return m == n;
}

INLINE st_smi
st_character_hash (st_oop character)
{
    return (st_smi) st_character_value (character);
}

#endif /* __ST_CHARACTER_H__ */
