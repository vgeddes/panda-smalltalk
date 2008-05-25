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
    st_wide_string_class,
    st_compiled_method_class,
    st_method_context_class,
    st_block_context_class,

    st_selector_doesNotUnderstand, 
    st_selector_mustBeBoolean,
    st_selector_startupSystem,
    st_selector_cannotReturn;

enum {
    ST_SPECIAL_PLUS,
    ST_SPECIAL_MINUS,
    ST_SPECIAL_LT,
    ST_SPECIAL_GT,
    ST_SPECIAL_LE,
    ST_SPECIAL_GE,
    ST_SPECIAL_EQ,
    ST_SPECIAL_NE,
    ST_SPECIAL_MUL,
    ST_SPECIAL_DIV,
    ST_SPECIAL_MOD,
    ST_SPECIAL_BITSHIFT,
    ST_SPECIAL_BITAND,
    ST_SPECIAL_BITOR,
    ST_SPECIAL_BITXOR,
    ST_SPECIAL_AT,
    ST_SPECIAL_ATPUT,
    ST_SPECIAL_SIZE,
    ST_SPECIAL_VALUE,
    ST_SPECIAL_VALUE_ARG,
    ST_SPECIAL_IDEQ,
    ST_SPECIAL_CLASS,
    ST_SPECIAL_NEW,
    ST_SPECIAL_NEW_ARG,

    ST_NUM_SPECIALS,
};

extern st_oop st_specials[ST_NUM_SPECIALS];

void   st_bootstrap_universe (void);

st_oop st_global_get (const char *name);



#endif /* __ST_UNIVERSE_H__ */
