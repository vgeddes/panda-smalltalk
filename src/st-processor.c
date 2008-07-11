
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

#include <stdlib.h>
#include <setjmp.h>

static st_oop 
method_context_new (st_processor *pr)
{
    st_oop  context;
    int     stack_size;
    st_oop *stack;
    bool large;

    large = st_method_get_large_context (pr->new_method);
    stack_size = large ? 32 : 12;

    context = st_memory_allocate_context (large);

    ST_CONTEXT_PART_SENDER (context)     = pr->context;
    ST_CONTEXT_PART_IP (context)         = st_smi_new (0);
    ST_CONTEXT_PART_SP (context)         = st_smi_new (0);
    ST_METHOD_CONTEXT_RECEIVER (context) = pr->message_receiver;
    ST_METHOD_CONTEXT_METHOD (context)   = pr->new_method;

    stack = ST_METHOD_CONTEXT (context)->stack;
    for (st_uint i=0; i < stack_size; i++)
	stack[i] = st_nil;

    return context;
}

static st_oop
block_context_new (st_processor *pr, st_uint initial_ip, st_uint argcount)
{
    st_oop  home;
    st_oop  context;
    st_oop  method;
    st_oop *stack;
    st_uint stack_size;

    stack_size = 32;
    
    context = st_memory_allocate (ST_SIZE_OOPS (struct st_block_context) + stack_size);
    st_object_initialize_header (context, st_block_context_class);
    st_object_set_large_context (context, true);
    
    if (ST_OBJECT_CLASS (pr->context) == st_block_context_class)
	home = ST_BLOCK_CONTEXT_HOME (pr->context);
    else
	home = pr->context;

    ST_CONTEXT_PART_SENDER (context) = st_nil;
    ST_CONTEXT_PART_IP (context)     = st_smi_new (0);
    ST_CONTEXT_PART_SP (context)     = st_smi_new (0);

    ST_BLOCK_CONTEXT_INITIALIP (context) = st_smi_new (initial_ip);
    ST_BLOCK_CONTEXT_ARGCOUNT (context)  = st_smi_new (argcount);
    ST_BLOCK_CONTEXT_CALLER (context)    = st_nil;
    ST_BLOCK_CONTEXT_HOME (context)      = home;

    stack = ST_BLOCK_CONTEXT (context)->stack;
    for (st_uint i=0; i < stack_size; i++)
	stack[i] = st_nil;
    
    return context;
}

static void
create_actual_message (st_processor *pr)
{
    st_oop *elements;
    st_oop array;

    array = st_object_new_arrayed (st_array_class, pr->message_argcount);
    elements = st_array_elements (array);
    for (st_uint i = 0; i < pr->message_argcount; i++)
	elements[i] =  pr->stack[pr_sp - pr->message_argcount + i];

    pr_sp -= pr->message_argcount;

    ST_STACK_PUSH (pr, st_message_new (pr->message_selector, array));

    pr->message_selector = st_selector_doesNotUnderstand;
    pr->message_argcount = 1;
}

static inline st_oop
lookup_method (st_processor *pr, st_oop class)
{
    st_oop method;
    st_oop parent = class;
    st_uint index;

    index = ST_METHOD_CACHE_HASH (class, pr->message_selector);

    if (pr->method_cache[index].class    == class &&
	pr->method_cache[index].selector == pr->message_selector)
	return pr->method_cache[index].method;

    while (parent != st_nil) {
	method = st_dictionary_at (ST_BEHAVIOR_METHOD_DICTIONARY (parent), pr->message_selector);
	if (method != st_nil) {
	    index = ST_METHOD_CACHE_HASH (parent, pr->message_selector);
	    pr->method_cache[index].class = class;
	    pr->method_cache[index].selector = pr->message_selector;
	    pr->method_cache[index].method = method;
	    return method;
	}
	parent = ST_BEHAVIOR_SUPERCLASS (parent);
    }

    if (pr->message_selector == st_selector_doesNotUnderstand) {
	fprintf (stderr, "no method found for #doesNotUnderstand:");
	exit(1);
    }

    create_actual_message (pr);

    return lookup_method (pr, class);
}

