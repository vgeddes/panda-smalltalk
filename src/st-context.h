/*
 * st-context.h
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

#ifndef __ST_CONTEXT_H__
#define __ST_CONTEXT_H__

#include <st-types.h>
#include <st-object.h>

struct st_context_part
{
    struct st_header header;

    st_oop sender;
    st_oop ip;
    st_oop sp;

};

struct st_method_context
{
    struct st_context_part parent;
    st_oop method;   
    st_oop receiver;

    st_oop stack[];
};

struct st_block_context
{
    struct st_context_part parent;
    st_oop initial_ip;
    st_oop argcount;
    st_oop caller;
    st_oop home;

    st_oop stack[];
};


st_oop  st_method_context_new (st_oop sender, st_oop receiver, st_oop method);

st_oop  st_block_context_new              (st_oop home,
					   st_uint initial_ip,
					   st_uint argcount);

st_oop  st_message_new (st_oop selector, st_oop arguments);

#define ST_CONTEXT_PART(oop)       ((struct st_context_part *)   ST_POINTER (oop))
#define ST_METHOD_CONTEXT(oop)     ((struct st_method_context *) ST_POINTER (oop))
#define ST_BLOCK_CONTEXT(oop)      ((struct st_block_context *)  ST_POINTER (oop))

#define ST_METHOD_CONTEXT_TEMPORARY_FRAME(oop) (ST_METHOD_CONTEXT (oop)->stack)

#define ST_METHOD_CONTEXT_STACK(oop)					\
    (ST_METHOD_CONTEXT (oop)->stack					\
     + st_method_get_temp_count (ST_METHOD_CONTEXT (oop)->method)	\
     + st_method_get_arg_count (ST_METHOD_CONTEXT (oop)->method))

#define ST_METHOD_CONTEXT_STACK_SIZE(oop)				\
    (st_method_get_temp_count (ST_METHOD_CONTEXT (oop)->method)		\
     + st_method_get_arg_count (ST_METHOD_CONTEXT (oop)->method)	\
     + st_method_get_stack_depth (ST_METHOD_CONTEXT (oop)->method))

#define ST_BLOCK_CONTEXT_STACK_SIZE(oop) \
    (st_method_get_stack_depth (ST_METHOD_CONTEXT (ST_BLOCK_CONTEXT (oop)->home)->method) \
     + st_smi_value (ST_BLOCK_CONTEXT (oop)->argcount))

#define ST_MESSAGE_SELECTOR(oop) (ST_HEADER (oop)->fields[0])
#define ST_MESSAGE_ARGUMENTS(oop) (ST_HEADER (oop)->fields[1])


st_descriptor *st_context_descriptor (void);

#endif /* __ST_CONTEXT_H__ */
