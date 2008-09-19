/*
 * st-machine.c
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

#include "st-types.h"
#include "st-compiler.h"
#include "st-universe.h"
#include "st-dictionary.h"
#include "st-symbol.h"
#include "st-object.h"
#include "st-behavior.h"
#include "st-context.h"
#include "st-primitives.h"
#include "st-method.h"
#include "st-array.h"
#include "st-association.h"
#include "st-memory.h"

#include <stdlib.h>
#include <setjmp.h>

#ifdef __GNUC__
#define HAVE_COMPUTED_GOTO
#endif

static inline st_oop
method_context_new (st_machine *machine)
{
    st_oop  context;
    st_uint temp_count;
    st_oop *stack;
    bool    large;

    temp_count = st_method_get_arg_count (machine->new_method) + st_method_get_temp_count (machine->new_method);

    context = st_memory_allocate_context ();

    ST_CONTEXT_PART_SENDER (context)     = machine->context;
    ST_CONTEXT_PART_IP (context)         = st_smi_new (0);
    ST_CONTEXT_PART_SP (context)         = st_smi_new (temp_count);
    ST_METHOD_CONTEXT_RECEIVER (context) = machine->message_receiver;
    ST_METHOD_CONTEXT_METHOD (context)   = machine->new_method;

    /* clear temporaries (and nothing above) */
    stack = ST_METHOD_CONTEXT_STACK (context);
    for (st_uint i=0; i < temp_count; i++)
	stack[i] = ST_NIL;

    return context;
}

static st_oop
block_context_new (st_machine *machine, st_uint initial_ip, st_uint argcount)
{
    st_oop  home;
    st_oop  context;
    st_oop  method;
    st_oop *stack;
    st_uint stack_size;

    stack_size = 32;
    
    context = st_memory_allocate (ST_SIZE_OOPS (struct st_block_context) + stack_size);
    if (context == 0) {
	st_memory_perform_gc ();
	context = st_memory_allocate (ST_SIZE_OOPS (struct st_block_context) + stack_size);
	st_assert (context != 0);
    }

    st_object_initialize_header (context, ST_BLOCK_CONTEXT_CLASS);
    
    if (ST_OBJECT_CLASS (machine->context) == ST_BLOCK_CONTEXT_CLASS)
	home = ST_BLOCK_CONTEXT_HOME (machine->context);
    else
	home = machine->context;

    ST_CONTEXT_PART_SENDER (context) = ST_NIL;
    ST_CONTEXT_PART_IP (context)     = st_smi_new (0);
    ST_CONTEXT_PART_SP (context)     = st_smi_new (0);

    ST_BLOCK_CONTEXT_INITIALIP (context) = st_smi_new (initial_ip);
    ST_BLOCK_CONTEXT_ARGCOUNT (context)  = st_smi_new (argcount);
    ST_BLOCK_CONTEXT_HOME (context)      = home;
    
    return context;
}

static void
create_actual_message (st_machine *machine)
{
    st_oop *elements;
    st_oop  message;
    st_oop  array;

    array = st_object_new_arrayed (ST_ARRAY_CLASS, machine->message_argcount);
    elements = st_array_elements (array);
    for (st_uint i = 0; i < machine->message_argcount; i++)
	elements[i] = machine->stack[machine->sp - machine->message_argcount + i];

    machine->sp -= machine->message_argcount;

    message = st_object_new (ST_MESSAGE_CLASS);
    if (message == 0) {
	st_memory_perform_gc ();
	message = st_object_new (ST_MESSAGE_CLASS);
	st_assert (message != 0);
    }

    ST_OBJECT_FIELDS (message)[0] = machine->message_selector;
    ST_OBJECT_FIELDS (message)[1] = array;

    ST_STACK_PUSH (machine, message);

    machine->message_selector = ST_SELECTOR_DOESNOTUNDERSTAND;
    machine->message_argcount = 1;
}