st_oop
st_processor_lookup_method (st_processor *pr, st_oop class)
{
    return lookup_method (pr, class);
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
activate_method (st_processor *pr)
{
    st_oop  context;
    st_oop *arguments;
    
    context = method_context_new (pr);

    arguments = ST_METHOD_CONTEXT_TEMPORARY_FRAME (context);
    for (st_uint i = 0; i < pr->message_argcount; i++)
	arguments[i] =  pr->stack[pr_sp - pr->message_argcount + i];

    pr_sp -= pr->message_argcount + 1;

    st_processor_set_active_context (pr, context);
}

void
st_processor_execute_method (st_processor *pr)
{
    st_uint primitive_index;
    st_method_flags flags;

    flags = st_method_get_flags (pr->new_method);
    if (flags == ST_METHOD_PRIMITIVE) {
	primitive_index = st_method_get_primitive_index (pr->new_method);
	pr->success = true;
	st_primitives[primitive_index].func (pr);
	if (ST_LIKELY (pr->success))
	    return;
    }
    
    activate_method (pr);
}


void
st_processor_set_active_context (st_processor *pr,
				 st_oop  context)
{
    st_oop home;

    /* save executation state of active context */
    if (pr->context != st_nil) {
	ST_CONTEXT_PART_IP (pr->context) = st_smi_new (pr_ip - st_method_bytecode_bytes (pr->method));
	ST_CONTEXT_PART_SP (pr->context) = st_smi_new (pr_sp);
    }
    
    if (st_object_class (context) == st_block_context_class) {

	home = ST_BLOCK_CONTEXT_HOME (context);

	pr->method   = ST_METHOD_CONTEXT_METHOD (home);
	pr->receiver = ST_METHOD_CONTEXT_RECEIVER (home);
	pr->literals = st_array_elements (ST_METHOD_LITERALS (pr->method));
	pr->temps    = ST_METHOD_CONTEXT_TEMPORARY_FRAME (home);
	pr->stack    = ST_BLOCK_CONTEXT_STACK (context);
    } else {
	pr->method   = ST_METHOD_CONTEXT_METHOD (context);
	pr->receiver = ST_METHOD_CONTEXT_RECEIVER (context);
	pr->literals = st_array_elements (ST_METHOD_LITERALS (pr->method));
	pr->temps    = ST_METHOD_CONTEXT_TEMPORARY_FRAME (context);
	pr->stack    = ST_METHOD_CONTEXT_STACK (context);
    }

    pr->context  = context;
    pr_sp        = st_smi_value (ST_CONTEXT_PART_SP (context));
    pr_ip        = st_method_bytecode_bytes (pr->method) + st_smi_value (ST_CONTEXT_PART_IP (context));
    pr->bytecode = st_method_bytecode_bytes (pr->method);

}

void
st_processor_prologue (st_processor *pr)
{
    if (st_verbose_mode ()) {
	fprintf (stderr, "** gc: totalPauseTime: %.6fs\n", st_timespec_to_double_seconds (&memory->total_pause_time));
    }
}

void
st_processor_send_selector (st_processor *pr,
			    st_oop        selector,
			    st_uint       argcount)
{
    st_oop method;
    st_uint primitive_index;
    st_method_flags flags;

    pr->message_argcount = argcount;
    pr->message_receiver = pr->stack[pr_sp - argcount - 1];
    pr->message_selector = selector;
    
    pr->new_method = st_processor_lookup_method (pr, st_object_class (pr->message_receiver));
    
    flags = st_method_get_flags (pr->new_method);
    if (flags == ST_METHOD_PRIMITIVE) {
	primitive_index = st_method_get_primitive_index (pr->new_method);
	pr->success = true;
	st_primitives[primitive_index].func (pr);
	if (ST_LIKELY (pr->success))
	    return;
    }

    activate_method (pr);
}

#define SEND_SELECTOR(pr, selector, argcount)				\
    st_processor_send_selector (pr, selector, argcount);		\

#define ACTIVATE_CONTEXT(pr, context)			\
    st_processor_set_active_context (pr, context);	\

#define ACTIVATE_METHOD(pr)				\
    activate_method (pr);				\

#define EXECUTE_PRIMITIVE(pr, index)			\
    pr->success = true;					\
    st_primitives[index].func (pr);

#define SEND_TEMPLATE(pr)						\
    st_uint  prim;							\
    st_method_flags flags;						\
    									\
    pr_ip += 1;								\
    									\
    pr->new_method = st_processor_lookup_method (pr, st_object_class (pr->message_receiver));\
    									\
    flags = st_method_get_flags (pr->new_method);			\
    if (flags == ST_METHOD_PRIMITIVE) {					\
	prim = st_method_get_primitive_index (pr->new_method);		\
									\
	EXECUTE_PRIMITIVE (pr, prim);					\
	if (ST_LIKELY (pr->success))					\
            NEXT ();							\
    }									\
    									\
    ACTIVATE_METHOD (pr);


#ifdef __GNUC__
#define I_HAS_COMPUTED_GOTO
#endif

#ifdef I_HAS_COMPUTED_GOTO
#define SWITCH(ip)                              \
static const st_pointer labels[] =		\
{						\
    NULL,					\
    && PUSH_TEMP,          && PUSH_INSTVAR,     \
    && PUSH_LITERAL_CONST, && PUSH_LITERAL_VAR,			\
    								\
    && STORE_LITERAL_VAR, && STORE_TEMP, && STORE_INSTVAR,	       \
    && STORE_POP_LITERAL_VAR, && STORE_POP_TEMP, && STORE_POP_INSTVAR, \
    								       \
    && PUSH_SELF, && PUSH_NIL, && PUSH_TRUE, && PUSH_FALSE, && PUSH_INTEGER, \
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

#ifdef I_HAS_COMPUTED_GOTO
#define CASE(OP) OP:              
#else
#define CASE(OP) case OP:
#endif

#ifdef I_HAS_COMPUTED_GOTO
#define NEXT() goto *labels[*pr_ip]
#else
#define NEXT() goto start
#endif

const st_uchar *pr_ip;
st_uint         pr_sp;

void
st_processor_main (st_processor *pr)
{
    if (setjmp (pr->main_loop))
	goto out;

    pr_ip = pr->bytecode;

    SWITCH (pr_ip) {

	CASE (PUSH_TEMP) {

	    ST_STACK_PUSH (pr, pr->temps[pr_ip[1]]);
	    
	    pr_ip += 2;
	    NEXT ();
	}

	CASE (PUSH_INSTVAR) {
    
	    ST_STACK_PUSH (pr, ST_OBJECT_FIELDS (pr->receiver)[pr_ip[1]]);
	    
	    pr_ip += 2;
	    NEXT ();
	}

	CASE (STORE_POP_INSTVAR) {
	    
	    ST_OBJECT_FIELDS (pr->receiver)[pr_ip[1]] = ST_STACK_POP (pr);
	    
	    pr_ip += 2;
	    NEXT ();
	}

	CASE (STORE_INSTVAR) {
    
	    ST_OBJECT_FIELDS (pr->receiver)[pr_ip[1]] = ST_STACK_PEEK (pr);
	    
	    pr_ip += 2;
	    NEXT ();
	}

	CASE (STORE_POP_TEMP) {
	    
	    pr->temps[pr_ip[1]] = ST_STACK_POP (pr);
	    
	    pr_ip += 2;
	    NEXT ();
	}    

	CASE (STORE_TEMP) {
	    
	    pr->temps[pr_ip[1]] = ST_STACK_PEEK (pr);
	    
	    pr_ip += 2;
	    NEXT ();
	}    
	
	CASE (STORE_LITERAL_VAR) {
	    
	    ST_ASSOCIATION_VALUE (pr->literals[pr_ip[1]]) = ST_STACK_PEEK (pr);
	    
	    pr_ip += 2;
	    NEXT ();
	}
	    
	CASE (STORE_POP_LITERAL_VAR) {
	    
	    ST_ASSOCIATION_VALUE (pr->literals[pr_ip[1]]) = ST_STACK_POP (pr);
	    
	    pr_ip += 2;
	    NEXT ();
	}
	
	CASE (PUSH_SELF) {
    
	    ST_STACK_PUSH (pr, pr->receiver);

	    pr_ip += 1;
	    NEXT ();
	}

	CASE (PUSH_TRUE) {
    
	    ST_STACK_PUSH (pr, st_true);
	    
	    pr_ip += 1;
	    NEXT ();
	}

	CASE (PUSH_FALSE) {
	    
	    ST_STACK_PUSH (pr, st_false);
	
	    pr_ip += 1;
	    NEXT ();
	}

	CASE (PUSH_NIL) {
    
	    ST_STACK_PUSH (pr, st_nil);
	    
	    pr_ip += 1;
	    NEXT ();
	}

	CASE (PUSH_INTEGER) {
    
	    ST_STACK_PUSH (pr, st_smi_new ((signed char) pr_ip[1]));
	    
	    pr_ip += 2;
	    NEXT ();
	}
	
	CASE (PUSH_ACTIVE_CONTEXT) {
    
	    ST_STACK_PUSH (pr, pr->context);
	    
	    pr_ip += 1;
	    NEXT ();
	}

	CASE (PUSH_LITERAL_CONST) {
    
	    ST_STACK_PUSH (pr, pr->literals[pr_ip[1]]);
	    
	    pr_ip += 2;
	    NEXT ();
	}

	CASE (PUSH_LITERAL_VAR) {
	    
	    st_oop var;
	    
	    var = ST_ASSOCIATION (pr->literals[pr_ip[1]])->value;
	    
	    ST_STACK_PUSH (pr, var);	    
	    
	    pr_ip += 2;
	    NEXT ();
	}
	    
	CASE (JUMP_TRUE) {
	    
	    if (ST_STACK_PEEK (pr) == st_true) {
		(void) ST_STACK_POP (pr);
		pr_ip += 3 + *((short *) (pr_ip + 1));
	    } else if (ST_LIKELY (ST_STACK_PEEK (pr) == st_false)) {
		(void) ST_STACK_POP (pr);
		pr_ip += 3;
	    } else {
		pr_ip += 3;
		SEND_SELECTOR (pr, st_selector_mustBeBoolean, 0);
	    }
	    
	    NEXT ();
	}

	CASE (JUMP_FALSE) {
	    
	    if (ST_STACK_PEEK (pr) == st_false) {
		(void) ST_STACK_POP (pr);
		pr_ip += 3 + *((short *) (pr_ip + 1));
	    } else if (ST_LIKELY (ST_STACK_PEEK (pr) == st_true)) {
		(void) ST_STACK_POP (pr);
		pr_ip += 3;
	    } else {
		pr_ip += 3;
		SEND_SELECTOR (pr, st_selector_mustBeBoolean, 0);
	    }
	    
	    NEXT ();
	}
    
	CASE (JUMP) {
	    
	    short offset = *((short *) (pr_ip + 1));
	    
	    pr_ip += ((offset >= 0) ? 3 : 0) + offset;
	    
	    NEXT ();    
	}
	
	CASE (SEND_PLUS) {
	    
	    st_oop a, b;
	    
	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_new (st_smi_value (a) + st_smi_value (b)));
		pr_ip++;
		NEXT ();
	    }
    
	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_PLUS];	    
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];

	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND_MINUS) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_new (st_smi_value (a) - st_smi_value (b)));
		pr_ip++;
		NEXT ();
	    }

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_MINUS];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];

	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
    
	CASE (SEND_MUL) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_new (st_smi_value (a) * st_smi_value (b)));
		pr_ip++;
		NEXT ();
	    }

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_MUL];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	
	}
    
	CASE (SEND_MOD) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_new (st_smi_value (a) % st_smi_value (b)));
		pr_ip++;
		NEXT ();
	    }

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_MOD];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	    
	}
    
	CASE (SEND_DIV) {

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_DIV];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND_BITSHIFT) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		if (st_smi_value (b) < 0) {
		    ST_STACK_PUSH (pr, st_smi_new (st_smi_value (a) >> -st_smi_value (b)));
		} else
		    ST_STACK_PUSH (pr, st_smi_new (st_smi_value (a) << st_smi_value (b)));
		pr_ip++;
		NEXT ();
	    }
    
	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_BITSHIFT];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();	    
	}
	
	CASE (SEND_BITAND) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_new (st_smi_value (a) & st_smi_value (b)));
		pr_ip++;
		NEXT ();
	    }

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_BITAND];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();	    
	}
	
	CASE (SEND_BITOR) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_new (st_smi_value (a) | st_smi_value (b)));
		pr_ip++;
		NEXT ();
	    }

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_BITOR];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();	    
	}
	
	CASE (SEND_BITXOR) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_new (st_smi_value (a) ^ st_smi_value (b)));
		pr_ip++;
		NEXT ();
	    }

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_BITXOR];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND_LT) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_value (a) < st_smi_value (b) ? st_true : st_false);
		pr_ip++;
		NEXT ();
	    }

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_LT];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND_GT) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_value (a) > st_smi_value (b) ? st_true : st_false);
		pr_ip++;
		NEXT ();
	    }

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_GT];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	
	    NEXT ();
	}
	
	CASE (SEND_LE) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_value (a) <= st_smi_value (b) ? st_true : st_false);
		pr_ip++;
		NEXT ();
	    }
	    
	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_LE];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();	    
	}
	
	CASE (SEND_GE) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (pr->stack[pr_sp - 1]) &&
			   st_object_is_smi (pr->stack[pr_sp - 2]))) {
		b = ST_STACK_POP (pr);
		a = ST_STACK_POP (pr);
		ST_STACK_PUSH (pr, st_smi_value (a) >= st_smi_value (b) ? st_true : st_false);
		pr_ip++;
		NEXT ();
	    }

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_GE];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND_CLASS) {

	    pr->message_argcount = 0;
	    pr->message_selector = st_specials[ST_SPECIAL_CLASS];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	
	    NEXT ();
	}
	
	CASE (SEND_SIZE) {
	    
	    pr->message_argcount = 0;
	    pr->message_selector = st_specials[ST_SPECIAL_SIZE];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
    
	CASE (SEND_AT) {

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_AT];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
    
	CASE (SEND_AT_PUT) {
	    
	    pr->message_argcount = 2;
	    pr->message_selector = st_specials[ST_SPECIAL_ATPUT];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND_EQ) {

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_EQ];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND_NE) {

	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_NE];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	
	    NEXT ();
	}
	
	CASE (SEND_IDENTITY_EQ) {

	    st_oop a, b;
	    a = ST_STACK_POP (pr);
	    b = ST_STACK_POP (pr);
	    
	    ST_STACK_PUSH (pr, (a == b) ? st_true : st_false);

	    pr_ip += 1;
	    NEXT ();
	}
	
	CASE (SEND_VALUE) {
    
	    pr->message_argcount = 0;
	    pr->message_selector = st_specials[ST_SPECIAL_VALUE];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
    
	CASE (SEND_VALUE_ARG) {
	    
	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_VALUE_ARG];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND_NEW) {
    
	    pr->message_argcount = 0;
	    pr->message_selector = st_specials[ST_SPECIAL_NEW];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND_NEW_ARG) {
	    
	    pr->message_argcount = 1;
	    pr->message_selector = st_specials[ST_SPECIAL_NEW_ARG];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    SEND_TEMPLATE (pr);
	    
	    NEXT ();
	}
	
	CASE (SEND) {
	    
	    st_uint  primitive_index;
	    st_method_flags flags;

	    pr->message_argcount = pr_ip[1];
	    pr->message_selector = pr->literals[pr_ip[2]];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];

	    pr_ip += 3;

	    pr->new_method = st_processor_lookup_method (pr, st_object_class (pr->message_receiver));
	    
	    flags = st_method_get_flags (pr->new_method);
	    if (flags == ST_METHOD_PRIMITIVE) {
		primitive_index = st_method_get_primitive_index (pr->new_method);
		
		EXECUTE_PRIMITIVE (pr, primitive_index);
		if (ST_LIKELY (pr->success))
		    NEXT ();
	    }
	    
	    ACTIVATE_METHOD (pr);
	    NEXT ();
	}
    
	CASE (SEND_SUPER) {
	    
	    st_oop literal_index;
	    st_uint  primitive_index;
	    st_method_flags flags;
	    
	    pr->message_argcount = pr_ip[1];
	    pr->message_selector = pr->literals[pr_ip[2]];
	    pr->message_receiver = pr->stack[pr_sp - pr->message_argcount - 1];
	    
	    pr_ip += 3;
	    
	    literal_index = st_smi_value (st_arrayed_object_size (ST_METHOD_LITERALS (pr->method))) - 1;

	    pr->new_method = st_processor_lookup_method (pr, ST_BEHAVIOR_SUPERCLASS (pr->literals[literal_index]));
	    
	    flags = st_method_get_flags (pr->new_method);
	    if (flags == ST_METHOD_PRIMITIVE) {
		primitive_index = st_method_get_primitive_index (pr->new_method);
		
		EXECUTE_PRIMITIVE (pr, primitive_index);
		if (ST_LIKELY (pr->success))
		    NEXT ();
	    }
	    
	    ACTIVATE_METHOD (pr);
	    NEXT ();
	}
	
	CASE (POP_STACK_TOP) {
	    
	    (void) ST_STACK_POP (pr);
	    
	    pr_ip += 1;
	    NEXT ();
	}

	CASE (DUPLICATE_STACK_TOP) {
	    
	    ST_STACK_PUSH (pr, ST_STACK_PEEK (pr));
	    
	    pr_ip += 1;
	    NEXT ();
	}
	
	CASE (BLOCK_COPY) {
	    
	    st_oop block;
	    st_oop home;
	    st_uint argcount = pr_ip[1];
	    st_uint initial_ip;
	    
	    pr_ip += 2;
	    
	    initial_ip = pr_ip - pr->bytecode + 3;
	    
	    block = block_context_new (pr, initial_ip, argcount);

	    ST_STACK_PUSH (pr, block);
	    
	    NEXT ();
	}
	
	CASE (RETURN_STACK_TOP) {
	    
	    st_oop sender;
	    st_oop value;
	    
	    value = ST_STACK_PEEK (pr);
	    
	    if (ST_OBJECT_CLASS (pr->context) == st_block_context_class)
		sender = ST_CONTEXT_PART_SENDER (ST_BLOCK_CONTEXT_HOME (pr->context));
	    else
		sender = ST_CONTEXT_PART_SENDER (pr->context);
	    
	    st_assert (st_object_is_heap (sender));

	    if (sender == st_nil) {
		ST_STACK_PUSH (pr, pr->context);
		ST_STACK_PUSH (pr, value);
		SEND_SELECTOR (pr, st_selector_cannotReturn, 1);
		NEXT ();
	    }

	    if (ST_OBJECT_CLASS (pr->context) == st_method_context_class)
		st_memory_recycle_context (pr->context);

	    ACTIVATE_CONTEXT (pr, sender);
	    ST_STACK_PUSH (pr, value);

	    NEXT ();
	}

	CASE (BLOCK_RETURN) {
	    
	    st_oop caller;
	    st_oop value;
	    
	    caller = ST_BLOCK_CONTEXT_CALLER (pr->context);
	    value = ST_STACK_PEEK (pr);
	    ACTIVATE_CONTEXT (pr, caller);
	    
	    /* push returned value onto caller's stack */
	    ST_STACK_PUSH (pr, value);
	    st_assert (pr->context == caller);
	    
	    NEXT ();
	}
    }

out:
    st_processor_prologue (pr);
}

void
st_processor_clear_caches (st_processor *pr)
{
    for (st_uint i = 0; i < ST_METHOD_CACHE_SIZE; i++) {
	pr->method_cache[i].class    = st_nil;
	pr->method_cache[i].selector = st_nil;
	pr->method_cache[i].method   = st_nil;
    }
}

void
st_processor_initialize (st_processor *pr)
{
    st_oop context;
    st_oop method;

    /* clear contents */
    memset (pr, 0, sizeof (st_processor));
    pr->context = st_nil;
    pr->receiver = st_nil;
    pr->method = st_nil;

    pr_sp = 0;
    pr->stack = NULL;

    st_processor_clear_caches (pr);

    pr->message_argcount = 0;
    pr->message_receiver = st_smalltalk;
    pr->message_selector = st_selector_startupSystem;

    pr->new_method = st_processor_lookup_method (pr, st_object_class (pr->message_receiver));
    st_assert (st_method_get_flags (pr->new_method) == ST_METHOD_NORMAL);

    context = method_context_new (pr);
    st_processor_set_active_context (pr, context);
}

