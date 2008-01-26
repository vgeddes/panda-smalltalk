/*
 * st-class.h
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

/* Definition of Class objects
 *
 * Inspired by Strongtalk, we include a vtable in each class object
 * See the descriptions in st-vtable.[ch] for an overview of the virtual dispatch
 * mechanism. The vtable is only used in the VM layer, and allows the VM
 * to work with oops's in an object-orientated way.
 *
 */

#ifndef __ST_CLASS_H__
#define __ST_CLASS_H__

#include <st-types.h>
#include <st-heap-object.h>
#include <st-small-integer.h>
#include <glib.h>

#define ST_CLASS_VTABLE(klass) (ST_BEHAVIOR (klass)->vtable)

typedef struct
{
    STHeader header;

    const STVTable *vtable;

    st_oop superclass;
    st_oop instance_size;
    st_oop method_dictionary;
    st_oop instance_variables;

} STBehavior;

typedef struct
{
    STBehavior parent;

    st_oop class_name;
    st_oop class_pool;

} STClass;

typedef struct
{
    STBehavior parent;

    st_oop instance_class;

} STMetaclass;

INLINE st_oop st_behavior_instance_size (st_oop klass);

INLINE st_oop st_behavior_superclass (st_oop klass);

INLINE st_oop st_behavior_method_dictionary (st_oop klass);

INLINE st_oop st_behavior_instance_variables (st_oop klass);

INLINE void st_behavior_set_instance_size (st_oop klass, st_smi instance_size);

INLINE void st_behavior_set_superclass (st_oop klass, st_oop superclass);

INLINE void st_behavior_set_method_dictionary (st_oop klass, st_oop method_dictionary);

INLINE void st_behavior_set_instance_variables (st_oop klass, st_oop instance_variables);


INLINE st_oop st_class_name (st_oop klass);

INLINE st_oop st_class_pool (st_oop klass);

INLINE void st_class_set_name (st_oop klass, st_oop name);

INLINE void st_class_set_pool (st_oop klass, st_oop class_pool);

GList *st_behavior_all_instance_variables (st_oop klass);

const STVTable *st_class_vtable (void);


INLINE st_oop st_metaclass_instance_class (st_oop metaclass);

INLINE void st_metaclass_set_instance_class (st_oop metaclass, st_oop instance_class);

const STVTable *st_metaclass_vtable (void);


/* inline definitions */

#define ST_BEHAVIOR(oop)      ((STBehavior *)  ST_POINTER (oop))

INLINE st_oop
st_behavior_instance_size (st_oop klass)
{
    return ST_BEHAVIOR (klass)->instance_size;
}

INLINE st_oop
st_behavior_superclass (st_oop klass)
{
    return ST_BEHAVIOR (klass)->superclass;
}

INLINE st_oop
st_behavior_method_dictionary (st_oop klass)
{
    return ST_BEHAVIOR (klass)->method_dictionary;
}

INLINE void
st_behavior_set_instance_size (st_oop klass, st_smi instance_size)
{
    ST_BEHAVIOR (klass)->instance_size = st_smi_new (instance_size);
}

INLINE void
st_behavior_set_superclass (st_oop klass, st_oop superclass)
{
    ST_BEHAVIOR (klass)->superclass = superclass;
}

INLINE void
st_behavior_set_method_dictionary (st_oop klass, st_oop method_dictionary)
{
    ST_BEHAVIOR (klass)->method_dictionary = method_dictionary;
}

INLINE void
st_behavior_set_instance_variables (st_oop klass, st_oop instance_variables)
{
    ST_BEHAVIOR (klass)->instance_variables = instance_variables;
}

INLINE st_oop
st_behavior_instance_variables (st_oop klass)
{
    return ST_BEHAVIOR (klass)->instance_variables;
}

#define ST_CLASS(oop)     ((STClass *)     ST_POINTER (oop))

INLINE void
st_class_set_name (st_oop klass, st_oop name)
{
    ST_CLASS (klass)->class_name = name;
}

INLINE st_oop
st_class_name (st_oop klass)
{
    return ST_CLASS (klass)->class_name;
}

INLINE void
st_class_set_pool (st_oop klass, st_oop class_pool)
{
    ST_CLASS (klass)->class_pool = class_pool;
}

INLINE st_oop
st_class_pool (st_oop klass)
{
    return ST_CLASS (klass)->class_pool;
}

#define ST_METACLASS(oop) ((STMetaclass *) ST_POINTER (oop))

INLINE void
st_metaclass_set_instance_class (st_oop metaclass, st_oop instance_class)
{
    ST_METACLASS (metaclass)->instance_class = instance_class;
}

INLINE st_oop
st_metaclass_instance_class (st_oop metaclass)
{
    return ST_METACLASS (metaclass)->instance_class;
}

#endif /* __ST_CLASS_H__ */