static st_oop
lookup_method (st_machine *machine, st_oop class)
{
    st_oop  method, dict, parent;
    st_uint hash;

    parent = class;
    hash = st_byte_array_hash (machine->message_selector);

    while (parent != ST_NIL) {

	st_oop el;
	st_uint mask, i;

	dict = ST_BEHAVIOR_METHOD_DICTIONARY (parent);
	mask = st_smi_value (st_arrayed_object_size (ST_OBJECT_FIELDS (dict)[2])) - 1;
	i    = (hash & mask) + 1;

	while (true) {
	    el = st_array_at (ST_OBJECT_FIELDS (dict)[2], i);
	    if (el == ST_NIL || el == (uintptr_t) ST_OBJECT_FIELDS (dict)[2])
		break;
	    if (machine->message_selector == ST_ASSOCIATION_KEY (el))
		return ST_ASSOCIATION_VALUE (el);
	    i = ((i + ST_ADVANCE_SIZE) & mask) + 1;
	}

	parent = ST_BEHAVIOR_SUPERCLASS (parent);
    }

    if (machine->message_selector == ST_SELECTOR_DOESNOTUNDERSTAND) {
	fprintf (stderr, "panda: error: no method found for #doesNotUnderstand:\n");
	exit(1);
    }

    create_actual_message (machine);

    return lookup_method (machine, class);
}

st_oop
st_machine_lookup_method (st_machine *machine, st_oop class)
{
    return lookup_method (machine, class);
}

/* 
 * Creates a new method context. Parameterised by
 * @sender, @receiver, @method, and @argcount
 *
 * Message arguments are copied into the new context's temporary
 * frame. Receiver and arguments are then popped off the stack.
 *
 */
static inline void
activate_method (st_machine *machine)
{
    st_oop  context;
    st_oop *arguments;
    
    context = method_context_new (machine);

    arguments = ST_METHOD_CONTEXT_STACK (context);
    for (st_uint i = 0; i < machine->message_argcount; i++)
	arguments[i] =  machine->stack[machine->sp - machine->message_argcount + i];

    machine->sp -= machine->message_argcount + 1;

    st_machine_set_active_context (machine, context);
}

void
st_machine_execute_method (st_machine *machine)
{
    st_uint primitive_index;
    st_method_flags flags;

    flags = st_method_get_flags (machine->new_method);
    if (flags == ST_METHOD_PRIMITIVE) {
	primitive_index = st_method_get_primitive_index (machine->new_method);
	machine->success = true;
	st_primitives[primitive_index].func (machine);
	if (ST_LIKELY (machine->success))
	    return;
    }
    
    activate_method (machine);
}

void
st_machine_set_active_context (st_machine *machine, st_oop context)
{	 
    st_oop home;

    /* save executation state of active context */
    if (ST_UNLIKELY (machine->context != ST_NIL)) {
	ST_CONTEXT_PART_IP (machine->context) = st_smi_new (machine->ip);
	ST_CONTEXT_PART_SP (machine->context) = st_smi_new (machine->sp);
    }
    
    if (ST_OBJECT_CLASS (context) == ST_BLOCK_CONTEXT_CLASS) {
	home = ST_BLOCK_CONTEXT_HOME (context);
	machine->method   = ST_METHOD_CONTEXT_METHOD (home);
	machine->receiver = ST_METHOD_CONTEXT_RECEIVER (home);
	machine->temps    = ST_METHOD_CONTEXT_STACK (home);
	machine->stack    = ST_BLOCK_CONTEXT_STACK (context);
    } else {
	machine->method   = ST_METHOD_CONTEXT_METHOD (context);
	machine->receiver = ST_METHOD_CONTEXT_RECEIVER (context);
	machine->temps    = ST_METHOD_CONTEXT_STACK (context);
	machine->stack    = ST_METHOD_CONTEXT_STACK (context);
    }

    machine->context  = context;
    machine->sp       = st_smi_value (ST_CONTEXT_PART_SP (context));
    machine->ip       = st_smi_value (ST_CONTEXT_PART_IP (context));
    machine->bytecode = st_method_bytecode_bytes (machine->method);
}

