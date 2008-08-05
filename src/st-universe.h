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

#define    ST_NIL                        __cpu.globals[0]
#define    ST_TRUE                       __cpu.globals[1]
#define    ST_FALSE                      __cpu.globals[2]
#define    ST_SYMBOLS                    __cpu.globals[3]
#define    ST_GLOBALS                    __cpu.globals[4]
#define    ST_SMALLTALK                  __cpu.globals[5]

#define    ST_UNDEFINED_OBJECT_CLASS     __cpu.globals[6]
#define    ST_METACLASS_CLASS            __cpu.globals[7]
#define    ST_BEHAVIOR_CLASS             __cpu.globals[8]
#define    ST_SMI_CLASS                  __cpu.globals[9]
#define    ST_LARGE_INTEGER_CLASS        __cpu.globals[10]
#define    ST_FLOAT_CLASS                __cpu.globals[11]
#define    ST_CHARACTER_CLASS            __cpu.globals[12]
#define    ST_TRUE_CLASS                 __cpu.globals[13]
#define    ST_FALSE_CLASS                __cpu.globals[14]
#define    ST_ARRAY_CLASS                __cpu.globals[15]
#define    ST_BYTE_ARRAY_CLASS           __cpu.globals[16]
#define    ST_WORD_ARRAY_CLASS           __cpu.globals[17]
#define    ST_FLOAT_ARRAY_CLASS          __cpu.globals[18]
#define    ST_SET_CLASS                  __cpu.globals[19]
#define    ST_DICTIONARY_CLASS           __cpu.globals[20]
#define    ST_ASSOCIATION_CLASS          __cpu.globals[21]
#define    ST_STRING_CLASS               __cpu.globals[22]
#define    ST_SYMBOL_CLASS               __cpu.globals[23]
#define    ST_WIDE_STRING_CLASS          __cpu.globals[24]
#define    ST_COMPILED_METHOD_CLASS      __cpu.globals[25]
#define    ST_METHOD_CONTEXT_CLASS       __cpu.globals[26]
#define    ST_BLOCK_CONTEXT_CLASS        __cpu.globals[27]
#define    ST_SYSTEM_CLASS               __cpu.globals[28]
#define    ST_HANDLE_CLASS               __cpu.globals[29]

#define    ST_SELECTOR_DOESNOTUNDERSTAND __cpu.globals[30]
#define    ST_SELECTOR_MUSTBEBOOLEAN     __cpu.globals[31]
#define    ST_SELECTOR_STARTUPSYSTEM     __cpu.globals[32]
#define    ST_SELECTOR_CANNOTRETURN      __cpu.globals[33]
#define    ST_SELECTOR_OUTOFMEMORY       __cpu.globals[34]

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

void st_set_verbosity (bool verbose);

bool st_verbose_mode (void) ST_GNUC_PURE;

void st_message (const char *format, ...);


#endif /* __ST_UNIVERSE_H__ */
