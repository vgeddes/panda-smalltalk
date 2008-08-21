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
#include <st-machine.h>

#define ST_NIL                        __machine.globals[0]
#define ST_TRUE                       __machine.globals[1]
#define ST_FALSE                      __machine.globals[2]
#define ST_SYMBOLS                    __machine.globals[3]
#define ST_GLOBALS                    __machine.globals[4]
#define ST_SMALLTALK                  __machine.globals[5]

#define ST_UNDEFINED_OBJECT_CLASS     __machine.globals[6]
#define ST_METACLASS_CLASS            __machine.globals[7]
#define ST_BEHAVIOR_CLASS             __machine.globals[8]
#define ST_SMI_CLASS                  __machine.globals[9]
#define ST_LARGE_INTEGER_CLASS        __machine.globals[10]
#define ST_FLOAT_CLASS                __machine.globals[11]
#define ST_CHARACTER_CLASS            __machine.globals[12]
#define ST_TRUE_CLASS                 __machine.globals[13]
#define ST_FALSE_CLASS                __machine.globals[14]
#define ST_ARRAY_CLASS                __machine.globals[15]
#define ST_BYTE_ARRAY_CLASS           __machine.globals[16]
#define ST_WORD_ARRAY_CLASS           __machine.globals[17]
#define ST_FLOAT_ARRAY_CLASS          __machine.globals[18]
#define ST_SET_CLASS                  __machine.globals[19]
#define ST_DICTIONARY_CLASS           __machine.globals[20]
#define ST_ASSOCIATION_CLASS          __machine.globals[21]
#define ST_STRING_CLASS               __machine.globals[22]
#define ST_SYMBOL_CLASS               __machine.globals[23]
#define ST_WIDE_STRING_CLASS          __machine.globals[24]
#define ST_COMPILED_METHOD_CLASS      __machine.globals[25]
#define ST_METHOD_CONTEXT_CLASS       __machine.globals[26]
#define ST_BLOCK_CONTEXT_CLASS        __machine.globals[27]
#define ST_SYSTEM_CLASS               __machine.globals[28]
#define ST_HANDLE_CLASS               __machine.globals[29]
#define ST_MESSAGE_CLASS              __machine.globals[30]
#define ST_SELECTOR_DOESNOTUNDERSTAND __machine.globals[31]
#define ST_SELECTOR_MUSTBEBOOLEAN     __machine.globals[32]
#define ST_SELECTOR_STARTUPSYSTEM     __machine.globals[33]
#define ST_SELECTOR_CANNOTRETURN      __machine.globals[34]
#define ST_SELECTOR_OUTOFMEMORY       __machine.globals[35]

#define ST_SELECTOR_PLUS       __machine.selectors[0]
#define ST_SELECTOR_MINUS      __machine.selectors[1]
#define ST_SELECTOR_LT         __machine.selectors[2]
#define ST_SELECTOR_GT         __machine.selectors[3]
#define ST_SELECTOR_LE         __machine.selectors[4]
#define ST_SELECTOR_GE         __machine.selectors[5]
#define ST_SELECTOR_EQ         __machine.selectors[6]
#define ST_SELECTOR_NE         __machine.selectors[7]
#define ST_SELECTOR_MUL        __machine.selectors[8]
#define ST_SELECTOR_DIV        __machine.selectors[9]
#define ST_SELECTOR_MOD        __machine.selectors[10]
#define ST_SELECTOR_BITSHIFT   __machine.selectors[11]
#define ST_SELECTOR_BITAND     __machine.selectors[12]
#define ST_SELECTOR_BITOR      __machine.selectors[13]
#define ST_SELECTOR_BITXOR     __machine.selectors[14]
#define ST_SELECTOR_AT         __machine.selectors[15]
#define ST_SELECTOR_ATPUT      __machine.selectors[16]
#define ST_SELECTOR_SIZE       __machine.selectors[17]
#define ST_SELECTOR_VALUE      __machine.selectors[18]
#define ST_SELECTOR_VALUE_ARG  __machine.selectors[19]
#define ST_SELECTOR_IDEQ       __machine.selectors[20]
#define ST_SELECTOR_CLASS      __machine.selectors[21]
#define ST_SELECTOR_NEW        __machine.selectors[22]
#define ST_SELECTOR_NEW_ARG    __machine.selectors[23]

extern st_memory *memory;

void   st_initialize (void);

st_oop st_global_get     (const char *name);

void   st_set_verbose_mode  (bool verbose);

bool   st_get_verbose_mode  (void) ST_GNUC_PURE;


#endif /* __ST_UNIVERSE_H__ */
