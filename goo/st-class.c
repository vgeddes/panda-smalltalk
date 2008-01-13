/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-class.c
 *
 * Copyright (C) 2008 Vincent Geddes <vgeddes@gnome.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "st-class.h"
#include "st-heap-object.h"
#include "st-hashed-collection.h"
#include "st-universe.h"
#include "st-symbol.h"
#include "st-object.h"
#include "st-vtable.h"


/* Class */

ST_DEFINE_VTABLE (st_class, st_heap_object_vtable ());

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
    if (!st_heap_object_vtable ()->verify (object))
	return false;

    verified = verified && (ST_CLASS_VTABLE (object) != NULL);

    // superclass of Object is st_nil !
    st_oop_t superclass = st_behavior_superclass (object);
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


static void
st_class_vtable_init (st_vtable_t * table)
{
    assert_static (sizeof (st_behavior_t) ==
		   (sizeof (st_header_t) + sizeof (st_vtable_t *) + 4 * sizeof (st_oop_t)));
    assert_static (sizeof (st_class_t) == (sizeof (st_behavior_t) + 2 * sizeof (st_oop_t)));

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
metaclass_verify (st_oop_t object)
{
    bool verified = true;

    /* verify header */
    if (!st_heap_object_vtable ()->verify (object))
	return false;

    verified = verified && (ST_CLASS_VTABLE (object) != NULL);

    // superclass of 'Object class' is 'Class' is st_nil !
    st_oop_t superclass = st_behavior_superclass (object);
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

static st_oop_t
metaclass_allocate (st_oop_t klass)
{
    st_oop_t object = st_allocate_object (ST_TYPE_SIZE (st_metaclass_t));

    st_object_initialize_header (object, st_metaclass_class);

    ST_CLASS_VTABLE (object) = st_class_vtable ();

    return object;
}

static void
st_metaclass_vtable_init (st_vtable_t * table)
{
    assert_static (sizeof (st_metaclass_t) == (sizeof (st_behavior_t) + 1 * sizeof (st_oop_t)));

    table->is_metaclass = is_metaclass;

    table->allocate = metaclass_allocate;
    table->verify = metaclass_verify;
}
