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


GList *st_behavior_all_instance_variables (st_oop klass);

/* inline definitions */

#define ST_BEHAVIOR(oop)  ((STBehavior *)  ST_POINTER (oop))
#define ST_CLASS(oop)     ((STClass *)     ST_POINTER (oop))
#define ST_METACLASS(oop) ((STMetaclass *) ST_POINTER (oop))

#define st_behavior_format(oop)              (ST_BEHAVIOR (oop)->format)
#define st_behavior_instance_size(oop)       (ST_BEHAVIOR (oop)->instance_size)
#define st_behavior_superclass(oop)          (ST_BEHAVIOR (oop)->superclass)
#define st_behavior_method_dictionary(oop)   (ST_BEHAVIOR (oop)->method_dictionary)
#define st_behavior_instance_variables(oop)  (ST_BEHAVIOR (oop)->instance_variables)

#define st_class_name(oop)                   (ST_CLASS (oop)->class_name)
#define st_class_pool(oop)                   (ST_CLASS (oop)->class_pool)

#define st_metaclass_instance_class(oop)     (ST_METACLASS (oop)->instance_class)

#endif /* __ST_CLASS_H__ */
