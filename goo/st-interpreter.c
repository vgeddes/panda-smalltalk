
#include "st-types.h"
#include "st-compiler.h"
#include "st-universe.h"
#include "st-hashed-collection.h"
#include "st-symbol.h"
#include "st-object.h"
#include "st-class.h"
#include "st-context.h"
#include "st-primitives.h"
#include "st-method.h"
#include "st-byte-array.h"
#include "st-array.h"
#include "st-association.h"

#include <stdlib.h>
#include <setjmp.h>
#include <glib.h>


static void
create_actual_message (st_processor *processor)
{
    st_oop *elements;
    st_oop array;

    array = st_object_new_arrayed (st_array_class, processor->message_argcount);
    elements = st_array_elements (array);
    for (st_uint i = 0; i < processor->message_argcount; i++)
	elements[i] =  processor->stack[processor->sp - processor->message_argcount + i];

    processor->sp -= processor->message_argcount;

    ST_STACK_PUSH (processor, st_message_new (processor->message_selector, array));

    processor->message_selector = st_selector_doesNotUnderstand;
    processor->message_argcount = 1;
}

INLINE st_oop
lookup_method (st_processor *processor, st_oop klass)
{
    st_oop method;
    st_oop parent = klass;
    st_uint index;

    index = (klass ^ processor->message_selector) & (ST_METHOD_CACHE_SIZE - 1);

    if (processor->method_cache[index].klass    == klass &&
	processor->method_cache[index].selector == processor->message_selector)
	return processor->method_cache[index].method;

    while (parent != st_nil) {
	method = st_dictionary_at (ST_BEHAVIOR (parent)->method_dictionary, processor->message_selector);
	if (method != st_nil) {
	    index = (parent ^ processor->message_selector) & (ST_METHOD_CACHE_SIZE - 1);
	    processor->method_cache[index].klass = klass;
	    processor->method_cache[index].selector = processor->message_selector;
	    processor->method_cache[index].method = method;
	    return method;
	}
	parent = ST_BEHAVIOR (parent)->superclass;
    }

    if (processor->message_selector == st_selector_doesNotUnderstand) {
	fprintf (stderr, "no method found for #doesNotUnderstand:");
	exit(1);
    }

    create_actual_message (processor);
    
    return lookup_method (processor, klass);
}

st_oop
st_processor_lookup_method (st_processor *processor, st_oop klass)
{
    return lookup_method (processor, klass);
}

/* 
 * Creates a new method context. Parameterised by
 * @sender, @receiver, @method, and @argcount
 *
 * Message arguments are copied into the new context's temporary
 * frame. Receiver and arguments are then popped off the stack.
 *
 */
INLINE void
activate_method (st_processor *processor, st_oop method)
{
    st_oop  context;
    st_oop *arguments;
    
    context = st_method_context_new (processor->context, processor->message_receiver,  method);

    arguments = ST_METHOD_CONTEXT_TEMPORARY_FRAME (context);
    for (st_uint i = 0; i < processor->message_argcount; i++)
	arguments[i] =  processor->stack[processor->sp - processor->message_argcount + i];

    processor->sp -= processor->message_argcount + 1;

    st_processor_set_active_context (processor, context);
}

void
st_processor_execute_method (st_processor *processor, st_oop method)
{
    st_uint primitive_index;
    STMethodFlags flags;

    flags = st_method_flags (method);
    if (flags == ST_METHOD_PRIMITIVE) {
	primitive_index = st_method_primitive_index (method);
	processor->success = true;
	st_primitives[primitive_index].func (processor);
	if (G_LIKELY (processor->success))
	    return;
    }

    activate_method (processor, method);
}

