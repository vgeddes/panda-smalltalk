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
#include <st-heap-object.h>

typedef struct
{
    STHeader header;

    st_oop sender;
    st_oop ip;
    st_oop sp;

} STContextPart;

typedef struct
{
    STContextPart parent;
    st_oop method;   
    st_oop receiver;
    st_oop stack[];
} STMethodContext;

typedef struct
{
    STContextPart parent;
    st_oop initial_ip;
    st_oop argcount;
    st_oop caller;
    st_oop home;
    st_oop stack[];
} STBlockContext;


st_oop  st_method_context_new (st_oop method);

st_oop *st_method_context_temporary_frame (st_oop context);
st_oop *st_method_context_stack_frame     (st_oop context);

st_oop  st_block_context_new     (st_oop home, guint initial_ip, guint argcount);


st_oop  st_message_new (st_oop selector, st_oop *args, guint args_size);




#define ST_CONTEXT_PART(oop)       ((STContextPart *)   ST_POINTER (oop))
#define ST_METHOD_CONTEXT(oop)     ((STMethodContext *) ST_POINTER (oop))
#define ST_BLOCK_CONTEXT(oop)      ((STBlockContext *)  ST_POINTER (oop))

#define ST_CONTEXT_PART_SENDER(oop)      (ST_CONTEXT_PART (oop)->sender)

#define ST_CONTEXT_PART_IP(oop)          (ST_CONTEXT_PART (oop)->ip)
#define ST_CONTEXT_PART_SP(oop)          (ST_CONTEXT_PART (oop)->sp)

#define ST_METHOD_CONTEXT_METHOD(oop)    (ST_METHOD_CONTEXT (oop)->method)
#define ST_METHOD_CONTEXT_RECEIVER(oop)  (ST_METHOD_CONTEXT (oop)->receiver)
#define ST_METHOD_CONTEXT_STACK(oop)     (ST_METHOD_CONTEXT (oop)->stack)

#define ST_BLOCK_CONTEXT_INITIAL_IP(oop) (ST_BLOCK_CONTEXT (oop)->initial_ip)
#define ST_BLOCK_CONTEXT_HOME(oop)       (ST_BLOCK_CONTEXT (oop)->home)
#define ST_BLOCK_CONTEXT_ARGCOUNT(oop)   (ST_BLOCK_CONTEXT (oop)->argcount)
#define ST_BLOCK_CONTEXT_CALLER(oop)     (ST_BLOCK_CONTEXT (oop)->caller)
#define ST_BLOCK_CONTEXT_STACK(oop)      (ST_BLOCK_CONTEXT (oop)->stack)



#endif /* __ST_CONTEXT_H__ */
