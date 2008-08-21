/*
 * st-cpu.h
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


#ifndef __ST_CPU_H__
#define __ST_CPU_H__

#include <st-types.h>
#include <setjmp.h>

/* cache size must be a power of 2 */
#define ST_METHOD_CACHE_SIZE      1024
#define ST_METHOD_CACHE_MASK      (ST_METHOD_CACHE_SIZE - 1)
#define ST_METHOD_CACHE_HASH(k,s) ((k) ^ (s))

#define ST_NUM_GLOBALS 36
#define ST_NUM_SELECTORS 24

typedef struct st_method_cache
{
    st_oop class;
    st_oop selector;
    st_oop method;

} st_method_cache;

struct st_cpu {

    st_oop  context;
    st_oop  receiver;
    st_oop  method;

    st_uchar *bytecode;

    st_oop *temps;
    st_oop *stack;

    st_oop  lookup_class;

    st_oop  message_receiver;
    st_oop  message_selector;
    int     message_argcount;
    
    st_oop  new_method;
    bool    success;

    st_uint ip;
    st_uint sp;

    jmp_buf main_loop;

    st_method_cache method_cache[ST_METHOD_CACHE_SIZE];

    st_oop globals[ST_NUM_GLOBALS];
    st_oop selectors[ST_NUM_SELECTORS];

} __cpu;

#define ST_STACK_POP(cpu)          (cpu->stack[--cpu->sp])
#define ST_STACK_PUSH(cpu, oop)    (cpu->stack[cpu->sp++] = oop)
#define ST_STACK_PEEK(cpu)         (cpu->stack[cpu->sp-1])
#define ST_STACK_UNPOP(cpu, count) (cpu->sp += count)

void   st_cpu_main               (void);
void   st_cpu_initialize         (void);
void   st_cpu_set_active_context (st_oop context);
void   st_cpu_execute_method     (void);
st_oop st_cpu_lookup_method      (st_oop class);
void   st_cpu_clear_caches       (void);

#endif /* __ST_CPU_H__ */