void
st_processor_set_active_context (st_processor *processor,
				   st_oop  context)
{
    st_oop home;

    /* save executation state of active context */
    if (processor->context != st_nil) {
	ST_CONTEXT_PART (processor->context)->ip = st_smi_new (processor->ip);
	ST_CONTEXT_PART (processor->context)->sp = st_smi_new (processor->sp);
    }
    
    if (st_object_class (context) == st_block_context_class) {

	home = ST_BLOCK_CONTEXT (context)->home;

	processor->method   = ST_METHOD_CONTEXT (home)->method;
	processor->receiver = ST_METHOD_CONTEXT (home)->receiver;
	processor->literals = st_array_elements (st_method_literals (processor->method));
	processor->temps    = ST_METHOD_CONTEXT_TEMPORARY_FRAME (home);
	processor->stack    = ST_BLOCK_CONTEXT (context)->stack;
    } else {
	processor->method   = ST_METHOD_CONTEXT (context)->method;
	processor->receiver = ST_METHOD_CONTEXT (context)->receiver;
	processor->literals = st_array_elements (st_method_literals (processor->method));
	processor->temps    = ST_METHOD_CONTEXT_TEMPORARY_FRAME (context);
	processor->stack    = ST_METHOD_CONTEXT_STACK (context);
    }

    processor->context  = context;
    processor->sp       = st_smi_value (ST_CONTEXT_PART (context)->sp);
    processor->ip       = st_smi_value (ST_CONTEXT_PART (context)->ip);
    processor->bytecode = st_method_bytecode_bytes (processor->method);
}

void
st_processor_send_selector (st_processor *processor,
			      st_oop            selector,
			      st_uint             argcount)
{
    st_oop method;
    st_uint primitive_index;
    STMethodFlags flags;

    processor->message_argcount = argcount;
    processor->message_receiver = processor->stack[processor->sp - argcount - 1];
    processor->message_selector = selector;
    
    method = st_processor_lookup_method (processor, st_object_class (processor->message_receiver));
    
    flags = st_method_flags (method);
    if (flags == ST_METHOD_PRIMITIVE) {
	primitive_index = st_method_primitive_index (method);
	processor->success = true;
	st_primitives[primitive_index].func (processor);
	if (G_LIKELY (processor->success))
	    return;
    }

    activate_method (processor, method);
}

#define SEND_SELECTOR(processor, selector, argcount)			\
    processor->ip = ip - processor->bytecode;				\
    processor->sp = sp - processor->stack;				\
    st_processor_send_selector (processor, selector, argcount);		\
    ip = processor->bytecode + processor->ip;				\
    sp = processor->stack + processor->sp;

#define ACTIVATE_CONTEXT(processor, context_oop)			\
    processor->ip = ip - processor->bytecode;				\
    processor->sp = sp - processor->stack;				\
    st_processor_set_active_context (processor, context_oop);		\
    ip = processor->bytecode + processor->ip;				\
    sp = processor->stack + processor->sp;

#define ACTIVATE_METHOD(processor, method_oop)			\
    processor->ip = ip - processor->bytecode;			\
    processor->sp = sp - processor->stack;			\
    activate_method (processor, method_oop);			\
    ip = processor->bytecode + processor->ip;			\
    sp = processor->stack + processor->sp;

#define EXECUTE_PRIMITIVE(processor, index)			\
    processor->success = true;					\
    processor->ip = ip - processor->bytecode;			\
    processor->sp = sp - processor->stack;			\
    st_primitives[index].func (processor);			\
    ip = processor->bytecode + processor->ip;			\
    sp = processor->stack + processor->sp;

#define SEND_TEMPLATE(processor)					\
    st_oop method;							\
    st_uint  prim;							\
    STMethodFlags flags;						\
    									\
    ip += 1;								\
    									\
    method = st_processor_lookup_method (processor, st_object_class (processor->message_receiver));\
    									\
    flags = st_method_flags (method);					\
    if (flags == ST_METHOD_PRIMITIVE) {					\
	prim = st_method_primitive_index (method);			\
									\
	EXECUTE_PRIMITIVE (processor, prim);				\
	if (G_LIKELY (processor->success))				\
            NEXT ();							\
    }									\
    									\
    ACTIVATE_METHOD (processor, method);


#ifdef __GNUC__
#define COMPUTED_GOTO
#endif

