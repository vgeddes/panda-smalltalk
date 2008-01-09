/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
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
 
/* Definition of Class objects
 *
 * Inspired by Strongtalk, we include a vtable in each class object
 * See the descriptions in st-mini.[ch] for an overview of the virtual dispatch
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

#define ST_CLASS_VTABLE(klass) (_ST_BEHAVIOR (klass)->vtable)

typedef struct
{
    st_header_t   header;

    st_vtable_t  *vtable;
    
    st_oop_t      superclass;
    st_oop_t      instance_size;
    st_oop_t      method_dictionary;
    st_oop_t      instance_variables;
	
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

INLINE st_oop_t   st_behavior_instance_size        (st_oop_t klass);

INLINE st_oop_t   st_behavior_superclass           (st_oop_t klass);

INLINE st_oop_t   st_behavior_method_dictionary    (st_oop_t klass);

INLINE st_oop_t   st_behavior_instance_variables   (st_oop_t klass);

INLINE void       st_behavior_set_instance_size    (st_oop_t klass, st_smi_t instance_size);

INLINE void       st_behavior_set_superclass       (st_oop_t klass, st_oop_t superclass);

INLINE void       st_behavior_set_method_dictionary  (st_oop_t klass, st_oop_t method_dictionary);

INLINE void       st_behavior_set_instance_variables (st_oop_t klass, st_oop_t instance_variables);


INLINE st_oop_t   st_class_name     (st_oop_t klass);

INLINE st_oop_t   st_class_pool     (st_oop_t klass);

INLINE void       st_class_set_name (st_oop_t klass, st_oop_t name);

INLINE void       st_class_set_pool (st_oop_t klass, st_oop_t class_pool);

st_vtable_t      *st_class_vtable   (void);


INLINE st_oop_t   st_metaclass_instance_class     (st_oop_t metaclass);

INLINE void       st_metaclass_set_instance_class (st_oop_t metaclass, st_oop_t instance_class);

st_vtable_t      *st_metaclass_vtable (void);


/* inline definitions */

#define _ST_BEHAVIOR(oop)      ((st_behavior_t *)  ST_POINTER (oop))

INLINE st_oop_t
st_behavior_instance_size (st_oop_t klass)
{
    return _ST_BEHAVIOR (klass)->instance_size;
}

INLINE st_oop_t
st_behavior_superclass (st_oop_t klass)
{
    return _ST_BEHAVIOR (klass)->superclass;
}

INLINE st_oop_t
st_behavior_method_dictionary (st_oop_t klass)
{
    return _ST_BEHAVIOR (klass)->method_dictionary;
}

INLINE void
st_behavior_set_instance_size (st_oop_t klass, st_smi_t instance_size)
{
    _ST_BEHAVIOR (klass)->instance_size = st_smi_new (instance_size);
}

INLINE void
st_behavior_set_superclass (st_oop_t klass, st_oop_t superclass)
{
    _ST_BEHAVIOR (klass)->superclass = superclass;
}

INLINE void
st_behavior_set_method_dictionary (st_oop_t klass, st_oop_t method_dictionary)
{
    _ST_BEHAVIOR (klass)->method_dictionary = method_dictionary;
}

INLINE void
st_behavior_set_instance_variables (st_oop_t klass, st_oop_t instance_variables)
{
    _ST_BEHAVIOR (klass)->instance_variables = instance_variables;
}

INLINE st_oop_t
st_behavior_instance_variables (st_oop_t klass)
{
    return _ST_BEHAVIOR (klass)->instance_variables;
}

#define _ST_CLASS(oop)     ((st_class_t *)     ST_POINTER (oop))

INLINE void
st_class_set_name (st_oop_t klass, st_oop_t name)
{
    _ST_CLASS (klass)->class_name = name;
}

INLINE st_oop_t
st_class_name (st_oop_t klass)
{
    return _ST_CLASS (klass)->class_name;
}

INLINE void
st_class_set_pool (st_oop_t klass, st_oop_t class_pool)
{
    _ST_CLASS (klass)->class_pool = class_pool;
}

INLINE st_oop_t
st_class_pool (st_oop_t klass)
{
    return _ST_CLASS (klass)->class_pool;
}

#define _ST_METACLASS(oop) ((st_metaclass_t *) ST_POINTER (oop))

INLINE void
st_metaclass_set_instance_class (st_oop_t metaclass, st_oop_t instance_class)
{
    _ST_METACLASS (metaclass)->instance_class = instance_class;
}

INLINE st_oop_t
st_metaclass_instance_class (st_oop_t metaclass)
{
    return _ST_METACLASS (metaclass)->instance_class;
}

#endif /* __ST_CLASS_H__ */
