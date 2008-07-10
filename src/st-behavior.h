/*
 * st-behavior.h
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

#ifndef __ST_BEHAVIOR_H__
#define __ST_BEHAVIOR_H__

#include <st-types.h>
#include <st-object.h>
#include <st-memory.h>
#include <st-small-integer.h>
#include <st-array.h>
#include <st-float.h>
#include <st-large-integer.h>

#define ST_BEHAVIOR(oop)  ((struct st_behavior *)  ST_POINTER (oop))
#define ST_CLASS(oop)     ((struct st_class *)     ST_POINTER (oop))
#define ST_METACLASS(oop) ((struct st_metaclass *) ST_POINTER (oop))

struct st_behavior
{
    struct st_header header;
    
    st_oop format;
    st_oop superclass;
    st_oop instance_size;
    st_oop method_dictionary;
    st_oop instance_variables;
};

struct st_class
{
    struct st_behavior parent;

    st_oop name;
};

struct st_metaclass
{
    struct st_behavior parent;

    st_oop instance_class;
};

#define ST_BEHAVIOR_FORMAT(oop)             (ST_BEHAVIOR (oop)->format)
#define ST_BEHAVIOR_SUPERCLASS(oop)         (ST_BEHAVIOR (oop)->superclass)
#define ST_BEHAVIOR_INSTANCE_SIZE(oop)      (ST_BEHAVIOR (oop)->instance_size)
#define ST_BEHAVIOR_METHOD_DICTIONARY(oop)  (ST_BEHAVIOR (oop)->method_dictionary)
#define ST_BEHAVIOR_INSTANCE_VARIABLES(oop) (ST_BEHAVIOR (oop)->instance_variables)
#define ST_CLASS_NAME(oop)                  (ST_CLASS (oop)->name)
#define ST_METACLASS_INSTANCE_CLASS(oop)    (ST_METACLASS (oop)->instance_class)


st_oop st_object_new (st_oop class);
st_oop st_object_new_arrayed (st_oop class, st_smi size);

st_list *st_behavior_all_instance_variables (st_oop class);



#endif /* __ST_BEHAVIOR_H__ */
