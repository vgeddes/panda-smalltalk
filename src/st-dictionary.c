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



/* Hashed Collection methods */

static inline st_uint
occupied_count (st_oop collection)
{
    return st_smi_value (SIZE (collection)) + st_smi_value (DELETED (collection));
}

static st_uint
array_size_for_capacity (st_uint capacity)
{
    st_uint size;

    size = MINIMUM_CAPACITY;

    while (size < capacity)
	size = size + size;

    return size;
}

static st_smi
collection_find_element_or_nil (st_oop collection, st_oop object)
{
    st_smi index;

    if (st_object_class (collection) == st_dictionary_class)
	index = dict_find (collection, object);
    else if (st_object_class (collection) == st_set_class)
	index = set_find (collection, object);
    else
	st_assert_not_reached ();

    if (index > 0)
	return index;

    st_assert_not_reached ();
}

static void
collection_check_grow (st_oop collection)
{
    st_smi array_size;
    st_oop old_array;
    st_smi new_size;
    st_smi n;

    array_size = ARRAY_SIZE (ARRAY (collection));
    
    if ((occupied_count (collection) * 2) <= array_size)
	return;

    old_array = ARRAY (collection);
    new_size = ARRAY_SIZE (old_array) * 2;
    ARRAY (collection) = st_object_new_arrayed (st_array_class, new_size);
    n = ARRAY_SIZE (old_array);

    for (st_smi i = 1; i <= n; i++)
	if (st_array_at (old_array, i) != st_nil) {
	    if (st_object_class (collection) == st_dictionary_class)
		dict_no_check_add (collection, st_array_at (old_array, i));
	    else if (st_object_class (collection) == st_set_class)
		set_no_check_add (collection, st_array_at (old_array, i));
	    else
		st_assert_not_reached ();
	}

    DELETED (collection) = st_smi_new (0);
}

static void
collection_at_new_index_put (st_oop collection, st_smi index, st_oop object)
{
    st_array_at_put (ARRAY (collection), index, object);

    SIZE (collection) = st_smi_increment (SIZE (collection));

    collection_check_grow (collection);
}

static void
collection_initialize (st_oop collection, st_smi capacity)
{
    st_assert (capacity > 0);

    capacity = array_size_for_capacity (capacity);

    SIZE (collection) = st_smi_new (0);
    DELETED (collection) = st_smi_new (0);
    ARRAY (collection) = st_object_new_arrayed (st_array_class, capacity);
}

static st_uint
dict_find (st_oop dict, st_oop object)
{
    st_oop el;
    st_uint mask;
    st_uint i;

    mask = ARRAY_SIZE (ARRAY (dict)) - 1;
    i = (st_object_hash (object) & mask) + 1;

    while (true) {
	el = st_array_at (ARRAY (dict), i);

	if (el == st_nil || st_object_equal (object, ST_ASSOCIATION_KEY (el)))
	    return i;
	i = ((i + 106720) & mask) + 1;
    }

    abort ();
    return 0;
}

static void
dict_no_check_add (st_oop dict, st_oop object)
{
    st_array_at_put (ARRAY (dict),
		     collection_find_element_or_nil (dict, ST_ASSOCIATION_KEY (object)), object);
}

void
st_dictionary_at_put (st_oop dict, st_oop key, st_oop value)
{
    st_smi index = collection_find_element_or_nil (dict, key);
    st_oop assoc = st_array_at (ARRAY (dict), index);

    if (assoc == st_nil)
	collection_at_new_index_put (dict, index, st_association_new (key, value));
    else
	ST_ASSOCIATION_VALUE (assoc) = value;
}

st_oop
st_dictionary_at (st_oop dict, st_oop key)
{
    st_smi index = collection_find_element_or_nil (dict, key);

    st_oop assoc = st_array_at (ARRAY (dict), index);

    if (assoc != st_nil)
	return ST_ASSOCIATION_VALUE (assoc);

    return st_nil;
}

st_oop
st_dictionary_association_at (st_oop dict, st_oop key)
{
    st_smi index = collection_find_element_or_nil (dict, key);

    return st_array_at (ARRAY (dict), index);
}

st_oop
st_dictionary_new (void)
{
    return st_dictionary_new_with_capacity (DEFAULT_CAPACITY);
}

st_oop
st_dictionary_new_with_capacity (st_smi capacity)
{
    st_oop dict;
    
    dict = st_object_new (st_dictionary_class);
    collection_initialize (dict, capacity);
    
    return dict;
}

static st_uint
set_find (st_oop dict, st_oop object)
{
    st_oop el;
    st_uint mask;
    st_uint i;

    mask = ARRAY_SIZE (ARRAY (dict)) - 1;
    i = (st_object_hash (object) & mask) + 1;

    while (true) {
	el = st_array_at (ARRAY (dict), i);
	if (el == st_nil || st_object_equal (el, object))
	    return i;
	i = ((i + 106720) & mask) + 1;
    }

    abort ();
    return 0;
}


static void
set_no_check_add (st_oop set, st_oop object)
{
    st_array_at_put (ARRAY (set), collection_find_element_or_nil (set, object), object);
}

void
st_set_add (st_oop set, st_oop object)
{
    st_smi  index;

    index = collection_find_element_or_nil (set, object);
    collection_at_new_index_put (set, index, object);
}

bool
st_set_includes (st_oop set, st_oop object)
{
    st_smi index = collection_find_element_or_nil (set, object);

    return st_array_at (ARRAY (set), index) != st_nil;
}

st_oop
st_set_like (st_oop set, st_oop object)
{
    st_smi index = collection_find_element_or_nil (set, object);

    return st_array_at (ARRAY (set), index);
}

st_oop
st_set_new_with_capacity (st_smi capacity)
{
    st_oop set;

    set = st_object_new (st_set_class);

    collection_initialize (set, capacity);

    return set;
}

st_oop
st_set_new (void)
{
    return st_set_new_with_capacity (DEFAULT_CAPACITY);
}
