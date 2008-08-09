/*
 * st-hashed-collection.c
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

#include "st-dictionary.h"
#include "st-array.h"
#include "st-small-integer.h"
#include "st-utils.h"
#include "st-universe.h"
#include "st-association.h"
#include "st-object.h"
#include "st-behavior.h"

#define DEFAULT_CAPACITY     8
#define MINIMUM_CAPACITY     4
#define SIZE(collection)     (ST_HASHED_COLLECTION (collection)->size)
#define DELETED(collection)  (ST_HASHED_COLLECTION (collection)->deleted)
#define ARRAY(collection)    (ST_HASHED_COLLECTION (collection)->array)
#define ARRAY_SIZE(array)    (st_smi_value (ST_ARRAYED_OBJECT (array)->size))

// prime number - 1
#define ADVANCE_SIZE         106720 

#define ST_HASHED_COLLECTION(oop) ((struct st_hashed_collection *) ST_POINTER (oop))

struct st_hashed_collection
{
    struct st_header header;

    st_oop size;
    st_oop deleted;
    st_oop array;

};

static st_uint  dict_find (st_oop dict, st_oop object);
static st_uint  set_find  (st_oop set, st_oop object);
static void     dict_no_check_add    (st_oop dict, st_oop object);
static void     set_no_check_add     (st_oop set, st_oop object);



/* Common methods */

static inline st_uint
occupied (st_oop collection)
{
    return st_smi_value (SIZE (collection)) + st_smi_value (DELETED (collection));
}

static st_uint
size_for_capacity (st_uint capacity)
{
    st_uint size;

    size = MINIMUM_CAPACITY;

    while (size < capacity)
	size = size + size;

    return size;
}

static void
initialize (st_oop collection, int capacity)
{
    st_assert (capacity > 0);

    capacity = size_for_capacity (capacity);

    SIZE (collection) = st_smi_new (0);
    DELETED (collection) = st_smi_new (0);
    ARRAY (collection) = st_object_new_arrayed (ST_ARRAY_CLASS, capacity);
}

static void
dict_check_grow (st_oop dict)
{
    st_oop  old, object;
    st_uint size, n;

    size = n = ARRAY_SIZE (ARRAY (dict));
    
    if (occupied (dict) * 2 <= size)
	return;

    size *= 2;
    old = ARRAY (dict);
    ARRAY (dict)   = st_object_new_arrayed (ST_ARRAY_CLASS, size);
    DELETED (dict) = st_smi_new (0);

    for (st_uint i = 1; i <= n; i++) {
	object = st_array_at (old, i);
	if (object != ST_NIL) {
	    st_array_at_put (ARRAY (dict), dict_find (dict, ST_ASSOCIATION_KEY (object)), object);
	}
    }
}

static st_uint
dict_find (st_oop dict, st_oop key)
{
    st_oop el;
    st_uint mask;
    st_uint i;

    mask = ARRAY_SIZE (ARRAY (dict)) - 1;
    i = (st_object_hash (key) & mask) + 1;

    while (true) {
	el = st_array_at (ARRAY (dict), i);
	if (el == ST_NIL || key == ST_ASSOCIATION_KEY (el))
	    return i;
	i = ((i + ADVANCE_SIZE) & mask) + 1;
    }
}

void
st_dictionary_at_put (st_oop dict, st_oop key, st_oop value)
{
    st_uint index; 
    st_oop  assoc;

    index = dict_find (dict, key);
    assoc = st_array_at (ARRAY (dict), index);

    if (assoc == ST_NIL) {
	st_array_at_put (ARRAY (dict), index, st_association_new (key, value));
	SIZE (dict) = st_smi_increment (SIZE (dict));
	dict_check_grow (dict);
    } else {
	ST_ASSOCIATION_VALUE (assoc) = value;
    }
}

st_oop
st_dictionary_at (st_oop dict, st_oop key)
{
    int index;
    st_oop assoc;

    assoc = st_array_at (ARRAY (dict), dict_find (dict, key));

    if (assoc != ST_NIL)
	return ST_ASSOCIATION_VALUE (assoc);

    return ST_NIL;
}

st_oop
st_dictionary_association_at (st_oop dict, st_oop key)
{
    return st_array_at (ARRAY (dict), dict_find (dict, key));
}

st_oop
st_dictionary_new (void)
{
    return st_dictionary_new_with_capacity (DEFAULT_CAPACITY);
}

st_oop
st_dictionary_new_with_capacity (int capacity)
{
    st_oop dict;
    
    dict = st_object_new (ST_DICTIONARY_CLASS);
    initialize (dict, capacity);
    
    return dict;
}


/* Set implementation */

static st_uint
set_find_cstring (st_oop set, const char *string)
{
    st_oop el;
    st_uint mask, i;

    mask = ARRAY_SIZE (ARRAY (set)) - 1;
    i = (st_string_hash (string) & mask) + 1;

    while (true) {
	el = st_array_at (ARRAY (set), i);
	if (el == ST_NIL || (strcmp (st_byte_array_bytes (el), string) == 0))
	    return i;
	i = ((i + ADVANCE_SIZE) & mask) + 1;
    }
}

static st_uint
set_find (st_oop set, st_oop object)
{
    st_oop el;
    st_uint mask, i;

    mask = ARRAY_SIZE (ARRAY (set)) - 1;
    i = (st_object_hash (object) & mask) + 1;

    while (true) {
	el = st_array_at (ARRAY (set), i);
	if (el == ST_NIL || el == object)
	    return i;
	i = ((i + ADVANCE_SIZE) & mask) + 1;
    }
}

static void
set_check_grow (st_oop set)
{
    st_oop  old, object;
    st_uint size, n;

    size = n = ARRAY_SIZE (ARRAY (set));
    
    if (occupied (set) * 2 <= size)
	return;

    size *= 2;
    old = ARRAY (set);
    ARRAY (set)   = st_object_new_arrayed (ST_ARRAY_CLASS, size);
    DELETED (set) = st_smi_new (0);

    for (st_uint i = 1; i <= n; i++) {
	object = st_array_at (old, i);
	if (object != ST_NIL)
	    st_array_at_put (ARRAY (set), set_find (set, object), object);
    }
}

st_oop
st_set_intern_cstring (st_oop set, const char *string)
{
    st_oop  intern;
    st_uint i, len;

    st_assert (string != NULL);

    i = set_find_cstring (set, string);
    intern = st_array_at (ARRAY (set), i);
    if (intern != ST_NIL)
	return intern;

    len = strlen (string);
    intern = st_object_new_arrayed (ST_SYMBOL_CLASS, len);
    memcpy (st_byte_array_bytes (intern), string, len);
    st_array_at_put (ARRAY (set), i, intern);
    SIZE (set) = st_smi_increment (SIZE (set));
    set_check_grow (set);

    return intern;
}

void
st_set_add (st_oop set, st_oop object)
{
    st_array_at_put (ARRAY (set), set_find (set, object), object);
    SIZE (set) = st_smi_increment (SIZE (set));
    set_check_grow (set);
}

bool
st_set_includes (st_oop set, st_oop object)
{
    return st_array_at (ARRAY (set), set_find (set, object)) != ST_NIL;
}

st_oop
st_set_like (st_oop set, st_oop object)
{
    return st_array_at (ARRAY (set), set_find (set, object));
}

st_oop
st_set_new_with_capacity (int capacity)
{
    st_oop set;

    set = st_object_new (ST_SET_CLASS);
    initialize (set, capacity);

    return set;
}

st_oop
st_set_new (void)
{
    return st_set_new_with_capacity (DEFAULT_CAPACITY);
}
