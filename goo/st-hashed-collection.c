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

#include "st-hashed-collection.h"
#include "st-array.h"
#include "st-small-integer.h"
#include "st-utils.h"
#include "st-universe.h"
#include "st-association.h"
#include "st-heap-object.h"

#include <glib.h>

#define DEFAULT_CAPACITY      5
#define TALLY(collection)    (ST_HASHED_COLLECTION (collection)->tally)
#define ARRAY(collection)    (ST_HASHED_COLLECTION (collection)->array)
#define ARRAY_SIZE(array)    (st_smi_value (st_arrayed_object_size (array)))

#define ST_HASHED_COLLECTION(oop) ((STHashedCollection *) ST_POINTER (oop))

typedef struct
{
    STHeader header;

    st_oop tally;
    st_oop array;

} STHashedCollection;

static st_smi  dict_scan_for_object (st_oop dict, st_oop object);

static void    dict_no_check_add    (st_oop dict, st_oop object);

static void    set_no_check_add     (st_oop set, st_oop object);

static st_smi  set_scan_for_object  (st_oop set, st_oop object);


/* Hashed Collection methods */

static st_smi
collection_find_element_or_nil (st_oop collection, st_oop object)
{
    st_smi index;

    if (st_object_class (collection) == st_dictionary_class)
	index = dict_scan_for_object (collection, object);
    else if (st_object_class (collection) == st_set_class)
	index = set_scan_for_object (collection, object);
    else
	g_assert_not_reached ();

    if (index > 0)
	return index;

    g_assert_not_reached ();
}

static st_smi
collection_grow_size (st_oop collection)
{
    return MAX (ARRAY_SIZE (ARRAY (collection)), 2);
}

static void
collection_grow (st_oop collection)
{
    st_oop old_array = ARRAY (collection);

    st_smi new_size = ARRAY_SIZE (old_array) + collection_grow_size (collection);

    ARRAY (collection) = st_object_new_arrayed (st_array_class, new_size);

    st_smi n = ARRAY_SIZE (old_array);

    for (st_smi i = 1; i <= n; i++)
	if (st_array_at (old_array, i) != st_nil) {

	    if (st_object_class (collection) == st_dictionary_class) {
		dict_no_check_add (collection, st_array_at (old_array, i));
	    } else if (st_object_class (collection) == st_set_class) {
		set_no_check_add (collection, st_array_at (old_array, i));
	    } else {
		g_assert_not_reached ();
	    }
	}
}

static void
collection_full_check (st_oop collection)
{
    st_smi array_size = ARRAY_SIZE (ARRAY (collection));
    st_smi tally = st_smi_value (TALLY (collection));

    if ((array_size - tally) < MAX ((array_size / 4), 1))
	collection_grow (collection);
}

static void
collection_at_new_index_put (st_oop collection, st_smi index, st_oop object)
{
    st_array_at_put (ARRAY (collection), index, object);

    TALLY (collection) = st_smi_increment (TALLY (collection));

    collection_full_check (collection);
}

static void
collection_initialize (st_oop collection, st_smi capacity)
{
    g_assert (capacity > 0);

    TALLY (collection) = st_smi_new (0);
    ARRAY (collection) = st_object_new_arrayed (st_array_class, capacity);
}

static st_smi
dict_scan_for_object (st_oop dict, st_oop object)
{
    st_smi finish = ARRAY_SIZE (ARRAY (dict));
    st_smi start = (st_object_hash (object) % finish) + 1;

    for (st_smi i = start; i <= finish; i++) {

	st_oop element = st_array_at (ARRAY (dict), i);

	if (element == st_nil)
	    return i;
	if (st_object_equal (object, st_association_key (element)))
	    return i;
    }

    for (st_smi i = 1; i <= (start - 1); i++) {

	st_oop element = st_array_at (ARRAY (dict), i);

	if (element == st_nil)
	    return i;
	if (st_object_equal (object, st_association_key (element)))
	    return i;

    }

    return 0;
}

static void
dict_no_check_add (st_oop dict, st_oop object)
{
    st_array_at_put (ARRAY (dict),
		     collection_find_element_or_nil (dict, st_association_key (object)), object);
}

void
st_dictionary_at_put (st_oop dict, st_oop key, st_oop value)
{
    st_smi index = collection_find_element_or_nil (dict, key);

    st_oop assoc = st_array_at (ARRAY (dict), index);

    if (assoc == st_nil) {

	assoc = st_association_new (key, value);
	collection_at_new_index_put (dict, index, assoc);

    } else {
	st_association_value (assoc) = value;
    }
}

st_oop
st_dictionary_at (st_oop dict, st_oop key)
{
    st_smi index = collection_find_element_or_nil (dict, key);

    st_oop assoc = st_array_at (ARRAY (dict), index);

    if (assoc != st_nil)
	return st_association_value (assoc);

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

static st_smi
set_scan_for_object (st_oop set, st_oop object)
{
    st_smi finish = ARRAY_SIZE (ARRAY (set));
    st_smi start = (st_object_hash (object) % finish) + 1;

    for (st_smi i = start; i <= finish; i++) {

	st_oop element = st_array_at (ARRAY (set), i);

	if (element == st_nil)
	    return i;
	if (st_object_equal (object, element))
	    return i;
    }

    for (st_smi i = 1; i <= (start - 1); i++) {

	st_oop element = st_array_at (ARRAY (set), i);

	if (element == st_nil)
	    return i;
	if (st_object_equal (object, element))
	    return i;

    }

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
    st_smi index = collection_find_element_or_nil (set, object);

    st_oop element = st_array_at (ARRAY (set), index);

    if (element == st_nil) {

	collection_at_new_index_put (set, index, object);

    }
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
