/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
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

#include <st-class.h>
#include <st-heap-object.h>
#include <st-hashed-collection.h>
#include <st-universe.h>
#include <st-symbol.h>
#include <st-object.h>
#include <st-mini.h>


/* Class */

ST_DEFINE_VTABLE (st_class, st_heap_object_vtable ())

static bool
is_class (void)
{
	return true;
}

static bool
class_verify (st_oop_t object)
{
    bool verified = true;

    /* verify header */
    if (!st_object_super_virtual (object)->verify (object))
	return false;
    
    verified = verified && (ST_CLASS_VTABLE (object) != NULL); 

    // superclass of Object is st_nil !
    st_oop_t superclass = st_behavior_superclass (object);
    if (object == st_global_get ("Object"))
	verified = verified && (superclass == st_nil);
    else
	verified = verified && st_object_is_class (superclass);

    /* Each test result is put into a var so errors can be debugged in gdb */ 
    verified = verified && st_object_is_smi        (st_behavior_instance_size (object));
    verified = verified && st_object_is_dictionary (st_behavior_method_dictionary (object));
    verified = verified && (st_object_is_array      (st_behavior_instance_variables (object)) || st_behavior_instance_variables (object) == st_nil);
    verified = verified && st_object_is_symbol     (st_class_name (object));
    verified = verified && st_object_is_dictionary (st_class_pool (object));

    return verified;
}


static void
st_class_vtable_init (st_vtable_t *table)
{
	assert_static (sizeof (st_behavior_t) == (sizeof (st_header_t) + sizeof (st_vtable_t *) + 4 * sizeof (st_oop_t)));
	assert_static (sizeof (st_class_t) == (sizeof (st_behavior_t) + 2 * sizeof (st_oop_t)));

	table->is_class = is_class;
	
	table->verify = class_verify;
}


/* Metaclass */

ST_DEFINE_VTABLE (st_metaclass, st_heap_object_vtable ())


static bool
is_metaclass (void)
{
	return true;
}

static bool
metaclass_verify (st_oop_t object)
{
    bool verified = true;

    /* verify header */
    if (!st_object_super_virtual (object)->verify (object))
	return false;
    
    verified = verified && (ST_CLASS_VTABLE (object) != NULL); 

    // superclass of 'Object class' is 'Class' is st_nil !
    st_oop_t superclass = st_behavior_superclass (object);
    if (object == st_object_class (st_global_get ("Object")))
	verified = verified && (superclass == st_global_get ("Class"));
    else
	verified = verified && st_object_is_metaclass (superclass);

    /* Each test result is put into a var so errors can be debugged in gdb */ 
    verified = verified && st_object_is_smi        (st_behavior_instance_size (object));
    verified = verified && st_object_is_dictionary (st_behavior_method_dictionary (object));
    verified = verified && (st_behavior_instance_variables (object) == st_nil);
    verified = verified && st_object_is_class      (st_metaclass_instance_class (object));

    return verified;
}

static st_oop_t
metaclass_allocate (st_oop_t klass)
{
	int object_size = sizeof (st_metaclass_t) / sizeof (st_oop_t);
	
	st_oop_t object = st_allocate_object (object_size);

	st_object_initialize_header (object, st_metaclass_class);
	
	ST_CLASS_VTABLE (object) = st_class_vtable ();

	return object;
}

static void
st_metaclass_vtable_init (st_vtable_t *table)
{
	assert_static (sizeof (st_metaclass_t) == (sizeof (st_behavior_t) + 1 * sizeof (st_oop_t)));
	
	table->is_metaclass = is_metaclass;
	
	table->allocate = metaclass_allocate;
	table->verify = metaclass_verify;
}


