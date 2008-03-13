/*
 * st-universe.h
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

#ifndef __ST_UNIVERSE_H__
#define __ST_UNIVERSE_H__

#include <st-types.h>

extern st_oop
    st_nil,
    st_true,
    st_false,
    st_symbol_table,
    st_smalltalk,

    st_undefined_object_class,
    st_metaclass_class,
    st_behavior_class,
    st_smi_class,
    st_large_integer_class,
    st_float_class,
    st_character_class,
    st_true_class,
    st_false_class,
    st_array_class,
    st_byte_array_class,
    st_set_class,
    st_dictionary_class,
    st_association_class,
    st_string_class,
    st_symbol_class,
    st_compiled_method_class,
    st_compiled_block_class;


void   st_bootstrap_universe (void);

st_oop st_global_get (const char *name);



#endif /* __ST_UNIVERSE_H__ */
