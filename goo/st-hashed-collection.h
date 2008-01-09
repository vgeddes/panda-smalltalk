/* 
 * st-hashed-collection.h
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

#ifndef __ST_HASHED_COLLECTION_H__
#define __ST_HASHED_COLLECTION_H__

#include <st-types.h>
#include <st-array.h>

st_oop_t   st_dictionary_new               (void);
st_oop_t   st_dictionary_new_with_capacity (long capacity);
st_oop_t   st_dictionary_at                (st_oop_t dict, st_oop_t key);
void       st_dictionary_at_put            (st_oop_t dict, st_oop_t key, st_oop_t value);


st_oop_t   st_set_new               (void);
st_oop_t   st_set_new_with_capacity (long capacity);
bool       st_set_includes          (st_oop_t set, st_oop_t object);
st_oop_t   st_set_like              (st_oop_t set, st_oop_t object);
void       st_set_add               (st_oop_t set, st_oop_t object);


st_vtable_t  *st_dictionary_vtable (void);
st_vtable_t  *st_set_vtable (void);


#endif /* __ST_HASHED_COLLECTION_H__ */

