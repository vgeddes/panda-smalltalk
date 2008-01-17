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
    st_header_t header;

    const st_vtable_t *vtable;

    st_oop_t superclass;
    st_oop_t instance_size;
    st_oop_t method_dictionary;
    st_oop_t instance_variables;

} st_behavior_t;

typedef struct
{
    st_behavior_t parent;

    st_oop_t class_name;
    st_oop_t class_pool;

} st_class_t;

typedef struct
{
    st_behavior_t parent;

    st_oop_t instance_class;

} st_metaclass_t;

INLINE st_oop_t st_behavior_instance_size (st_oop_t klass);

INLINE st_oop_t st_behavior_superclass (st_oop_t klass);

INLINE st_oop_t st_behavior_method_dictionary (st_oop_t klass);

INLINE st_oop_t st_behavior_instance_variables (st_oop_t klass);

INLINE void st_behavior_set_instance_size (st_oop_t klass, st_smi_t instance_size);

INLINE void st_behavior_set_superclass (st_oop_t klass, st_oop_t superclass);

INLINE void st_behavior_set_method_dictionary (st_oop_t klass, st_oop_t method_dictionary);

INLINE void st_behavior_set_instance_variables (st_oop_t klass, st_oop_t instance_variables);


INLINE st_oop_t st_class_name (st_oop_t klass);

INLINE st_oop_t st_class_pool (st_oop_t klass);

INLINE void st_class_set_name (st_oop_t klass, st_oop_t name);

INLINE void st_class_set_pool (st_oop_t klass, st_oop_t class_pool);

const st_vtable_t *st_class_vtable (void);


INLINE st_oop_t st_metaclass_instance_class (st_oop_t metaclass);

INLINE void st_metaclass_set_instance_class (st_oop_t metaclass, st_oop_t instance_class);

const st_vtable_t *st_metaclass_vtable (void);


/* inline definitions */

#define ST_BEHAVIOR(oop)      ((st_behavior_t *)  ST_POINTER (oop))

INLINE st_oop_t
st_behavior_instance_size (st_oop_t klass)
{
    return ST_BEHAVIOR (klass)->instance_size;
}

INLINE st_oop_t
st_behavior_superclass (st_oop_t klass)
{
    return ST_BEHAVIOR (klass)->superclass;
}

INLINE st_oop_t
st_behavior_method_dictionary (st_oop_t klass)
{
    return ST_BEHAVIOR (klass)->method_dictionary;
}

INLINE void
st_behavior_set_instance_size (st_oop_t klass, st_smi_t instance_size)
{
    ST_BEHAVIOR (klass)->instance_size = st_smi_new (instance_size);
}

INLINE void
st_behavior_set_superclass (st_oop_t klass, st_oop_t superclass)
{
    ST_BEHAVIOR (klass)->superclass = superclass;
}

INLINE void
st_behavior_set_method_dictionary (st_oop_t klass, st_oop_t method_dictionary)
{
    ST_BEHAVIOR (klass)->method_dictionary = method_dictionary;
}

INLINE void
st_behavior_set_instance_variables (st_oop_t klass, st_oop_t instance_variables)
{
    ST_BEHAVIOR (klass)->instance_variables = instance_variables;
}

INLINE st_oop_t
st_behavior_instance_variables (st_oop_t klass)
{
    return ST_BEHAVIOR (klass)->instance_variables;
}

#define ST_CLASS(oop)     ((st_class_t *)     ST_POINTER (oop))

INLINE void
st_class_set_name (st_oop_t klass, st_oop_t name)
{
    ST_CLASS (klass)->class_name = name;
}

INLINE st_oop_t
st_class_name (st_oop_t klass)
{
    return ST_CLASS (klass)->class_name;
}

INLINE void
st_class_set_pool (st_oop_t klass, st_oop_t class_pool)
{
    ST_CLASS (klass)->class_pool = class_pool;
}

INLINE st_oop_t
st_class_pool (st_oop_t klass)
{
    return ST_CLASS (klass)->class_pool;
}

#define ST_METACLASS(oop) ((st_metaclass_t *) ST_POINTER (oop))

INLINE void
st_metaclass_set_instance_class (st_oop_t metaclass, st_oop_t instance_class)
{
    ST_METACLASS (metaclass)->instance_class = instance_class;
}

INLINE st_oop_t
st_metaclass_instance_class (st_oop_t metaclass)
{
    return ST_METACLASS (metaclass)->instance_class;
}

#endif /* __ST_CLASS_H__ */