#ifdef COMPUTED_GOTO
#define SWITCH(ip)                              \
static const st_pointer labels[] =		\
{						\
    NULL,                                       \
    && PUSH_TEMP,          && PUSH_INSTVAR,     \
    && PUSH_LITERAL_CONST, && PUSH_LITERAL_VAR, \
                                                \
    && STORE_LITERAL_VAR, && STORE_TEMP, && STORE_INSTVAR, \
    && STORE_POP_LITERAL_VAR, && STORE_POP_TEMP, && STORE_POP_INSTVAR, \
                                                                       \
    && PUSH_SELF, && PUSH_NIL, && PUSH_TRUE, && PUSH_FALSE,            \
                                                                       \
    && RETURN_STACK_TOP, && BLOCK_RETURN,       \
    && POP_STACK_TOP, && DUPLICATE_STACK_TOP,   \
                                                \
    && PUSH_ACTIVE_CONTEXT, && BLOCK_COPY,      \
                                                \
    && JUMP_TRUE, && JUMP_FALSE, && JUMP,       \
                                                \
    && SEND, && SEND_SUPER,                     \
                                                \
    && SEND_PLUS,	&& SEND_MINUS,          \
    && SEND_LT,     && SEND_GT,                 \
    && SEND_LE,     && SEND_GE,                 \
    && SEND_EQ,     && SEND_NE,                 \
    && SEND_MUL,    && SEND_DIV,                \
    && SEND_MOD, 	&& SEND_BITSHIFT,       \
    && SEND_BITAND,	&& SEND_BITOR,          \
    && SEND_BITXOR,                             \
                                                \
    && SEND_AT,        && SEND_AT_PUT,          \
    && SEND_SIZE,      && SEND_VALUE,           \
    && SEND_VALUE_ARG, && SEND_IDENTITY_EQ,     \
    && SEND_CLASS,     && SEND_NEW,             \
    && SEND_NEW_ARG,                            \
};                                              \
goto *labels[*ip];
#else
#define SWITCH(ip) \
start:             \
switch (*ip)
#endif

#ifdef COMPUTED_GOTO
#define CASE(OP) OP:              
#else
#define CASE(OP) case OP:
#endif

#ifdef COMPUTED_GOTO
#define NEXT() goto *labels[*ip]
#else
#define NEXT() goto start
#endif


#define STACK_PUSH(processor,value) (*sp++ = (value))
#define STACK_POP(processor)        *(--sp)
#define STACK_PEEK(processor)        sp[-1]