#define SEND_SELECTOR(selector, argcount)				\
    machine->message_argcount = argcount;				\
    machine->message_receiver = sp[- argcount - 1];			\
    machine->message_selector = selector;				\
    goto send_common;
              
#define SEND_TEMPLATE()							\
    machine->lookup_class = st_object_class (machine->message_receiver);	\
    ip += 1;								\
    goto common;

#ifdef HAVE_COMPUTED_GOTO
#define SWITCH(ip)							\
static const st_pointer labels[] =					\
{									\
    && PUSH_TEMP,							\
    && PUSH_INSTVAR,							\
    && PUSH_LITERAL_CONST,						\
    && PUSH_LITERAL_VAR,						\
    && STORE_LITERAL_VAR,						\
    && STORE_TEMP,							\
    && STORE_INSTVAR,							\
    && STORE_POP_LITERAL_VAR,						\
    && STORE_POP_TEMP,							\
    && STORE_POP_INSTVAR,						\
    && PUSH_SELF,							\
    && PUSH_NIL,							\
    && PUSH_TRUE,							\
    && PUSH_FALSE,							\
    && PUSH_INTEGER,							\
    && RETURN_STACK_TOP,						\
    && BLOCK_RETURN,							\
    && POP_STACK_TOP,							\
    && DUPLICATE_STACK_TOP,						\
    && PUSH_ACTIVE_CONTEXT,						\
    && BLOCK_COPY,							\
    && JUMP_TRUE,							\
    && JUMP_FALSE,							\
    && JUMP,								\
    && SEND,								\
    && SEND_SUPER,							\
    && SEND_PLUS,							\
    && SEND_MINUS,							\
    && SEND_LT,								\
    && SEND_GT,								\
    && SEND_LE,								\
    && SEND_GE,								\
    && SEND_EQ,								\
    && SEND_NE,								\
    && SEND_MUL,							\
    && SEND_DIV,							\
    && SEND_MOD,							\
    && SEND_BITSHIFT,							\
    && SEND_BITAND,							\
    && SEND_BITOR,							\
    && SEND_BITXOR,							\
    && SEND_AT,								\
    && SEND_AT_PUT,							\
    && SEND_SIZE,							\
    && SEND_VALUE,							\
    && SEND_VALUE_ARG,							\
    && SEND_IDENTITY_EQ,						\
    && SEND_CLASS,							\
    && SEND_NEW,							\
    && SEND_NEW_ARG,							\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,	        \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,	        \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,	        \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,	        \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,	        \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,    	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,	        \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,	        \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,	        \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,   	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID, 	\
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,         \
    && INVALID, && INVALID, && INVALID, && INVALID, && INVALID,		\
    && INVALID,								\
};									\
goto *labels[*ip];
#else
#define SWITCH(ip) \
start:             \
switch (*ip)
#endif

#ifdef HAVE_COMPUTED_GOTO
#define INVALID() INVALID:
#define CASE(OP) OP:              
#else
#define INVALID() default:
#define CASE(OP) case OP:
#endif

#ifdef HAVE_COMPUTED_GOTO
#define NEXT() goto *labels[*ip]
#else
#define NEXT() goto start
#endif

static inline void
install_method_in_cache (st_machine *machine)
{
    st_uint index;

    index = ST_METHOD_CACHE_HASH (machine->lookup_class, machine->message_selector) & ST_METHOD_CACHE_MASK;
    machine->method_cache[index].class    = machine->lookup_class;
    machine->method_cache[index].selector = machine->message_selector;
    machine->method_cache[index].method   = machine->new_method;
}

