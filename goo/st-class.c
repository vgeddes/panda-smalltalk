/*
 * st-class.c
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

#include "st-class.h"
#include "st-heap-object.h"
#include "st-hashed-collection.h"
#include "st-universe.h"
#include "st-symbol.h"
#include "st-object.h"
#include "st-byte-array.h"
#include "st-vtable.h"


/* Class */

ST_DEFINE_VTABLE (st_class, st_heap_object_vtable ());

static bool
is_class (void)
{
    return true;
}

static bool
class_verify (st_oop object)
{
    bool verified = true;

    /* verify header */
    if (!tables[st_heap_object_vtable ()].verify (object))
	return false;

    // superclass of Object is st_nil !
    st_oop superclass = st_behavior_superclass (object);
    if (object == st_global_get ("Object"))
	verified = verified && (superclass == st_nil);
    else
	verified = verified && st_object_is_class (superclass);

    /* Each test result is put into a var so errors can be debugged in gdb */
    verified = verified && st_object_is_smi (st_behavior_instance_size (object));
    verified = verified && st_object_is_dictionary (st_behavior_method_dictionary (object));
    verified = verified && (st_object_is_array (st_behavior_instance_variables (object))
			    || st_behavior_instance_variables (object) == st_nil);
    verified = verified && st_object_is_symbol (st_class_name (object));
    verified = verified && st_object_is_dictionary (st_class_pool (object));

    return verified;
}


GList *
st_behavior_all_instance_variables (st_oop klass)
{
    st_oop superclass;
    GList *a = NULL, *b = NULL;
    st_oop names;
    st_smi size;

    /* superclass of Object is nil */
    if (klass == st_nil)
	return NULL;
    
    a = st_behavior_all_instance_variables (st_behavior_superclass (klass));

    names = st_behavior_instance_variables (klass);
    if (names != st_nil) {
	size = st_smi_value (st_array_size (names));
	for (st_smi i = 1; i <= size; i++)
	    b = g_list_append (b, (gpointer) st_array_at (names, i));
    }

    return g_list_concat (a, b);
}


static void
st_class_vtable_init (STVTable * table)
{
    assert_static (sizeof (STBehavior) ==
		   (sizeof (STHeader) + 5 * sizeof (st_oop)));
    assert_static (sizeof (STClass) == (sizeof (STBehavior) + 2 * sizeof (st_oop)));

    table->is_class = is_class;

    table->verify = class_verify;
}


/* Metaclass */

ST_DEFINE_VTABLE (st_metaclass, st_heap_object_vtable ());


static bool
is_metaclass (void)
{
    return true;
}

static bool
metaclass_verify (st_oop object)
{
    bool verified = true;

    /* verify header */
    if (!tables[st_heap_object_vtable ()].verify (object))
	return false;

    // superclass of 'Object class' is 'Class' is st_nil !
    st_oop superclass = st_behavior_superclass (object);
    if (object == st_object_class (st_global_get ("Object")))
	verified = verified && (superclass == st_global_get ("Class"));
    else
	verified = verified && st_object_is_metaclass (superclass);

    /* Each test result is put into a var so errors can be debugged in gdb */
    verified = verified && st_object_is_smi (st_behavior_instance_size (object));
    verified = verified && st_object_is_dictionary (st_behavior_method_dictionary (object));
    verified = verified && (st_behavior_instance_variables (object) == st_nil);
    verified = verified && st_object_is_class (st_metaclass_instance_class (object));

    return verified;
}

static st_oop
metaclass_allocate (st_oop klass)
{
    st_oop object = st_allocate_object (ST_TYPE_SIZE (STMetaclass));

    st_object_initialize_header (object, st_metaclass_class);

    return object;
}

static void
st_metaclass_vtable_init (STVTable * table)
{
    assert_static (sizeof (STMetaclass) == (sizeof (STBehavior) + 1 * sizeof (st_oop)));

    table->is_metaclass = is_metaclass;

    table->allocate = metaclass_allocate;
    table->verify = metaclass_verify;
}
