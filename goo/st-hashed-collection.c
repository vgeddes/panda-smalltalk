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

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <st-hashed-collection.h>
#include <st-array.h>
#include <st-small-integer.h>
#include <st-utils.h>
#include <st-universe.h>
#include <st-association.h>
#include <st-mark.h>

#include <st-mini.h>

#include <glib.h>

#define DEFAULT_CAPACITY      5
#define TALLY(collection)    (ST_HASHED_COLLECTION (collection)->tally)
#define ARRAY(collection)    (ST_HASHED_COLLECTION (collection)->array)
#define ARRAY_SIZE(array)    (st_smi_value (st_array_size (array)))

#define ST_HASHED_COLLECTION(oop) ((st_hashed_collection_t *) (ST_POINTER (oop)))

typedef struct
{
	st_header_t header;
	
	st_oop_t tally;
	st_oop_t array;
	
} st_hashed_collection_t;


/* Hashed Collection methods */

static st_smi_t
collection_find_element_or_nil (st_oop_t collection, st_oop_t object)
{
	st_smi_t index;
	
	index = st_object_virtual (collection)->scan_for_object (collection, object);
	
	if (index > 0)
		return index;
	
	g_assert_not_reached();
}

static st_smi_t
collection_grow_size (st_oop_t collection)
{
	return MAX (ARRAY_SIZE (ARRAY (collection)), 2);
}

static void
collection_grow (st_oop_t collection)
{
	st_oop_t old_array = ARRAY (collection);
	
	st_smi_t new_size = ARRAY_SIZE (old_array) + collection_grow_size (collection);
	
	ARRAY (collection) = st_object_new_arrayed (st_array_class, new_size);

	st_smi_t n = ARRAY_SIZE (old_array);

	for (st_smi_t i = 1; i <= n; i++)
		if (st_array_at (old_array, i) != st_nil)
			st_object_virtual (collection)->no_check_add (collection,
								       st_array_at (old_array, i));
}

static void
collection_full_check (st_oop_t collection)
{
	st_smi_t array_size = ARRAY_SIZE (ARRAY (collection));
	st_smi_t tally = st_smi_value (TALLY (collection));
	
	if ((array_size - tally) < MAX ((array_size / 4), 1))
		collection_grow (collection);
}

static void
collection_at_new_index_put (st_oop_t collection, st_smi_t index, st_oop_t object)
{
	st_array_at_put (ARRAY (collection), index, object);
	
	TALLY (collection) = st_smi_increment (TALLY (collection));

	collection_full_check (collection);
}

static void
collection_initialize (st_oop_t collection, st_smi_t capacity)
{
	g_assert (capacity > 0);

	TALLY (collection) = st_smi_new (0);
	ARRAY (collection) = st_object_new_arrayed (st_array_class, capacity);
}




/* Dictionary methods */

ST_DEFINE_VTABLE (st_dictionary, st_heap_object_vtable ())

static st_smi_t
dict_scan_for_object (st_oop_t dict, st_oop_t object)
{
	st_smi_t finish = ARRAY_SIZE (ARRAY (dict));
	st_smi_t start = (st_object_hash (object) % finish) + 1;
	
	for (st_smi_t i = start; i <= finish; i++) {
	
		st_oop_t element = st_array_at (ARRAY(dict), i);
		
		if (element == st_nil)
			return i;	
		if (st_object_equal (object, st_association_key (element)))
			return i;
	}
	
	for (st_smi_t i = 1; i <= (start - 1); i++) {
	
		st_oop_t element = st_array_at (ARRAY(dict), i);
	
		if (element == st_nil)
			return i;	
		if (st_object_equal (object, st_association_key (element)))
			return i;	
	
	}
	
	return 0;
}


static void
dict_no_check_add (st_oop_t dict, st_oop_t object)
{
	st_array_at_put (ARRAY(dict),
				    collection_find_element_or_nil (dict, st_association_key (object)),
				    object);
}


void
st_dictionary_at_put (st_oop_t dict, st_oop_t key, st_oop_t value)
{	
	st_smi_t index = collection_find_element_or_nil (dict, key);

	st_oop_t assoc = st_array_at (ARRAY (dict), index);
	
	if (assoc == st_nil) {
	
		assoc = st_association_new (key, value);	
		collection_at_new_index_put (dict, index, assoc);
	
	} else {
		st_association_set_value (assoc, value);
	}

}

st_oop_t
st_dictionary_at (st_oop_t dict, st_oop_t key)
{
	st_smi_t index = collection_find_element_or_nil (dict, key);

	st_oop_t assoc = st_array_at (ARRAY (dict), index);

	if (assoc != st_nil)
		return st_association_value (assoc); 

	return st_nil;
}

st_oop_t
st_dictionary_new_with_capacity (st_smi_t capacity)
{
	st_oop_t dict;

	dict = st_object_new (st_dictionary_class);
	
	collection_initialize (dict, capacity);
	
	return dict;
}

st_oop_t
st_dictionary_new (void)
{
	return st_dictionary_new_with_capacity (DEFAULT_CAPACITY);
}

static bool
is_dictionary (void)
{
	return true;
}

static void
st_dictionary_vtable_init (st_vtable_t *table)
{
	table->scan_for_object = dict_scan_for_object;
	table->no_check_add    = dict_no_check_add;
	
	table->is_dictionary = is_dictionary;
}

/* Set methods */

ST_DEFINE_VTABLE (st_set, st_heap_object_vtable ())

static st_smi_t
set_scan_for_object (st_oop_t set, st_oop_t object)
{
	st_smi_t finish = ARRAY_SIZE (ARRAY (set));
	st_smi_t start = (st_object_hash (object) % finish) + 1;
	
	for (st_smi_t i = start; i <= finish; i++) {
	
		st_oop_t element = st_array_at (ARRAY(set), i);
		
		if (element == st_nil)
			return i;	
		if (st_object_equal (object, element))
			return i;
	}
	
	for (st_smi_t i = 1; i <= (start - 1); i++) {
	
		st_oop_t element = st_array_at (ARRAY(set), i);
	
		if (element == st_nil)
			return i;	
		if (st_object_equal (object, element))
			return i;	
	
	}
	
	return 0;
}

static void
set_no_check_add (st_oop_t set, st_oop_t object)
{
	st_array_at_put (ARRAY(set),
				    collection_find_element_or_nil (set, object),
				    object);
}


void
st_set_add (st_oop_t set, st_oop_t object)
{	
	st_smi_t index = collection_find_element_or_nil (set, object);

	st_oop_t element = st_array_at (ARRAY (set), index);
	
	if (element == st_nil) {
		
		collection_at_new_index_put (set, index, object);
	
	}
}

bool
st_set_includes (st_oop_t set, st_oop_t object)
{
	st_smi_t index = collection_find_element_or_nil (set, object);

	return st_array_at (ARRAY (set), index) != st_nil;
}

st_oop_t
st_set_like (st_oop_t set, st_oop_t object)
{
	st_smi_t index = collection_find_element_or_nil (set, object);

	return st_array_at (ARRAY (set), index);
}

st_oop_t
st_set_new_with_capacity (st_smi_t capacity)
{
	st_oop_t set;

	set = st_object_new (st_set_class);
	
	collection_initialize (set, capacity);
	
	return set;
}

st_oop_t
st_set_new (void)
{
	return st_set_new_with_capacity (DEFAULT_CAPACITY);
}

static bool
is_set (void)
{
	return true;
}

static void
st_set_vtable_init (st_vtable_t *table)
{
	table->scan_for_object = set_scan_for_object;
	table->no_check_add    = set_no_check_add;
	
	table->is_set = is_set;
}
