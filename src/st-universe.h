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
#include <st-memory.h>
#include <st-processor.h>

extern st_oop globals[34];

#define    st_nil                        globals[0]
#define    st_true                       globals[1]
#define    st_false                      globals[2]
#define    st_symbols                    globals[3]
#define    st_globals                    globals[4]
#define    st_smalltalk                  globals[5]

#define    st_undefined_object_class     globals[6]
#define    st_metaclass_class            globals[7]
#define    st_behavior_class             globals[8]
#define    st_smi_class                  globals[9]
#define    st_large_integer_class        globals[10]
#define    st_float_class                globals[11]
#define    st_character_class            globals[12]
#define    st_true_class                 globals[13]
#define    st_false_class                globals[14]
#define    st_array_class                globals[15]
#define    st_byte_array_class           globals[16]
#define    st_word_array_class           globals[17]
#define    st_float_array_class          globals[18]
#define    st_set_class                  globals[19]
#define    st_dictionary_class           globals[20]
#define    st_association_class          globals[21]
#define    st_string_class               globals[22]
#define    st_symbol_class               globals[23]
#define    st_wide_string_class          globals[24]
#define    st_compiled_method_class      globals[25]
#define    st_method_context_class       globals[26]
#define    st_block_context_class        globals[27]
#define    st_system_class               globals[28]

#define    st_selector_doesNotUnderstand globals[29]
#define    st_selector_mustBeBoolean     globals[30]
#define    st_selector_startupSystem     globals[31]
#define    st_selector_cannotReturn      globals[32]
#define    st_selector_outOfMemory       globals[33]

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

extern st_memory *memory;

extern st_oop st_specials[ST_NUM_SPECIALS];

void   st_bootstrap_universe (void);

st_oop st_global_get (const char *name);

bool   st_verbose_mode (void) ST_GNUC_PURE;

#endif /* __ST_UNIVERSE_H__ */
