/*
 * st-class.h
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

typedef struct
{
    STHeader header;
    
    st_oop format;

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

INLINE st_oop st_behavior_format (st_oop klass);

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

guint st_class_vtable (void);


INLINE st_oop st_metaclass_instance_class (st_oop metaclass);

INLINE void st_metaclass_set_instance_class (st_oop metaclass, st_oop instance_class);

guint st_metaclass_vtable (void);


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
st_behavior_format (st_oop klass)
{
    return ST_BEHAVIOR (klass)->format;    
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


INLINE st_oop
st_behavior_set_format (st_oop klass, guint format)
{
    return ST_BEHAVIOR (klass)->format = st_smi_new (format);    
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
