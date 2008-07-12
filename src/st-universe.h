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
#include <st-cpu.h>

#define    st_nil                        __cpu.globals[0]
#define    st_true                       __cpu.globals[1]
#define    st_false                      __cpu.globals[2]
#define    st_symbols                    __cpu.globals[3]
#define    st_globals                    __cpu.globals[4]
#define    st_smalltalk                  __cpu.globals[5]

#define    st_undefined_object_class     __cpu.globals[6]
#define    st_metaclass_class            __cpu.globals[7]
#define    st_behavior_class             __cpu.globals[8]
#define    st_smi_class                  __cpu.globals[9]
#define    st_large_integer_class        __cpu.globals[10]
#define    st_float_class                __cpu.globals[11]
#define    st_character_class            __cpu.globals[12]
#define    st_true_class                 __cpu.globals[13]
#define    st_false_class                __cpu.globals[14]
#define    st_array_class                __cpu.globals[15]
#define    st_byte_array_class           __cpu.globals[16]
#define    st_word_array_class           __cpu.globals[17]
#define    st_float_array_class          __cpu.globals[18]
#define    st_set_class                  __cpu.globals[19]
#define    st_dictionary_class           __cpu.globals[20]
#define    st_association_class          __cpu.globals[21]
#define    st_string_class               __cpu.globals[22]
#define    st_symbol_class               __cpu.globals[23]
#define    st_wide_string_class          __cpu.globals[24]
#define    st_compiled_method_class      __cpu.globals[25]
#define    st_method_context_class       __cpu.globals[26]
#define    st_block_context_class        __cpu.globals[27]
#define    st_system_class               __cpu.globals[28]

#define    st_selector_doesNotUnderstand __cpu.globals[29]
#define    st_selector_mustBeBoolean     __cpu.globals[30]
#define    st_selector_startupSystem     __cpu.globals[31]
#define    st_selector_cannotReturn      __cpu.globals[32]
#define    st_selector_outOfMemory       __cpu.globals[33]

#define ST_SELECTOR_PLUS       __cpu.selectors[0]
#define ST_SELECTOR_MINUS      __cpu.selectors[1]
#define ST_SELECTOR_LT         __cpu.selectors[2]
#define ST_SELECTOR_GT         __cpu.selectors[3]
#define ST_SELECTOR_LE         __cpu.selectors[4]
#define ST_SELECTOR_GE         __cpu.selectors[5]
#define ST_SELECTOR_EQ         __cpu.selectors[6]
#define ST_SELECTOR_NE         __cpu.selectors[7]
#define ST_SELECTOR_MUL        __cpu.selectors[8]
#define ST_SELECTOR_DIV        __cpu.selectors[9]
#define ST_SELECTOR_MOD        __cpu.selectors[10]
#define ST_SELECTOR_BITSHIFT   __cpu.selectors[11]
#define ST_SELECTOR_BITAND     __cpu.selectors[12]
#define ST_SELECTOR_BITOR      __cpu.selectors[13]
#define ST_SELECTOR_BITXOR     __cpu.selectors[14]
#define ST_SELECTOR_AT         __cpu.selectors[15]
#define ST_SELECTOR_ATPUT      __cpu.selectors[16]
#define ST_SELECTOR_SIZE       __cpu.selectors[17]
#define ST_SELECTOR_VALUE      __cpu.selectors[18]
#define ST_SELECTOR_VALUE_ARG  __cpu.selectors[19]
#define ST_SELECTOR_IDEQ       __cpu.selectors[20]
#define ST_SELECTOR_CLASS      __cpu.selectors[21]
#define ST_SELECTOR_NEW        __cpu.selectors[22]
#define ST_SELECTOR_NEW_ARG    __cpu.selectors[23]

extern st_memory *memory;


void   st_bootstrap_universe (void);

st_oop st_global_get (const char *name);

bool   st_verbose_mode (void) ST_GNUC_PURE;

#endif /* __ST_UNIVERSE_H__ */