void
st_processor_main (st_processor *processor)
{
    register const st_uchar *ip;
    register st_oop *sp;

    if (setjmp (processor->main_loop))
	return;

    ip = processor->bytecode + processor->ip;   
    sp = processor->stack + processor->sp;

    SWITCH (ip) {

	CASE (PUSH_TEMP) {

	    STACK_PUSH (processor, processor->temps[ip[1]]);
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (PUSH_INSTVAR) {
    
	    STACK_PUSH (processor, st_heap_object_body (processor->receiver)[ip[1]]);
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (STORE_POP_INSTVAR) {
	    
	    st_heap_object_body (processor->receiver)[ip[1]] = STACK_POP (processor);
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (STORE_INSTVAR) {
    
	    st_heap_object_body (processor->receiver)[ip[1]] = STACK_PEEK (processor);
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (STORE_POP_TEMP) {
	    
	    processor->temps[ip[1]] = STACK_POP (processor);
	    
	    ip += 2;
	    NEXT ();
	}    

	CASE (STORE_TEMP) {
	    
	    processor->temps[ip[1]] = STACK_PEEK (processor);
	    
	    ip += 2;
	    NEXT ();
	}    
	
	CASE (STORE_LITERAL_VAR) {
	    
	    ST_ASSOCIATION (processor->literals[ip[1]])->value = STACK_PEEK (processor);
	    
	    ip += 2;
	    NEXT ();
	}
	    
	CASE (STORE_POP_LITERAL_VAR) {
	    
	    ST_ASSOCIATION (processor->literals[ip[1]])->value = STACK_POP (processor);
	    
	    ip += 2;
	    NEXT ();
	}
	
	CASE (PUSH_SELF) {
    
	    STACK_PUSH (processor, processor->receiver);
	    st_assert (sp[-1] == processor->receiver);

	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_TRUE) {
    
	    STACK_PUSH (processor, st_true);
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_FALSE) {
	    
	    STACK_PUSH (processor, st_false);
	
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_NIL) {
    
	    STACK_PUSH (processor, st_nil);
	    
	    ip += 1;
	    NEXT ();
	}
	
	CASE (PUSH_ACTIVE_CONTEXT) {
    
	    STACK_PUSH (processor, processor->context);
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_LITERAL_CONST) {
    
	    STACK_PUSH (processor, processor->literals[ip[1]]);
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (PUSH_LITERAL_VAR) {
	    
	    st_oop var;
	    
	    var = ST_ASSOCIATION (processor->literals[ip[1]])->value;
	    
	    STACK_PUSH (processor, var);	    
	    
	    ip += 2;
	    NEXT ();
	}
	    
	CASE (JUMP_TRUE) {
	    
	    if (STACK_PEEK (processor) == st_true) {
		(void) STACK_POP (processor);
		ip += 3 + ((ip[1] << 8) | ip[2]);
	    } else if (STACK_PEEK (processor) == st_false) {
		(void) STACK_POP (processor);
		ip += 3;
	    } else {
		ip += 3;
		SEND_SELECTOR (processor, st_selector_mustBeBoolean, 0);
	    }
	    
	    NEXT ();
	}

	CASE (JUMP_FALSE) {
	    
	    if (STACK_PEEK (processor) == st_false) {
		(void) STACK_POP (processor);
		ip += 3 + ((ip[1] << 8) | ip[2]);
	    } else if (STACK_PEEK (processor) == st_true) {
		(void) STACK_POP (processor);
		ip += 3;
	    } else {
		ip += 3;
		SEND_SELECTOR (processor, st_selector_mustBeBoolean, 0);
	    }
	    
	    NEXT ();
	}
    
	CASE (JUMP) {
	    
	    short offset =  ((ip[1] << 8) | ip[2]);    
	    
	    ip += ((offset >= 0) ? 3 : 0) + offset;
	    
	    NEXT ();    
	}
	
	CASE (SEND_PLUS) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_PLUS];	    
	    processor->message_receiver = sp[- processor->message_argcount - 1];


	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND_MINUS) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_MINUS];
	    processor->message_receiver = sp[- processor->message_argcount - 1];

	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
    
	CASE (SEND_MUL) {
	
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_MUL];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	
	}
    
	CASE (SEND_MOD) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_MOD];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	    
	}
    
	CASE (SEND_DIV) {
    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_DIV];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND_BITSHIFT) {
    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_BITSHIFT];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();	    
	}
	
	CASE (SEND_BITAND) {
    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_BITAND];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();	    
	}
	
	CASE (SEND_BITOR) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_BITOR];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();	    
	}
	
	CASE (SEND_BITXOR) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_BITOR];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND_LT) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_LT];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND_GT) {
    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_GT];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	
	    NEXT ();
	}
	
	CASE (SEND_LE) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_LE];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();	    
	}
	
	CASE (SEND_GE) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_GE];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND_CLASS) {
	    
	    processor->message_argcount = 0;
	    processor->message_selector = st_specials[ST_SPECIAL_CLASS];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	
	    NEXT ();
	}
	
	CASE (SEND_SIZE) {
	    
	    processor->message_argcount = 0;
	    processor->message_selector = st_specials[ST_SPECIAL_SIZE];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
    
	CASE (SEND_AT) {
    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_AT];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
    
	CASE (SEND_AT_PUT) {
	    
	    processor->message_argcount = 2;
	    processor->message_selector = st_specials[ST_SPECIAL_ATPUT];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND_EQ) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_EQ];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND_NE) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_NE];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	
	    NEXT ();
	}
	
	CASE (SEND_IDENTITY_EQ) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_IDEQ];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	
	    NEXT ();
	}
	
	CASE (SEND_VALUE) {
    
	    processor->message_argcount = 0;
	    processor->message_selector = st_specials[ST_SPECIAL_VALUE];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
    
	CASE (SEND_VALUE_ARG) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_VALUE_ARG];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND_NEW) {
	    
	    processor->message_argcount = 0;
	    processor->message_selector = st_specials[ST_SPECIAL_NEW];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND_NEW_ARG) {
	    
	    processor->message_argcount = 1;
	    processor->message_selector = st_specials[ST_SPECIAL_NEW_ARG];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    SEND_TEMPLATE (processor);
	    
	    NEXT ();
	}
	
	CASE (SEND) {
	    
	    st_oop method;
	    st_uint  primitive_index;
	    STMethodFlags flags;

	    processor->message_argcount = ip[1];
	    processor->message_selector = processor->literals[ip[2]];
	    processor->message_receiver = sp[- processor->message_argcount - 1];

	    ip += 3;
	    
	    method = st_processor_lookup_method (processor, st_object_class (processor->message_receiver));
	    
	    flags = st_method_flags (method);
	    if (flags == ST_METHOD_PRIMITIVE) {
		primitive_index = st_method_primitive_index (method);
		
		EXECUTE_PRIMITIVE (processor, primitive_index);
		if (G_LIKELY (processor->success))
		    NEXT ();
	    }
	    
	    ACTIVATE_METHOD (processor, method);
	    
	    NEXT ();
	}
    
	CASE (SEND_SUPER) {
	    
	    st_oop method;
	    st_oop literal_index;
	    st_uint  primitive_index;
	    STMethodFlags flags;
	    
	    processor->message_argcount = ip[1];
	    processor->message_selector = processor->literals[ip[2]];
	    processor->message_receiver = sp[- processor->message_argcount - 1];
	    
	    ip += 3;
	    
	    literal_index = st_smi_value (st_arrayed_object_size (st_method_literals (processor->method))) - 1;
	    
	    method = st_processor_lookup_method (processor, ST_BEHAVIOR (processor->literals[literal_index])->superclass);
	    
	    flags = st_method_flags (method);
	    if (flags == ST_METHOD_PRIMITIVE) {
		primitive_index = st_method_primitive_index (method);
		
		EXECUTE_PRIMITIVE (processor, primitive_index);
		if (G_LIKELY (processor->success))
		    NEXT ();
	    }
	    
	    ACTIVATE_METHOD (processor, method);
	    
	    NEXT ();
	}
	
	CASE (POP_STACK_TOP) {
	    
	    (void) STACK_POP (processor);
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (DUPLICATE_STACK_TOP) {
	    
	    STACK_PUSH (processor, STACK_PEEK (processor));
	    
	    ip += 1;
	    NEXT ();
	}
	
	CASE (BLOCK_COPY) {
	    
	    st_oop block;
	    st_oop home;
	    st_uint argcount = ip[1];
	    st_uint initial_ip;
	    
	    ip += 2;
	    
	    initial_ip = ip - processor->bytecode + 3;
	    
	    if (st_object_class (processor->context) == st_block_context_class)
		home = ST_BLOCK_CONTEXT (processor->context)->home;
	    else
		home = processor->context;
	    
	    block = st_block_context_new (home, initial_ip, argcount);
	    
	    STACK_PUSH (processor, block);
	    
	    NEXT ();
	}
	
	CASE (RETURN_STACK_TOP) {
	    
	    st_oop sender;
	    st_oop value;
	    
	    value = STACK_PEEK (processor);
	    
	    if (st_object_class (processor->context) == st_block_context_class)
		sender = ST_CONTEXT_PART (ST_BLOCK_CONTEXT (processor->context)->home)->sender;
	    else
		sender = ST_CONTEXT_PART (processor->context)->sender;
	    
	    if (sender == st_nil) {
		STACK_PUSH (processor, processor->context);
		STACK_PUSH (processor, value);
		SEND_SELECTOR (processor, st_selector_cannotReturn, 1);
		NEXT ();
	    }
	    
	    ACTIVATE_CONTEXT (processor, sender);
	    STACK_PUSH (processor, value);
    
	    NEXT ();
	}

	CASE (BLOCK_RETURN) {
	    
	    st_oop caller;
	    st_oop value;
	    
	    caller = ST_BLOCK_CONTEXT (processor->context)->caller;
	    value = STACK_PEEK (processor);
	    ACTIVATE_CONTEXT (processor, caller);
	    
	    /* push returned value onto caller's stack */
	    STACK_PUSH (processor, value);
	    st_assert (processor->context == caller);
	    
	    NEXT ();
	}
    }
}

static void
flush_cache (st_processor *processor)
{
    for (st_uint i = 0; i < ST_METHOD_CACHE_SIZE; i++) {
	processor->method_cache[i].klass    = st_nil;
	processor->method_cache[i].selector = st_nil;
	processor->method_cache[i].method   = st_nil;
    }
}

void
st_processor_initialize (st_processor *processor)
{
    st_oop context;
    st_oop method;

    /* clear contents */
    memset (processor, 0, sizeof (st_processor));
    processor->context = st_nil;
    processor->receiver = st_nil;
    processor->method = st_nil;

    processor->ip = 0;
    processor->sp = 0;
    processor->stack = NULL;

    processor->message_argcount = 0;
    processor->message_receiver = st_nil;
    processor->message_selector = st_selector_startupSystem;
    
    flush_cache (processor); 

    method = st_processor_lookup_method (processor, st_object_class (processor->message_receiver));
    st_assert (st_method_flags (method) == ST_METHOD_NORMAL);

    context = st_method_context_new (processor->context, processor->message_receiver,  method);
    st_processor_set_active_context (processor, context);
}