static inline bool
lookup_method_in_cache (st_machine  *machine)
{
    st_uint index;

    index = ST_METHOD_CACHE_HASH (machine->lookup_class, machine->message_selector) & ST_METHOD_CACHE_MASK;
    if (machine->method_cache[index].class    == machine->lookup_class && 
	machine->method_cache[index].selector == machine->message_selector) {
	machine->new_method = machine->method_cache[index].method;
	return true;
    }
    return false;
}

#define STACK_POP(oop)     (*--sp)
#define STACK_PUSH(oop)    (*sp++ = (oop))
#define STACK_PEEK(oop)    (*(sp-1))
#define STACK_UNPOP(count) (sp += count)
#define STORE_REGISTERS()						\
    machine->ip = ip - machine->bytecode;				\
    machine->sp = sp - machine->stack;					\
    ST_CONTEXT_PART_IP (machine->context) = st_smi_new (machine->ip);   \
    ST_CONTEXT_PART_SP (machine->context) = st_smi_new (machine->sp);
#define LOAD_REGISTERS()						\
    ip = machine->bytecode + machine->ip;				\
    sp = machine->stack + machine->sp;


void
st_machine_main (st_machine *machine)
{
    register const st_uchar *ip;
    register st_oop *sp = machine->stack;

    if (setjmp (machine->main_loop))
	goto out;

    ip = machine->bytecode + machine->ip;

    SWITCH (ip) {
	
	CASE (PUSH_TEMP) {

	    STACK_PUSH (machine->temps[ip[1]]);
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (PUSH_INSTVAR) {
    
	    STACK_PUSH (ST_OBJECT_FIELDS (machine->receiver)[ip[1]]);

	    ip += 2;
	    NEXT ();
	}

	CASE (STORE_POP_INSTVAR) {
	    
	    ST_OBJECT_FIELDS (machine->receiver)[ip[1]] = STACK_POP ();
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (STORE_INSTVAR) {
    
	    ST_OBJECT_FIELDS (machine->receiver)[ip[1]] = STACK_PEEK ();
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (STORE_POP_TEMP) {
	    
	    machine->temps[ip[1]] = STACK_POP ();
	    
	    ip += 2;
	    NEXT ();
	}    

	CASE (STORE_TEMP) {
	    
	    machine->temps[ip[1]] = STACK_PEEK ();
	    
	    ip += 2;
	    NEXT ();
	}    
	
	CASE (STORE_LITERAL_VAR) {
	    
	   
	    ST_ASSOCIATION_VALUE (st_array_elements (ST_METHOD_LITERALS (machine->method))[ip[1]]) = STACK_PEEK ();

	    ip += 2;
	    NEXT ();
	}
	    
	CASE (STORE_POP_LITERAL_VAR) {
	    
	    ST_ASSOCIATION_VALUE (st_array_elements (ST_METHOD_LITERALS (machine->method))[ip[1]]) = STACK_POP ();
	    
	    ip += 2;
	    NEXT ();
	}
	
	CASE (PUSH_SELF) {
    
	    STACK_PUSH (machine->receiver);

	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_TRUE) {
    
	    STACK_PUSH (ST_TRUE);
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_FALSE) {
	    
	    STACK_PUSH (ST_FALSE);
	
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_NIL) {
    
	    STACK_PUSH (ST_NIL);
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_INTEGER) {
    
	    STACK_PUSH (st_smi_new ((signed char) ip[1]));
	    
	    ip += 2;
	    NEXT ();
	}
	
	CASE (PUSH_ACTIVE_CONTEXT) {
    
	    STACK_PUSH (machine->context);
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_LITERAL_CONST) {
    
	    STACK_PUSH (st_array_elements (ST_METHOD_LITERALS (machine->method))[ip[1]]);
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (PUSH_LITERAL_VAR) {
	    
	    st_oop var;
	    
	    var = ST_ASSOCIATION_VALUE (st_array_elements (ST_METHOD_LITERALS (machine->method))[ip[1]]);
	    
	    STACK_PUSH (var);	    
	    
	    ip += 2;
	    NEXT ();
	}
	    
	CASE (JUMP_TRUE) {
	    
	    if (STACK_PEEK () == ST_TRUE) {
		(void) STACK_POP ();
		ip += *((unsigned short *) (ip + 1)) + 3;
	    } else if (ST_LIKELY (STACK_PEEK () == ST_FALSE)) {
		(void) STACK_POP ();
		ip += 3;
	    } else {
		ip += 3;
		SEND_SELECTOR (ST_SELECTOR_MUSTBEBOOLEAN, 0);
	    }

	    NEXT ();
	    
	}

	CASE (JUMP_FALSE) {
	    
	    if (STACK_PEEK () == ST_FALSE) {
		(void) STACK_POP ();
		ip += *((unsigned short *) (ip + 1)) + 3;
	    } else if (ST_LIKELY (STACK_PEEK () == ST_TRUE)) {
		(void) STACK_POP ();
		ip += 3;
	    } else {
		ip += 3;
		SEND_SELECTOR (ST_SELECTOR_MUSTBEBOOLEAN, 0);
	    }
	   
	    NEXT ();
	}
    
	CASE (JUMP) {
	    
	    ip += *((short *) (ip + 1)) + 3;
	    NEXT ();    
	}
	
	CASE (SEND_PLUS) {
	    
	    int a, b, result;
	    
	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = st_smi_value (sp[-1]);
		a = st_smi_value (sp[-2]);
		result = a + b;
		if (((result << 1) ^ (result << 2)) >= 0) {
		    sp -= 2;
		    STACK_PUSH (st_smi_new (result));
		    ip++;
		    NEXT ();
		}
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_PLUS;	    
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND_MINUS) {

	    int a, b, result;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = st_smi_value (sp[-1]);
		a = st_smi_value (sp[-2]);
		result = a - b;
		if (((result << 1) ^ (result << 2)) >= 0) {
		    sp -= 2;
		    STACK_PUSH (st_smi_new (result));
		    ip++;
		    NEXT ();
		} else {
		    STACK_UNPOP (2);
		}
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_MINUS;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
    
	CASE (SEND_MUL) {

	    int a, b;
	    int64_t result;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = st_smi_value (sp[-1]);
		a = st_smi_value (sp[-2]);
		result = a * b;
		if (result >= ST_SMALL_INTEGER_MIN && result <= ST_SMALL_INTEGER_MAX) {
		    sp -= 2;
		    STACK_PUSH (st_smi_new ((int)result));
		    ip++;
		    NEXT ();
		}
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_MUL;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	
	}
    
	CASE (SEND_MOD) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_new (st_smi_value (a) % st_smi_value (b)));
		ip++;
		NEXT ();
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_MOD;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
    
	CASE (SEND_DIV) {

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_DIV;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND_BITSHIFT) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		if (st_smi_value (b) < 0) {
		    STACK_PUSH (st_smi_new (st_smi_value (a) >> -st_smi_value (b)));
		} else
		    STACK_PUSH (st_smi_new (st_smi_value (a) << st_smi_value (b)));
		ip++;
		NEXT ();
	    }
    
	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_BITSHIFT;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;	    
	}
	
	CASE (SEND_BITAND) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_new (st_smi_value (a) & st_smi_value (b)));
		ip++;
		NEXT ();
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_BITAND;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;  
	}
	
	CASE (SEND_BITOR) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_new (st_smi_value (a) | st_smi_value (b)));
		ip++;
		NEXT ();
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_BITOR;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	    
	}
	
	CASE (SEND_BITXOR) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_new (st_smi_value (a) ^ st_smi_value (b)));
		ip++;
		NEXT ();
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_BITXOR;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND_LT) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_value (a) < st_smi_value (b) ? ST_TRUE : ST_FALSE);
		ip++;
		NEXT ();
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_LT;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND_GT) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_value (a) > st_smi_value (b) ? ST_TRUE : ST_FALSE);
		ip++;
		NEXT ();
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_GT;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;

	}
	
	CASE (SEND_LE) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_value (a) <= st_smi_value (b) ? ST_TRUE : ST_FALSE);
		ip++;
		NEXT ();
	    }
	    
	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_LE;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;    
	}
	
	CASE (SEND_GE) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_value (a) >= st_smi_value (b) ? ST_TRUE : ST_FALSE);
		ip++;
		NEXT ();
	    }

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_GE;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common; 

	}
	
	CASE (SEND_CLASS) {

	    machine->message_argcount = 0;
	    machine->message_selector = ST_SELECTOR_CLASS;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common; 
	}
	
	CASE (SEND_SIZE) {
	    
	    machine->message_argcount = 0;
	    machine->message_selector = ST_SELECTOR_SIZE;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
    
	CASE (SEND_AT) {

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_AT;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
    
	CASE (SEND_AT_PUT) {
	    
	    machine->message_argcount = 2;
	    machine->message_selector = ST_SELECTOR_ATPUT;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND_EQ) {

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_EQ;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND_NE) {

	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_NE;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND_IDENTITY_EQ) {

	    st_oop a, b;
	    a = STACK_POP ();
	    b = STACK_POP ();

	    STACK_PUSH ((a == b) ? ST_TRUE : ST_FALSE);

	    ip += 1;
	    NEXT ();
	}
	
	CASE (SEND_VALUE) {

	    machine->message_argcount = 0;
	    machine->message_selector = ST_SELECTOR_VALUE;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
    
	CASE (SEND_VALUE_ARG) {
	    
	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_VALUE_ARG;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND_NEW) {
    
	    machine->message_argcount = 0;
	    machine->message_selector = ST_SELECTOR_NEW;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND_NEW_ARG) {
	    
	    machine->message_argcount = 1;
	    machine->message_selector = ST_SELECTOR_NEW_ARG;
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 1;
	    goto send_common;
	}
	
	CASE (SEND) {
	    
	    st_uint primitive_index;
	    st_method_flags flags;
	    st_oop  context;
	    st_oop *arguments;

	    machine->message_argcount = ip[1];
	    machine->message_selector = st_array_elements (ST_METHOD_LITERALS (machine->method))[ip[2]];
	    machine->message_receiver = sp[- machine->message_argcount - 1];
	    machine->lookup_class = st_object_class (machine->message_receiver);
	    ip += 3;
	    
	send_common:

	    if (!lookup_method_in_cache (machine)) {
		STORE_REGISTERS ();
		machine->new_method = lookup_method (machine, machine->lookup_class);
		LOAD_REGISTERS ();
		install_method_in_cache (machine);
	    }
	    
	    flags = st_method_get_flags (machine->new_method);
	    if (flags == ST_METHOD_PRIMITIVE) {
		primitive_index = st_method_get_primitive_index (machine->new_method);

		machine->success = true;
		STORE_REGISTERS ();
		st_primitives[primitive_index].func (machine);
		LOAD_REGISTERS ();

		if (ST_LIKELY (machine->success))
		    NEXT ();
	    }
	    
	    /* store registers as a gc could occur */
	    STORE_REGISTERS ();
	    context = method_context_new (machine);
	    LOAD_REGISTERS ();
	    arguments = ST_METHOD_CONTEXT_STACK (context);
	    for (int i = 0; i < machine->message_argcount; i++)
		arguments[i] =  sp[- machine->message_argcount + i];
	    sp -= machine->message_argcount + 1;

	    STORE_REGISTERS ();
	    st_machine_set_active_context (machine, context);
	    LOAD_REGISTERS ();

	    /* We have to nil these fields here. Its possible that
               that the objects they reference may be zapped by the gc.
               Another GC invocation may try to remap these fields not knowing that
               the references are invalid. 	       
	       FIXME: move this nilling out of such a critical execution path */
	    machine->message_receiver = ST_NIL;
	    machine->message_selector = ST_NIL;

	    NEXT ();
	}

	CASE (SEND_SUPER) {
	    
	    st_oop index;

	    machine->message_argcount = ip[1];
	    machine->message_selector = st_array_elements (ST_METHOD_LITERALS (machine->method))[ip[2]];
	    machine->message_receiver = sp[- machine->message_argcount - 1];

	    index = st_smi_value (st_arrayed_object_size (ST_METHOD_LITERALS (machine->method))) - 1;
	    machine->lookup_class = ST_BEHAVIOR_SUPERCLASS (st_array_elements (ST_METHOD_LITERALS (machine->method))[index]);

	    ip += 3;

	    goto send_common;
	}
	
	CASE (POP_STACK_TOP) {
	    
	    (void) STACK_POP ();
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (DUPLICATE_STACK_TOP) {
	    
	    STACK_PUSH (STACK_PEEK ());
	    
	    ip += 1;
	    NEXT ();
	}
	
	CASE (BLOCK_COPY) {
	    
	    st_oop block;
	    st_oop home;
	    st_uint argcount = ip[1];
	    st_uint initial_ip;
	    
	    ip += 2;
	    
	    initial_ip = ip - machine->bytecode + 3;
	    
	    STORE_REGISTERS ();
	    block = block_context_new (machine, initial_ip, argcount);
	    LOAD_REGISTERS ();

	    STACK_PUSH (block);
	    
	    NEXT ();
	}
	
	CASE (RETURN_STACK_TOP) {
	    
	    st_oop sender;
	    st_oop value;
	    st_oop home;
	    
	    value = STACK_PEEK ();
	    
	    if (ST_OBJECT_CLASS (machine->context) == ST_BLOCK_CONTEXT_CLASS)
		sender = ST_CONTEXT_PART_SENDER (ST_BLOCK_CONTEXT_HOME (machine->context));
	    else {
		sender = ST_CONTEXT_PART_SENDER (machine->context);
		st_memory_recycle_context (machine->context);
	    }

	    if (ST_UNLIKELY (sender == ST_NIL)) {
		STACK_PUSH (machine->context);
		STACK_PUSH (value);
		SEND_SELECTOR (ST_SELECTOR_CANNOTRETURN, 1);
		NEXT ();
	    }		

	    st_machine_set_active_context (machine, sender);
	    LOAD_REGISTERS ();
	    STACK_PUSH (value);

	    NEXT ();
	}

	CASE (BLOCK_RETURN) {
	    
	    st_oop caller;
	    st_oop value;
	    st_oop home;
	    
	    caller = ST_CONTEXT_PART_SENDER (machine->context);
	    value = STACK_PEEK ();

	    st_machine_set_active_context (machine, caller);
	    LOAD_REGISTERS ();
	    STACK_PUSH (value);
	    
	    NEXT ();
	}
	
	INVALID () {
	    abort ();
	}

    }

out:
    st_log ("gc", "totalPauseTime: %.6fs\n",
	    st_timespec_to_double_seconds (&memory->total_pause_time));
}

void
st_machine_clear_caches (st_machine *machine)
{
    memset (machine->method_cache, 0, ST_METHOD_CACHE_SIZE * 3 * sizeof (st_oop));
}

void
st_machine_initialize (st_machine *machine)
{
    st_oop context;
    st_oop method;

    /* clear contents */
    machine->context  = ST_NIL;
    machine->receiver = ST_NIL;
    machine->method   = ST_NIL;

    machine->sp = 0;
    machine->ip = 0;
    machine->stack = NULL;

    st_machine_clear_caches (machine);

    machine->message_argcount = 0;
    machine->message_receiver = ST_SMALLTALK;
    machine->message_selector = ST_SELECTOR_STARTUPSYSTEM;

    machine->new_method = lookup_method (machine, st_object_class (machine->message_receiver));
    st_assert (st_method_get_flags (machine->new_method) == ST_METHOD_NORMAL);

    context = method_context_new (machine);
    st_machine_set_active_context (machine, context);
}

