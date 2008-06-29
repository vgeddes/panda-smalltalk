/*
 * st-association.c
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

#include "st-association.h"
#include "st-behavior.h"

st_uint
st_association_hash (st_oop object)
{    
    return st_object_hash (ST_ASSOCIATION (object)->key) ^ st_object_hash (ST_ASSOCIATION (object)->value);
}

bool
st_association_equal (st_oop object, st_oop other)
{
    struct st_association *m, *n;

    if (st_object_class (other) != st_association_class)
	return false;

    m = ST_ASSOCIATION (object);
    n = ST_ASSOCIATION (other);

    return st_object_equal (m->key, n->key) && st_object_equal (m->value, n->value);
}

st_oop
st_association_new (st_oop key, st_oop value)
{
    st_oop assoc = st_object_new (st_association_class);

    ST_ASSOCIATION (assoc)->key   =  key;
    ST_ASSOCIATION (assoc)->value = value;

    return assoc;
}
