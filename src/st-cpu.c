
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

static inline st_oop 
method_context_new (void)
{
    register struct st_cpu *cpu = &__cpu;
    st_oop  context;
    int     stack_size;
    st_uint temp_count;
    st_oop *stack;
    bool    large;

    large = st_method_get_large_context (cpu->new_method);
    temp_count = st_method_get_arg_count (cpu->new_method) + st_method_get_temp_count (cpu->new_method);
    stack_size = large ? 32 : 12;

    context = st_memory_allocate_context (large);

    ST_CONTEXT_PART_SENDER (context)     = cpu->context;
    ST_CONTEXT_PART_IP (context)         = st_smi_new (0);
    ST_CONTEXT_PART_SP (context)         = st_smi_new (temp_count);
    ST_METHOD_CONTEXT_RECEIVER (context) = cpu->message_receiver;
    ST_METHOD_CONTEXT_METHOD (context)   = cpu->new_method;

    /* clear temporaries (and nothing above) */
    stack = ST_METHOD_CONTEXT_STACK (context);
    for (st_uint i=0; i < temp_count; i++)
	stack[i] = st_nil;

    return context;
}

static st_oop
block_context_new (st_uint initial_ip, st_uint argcount)
{
    register struct st_cpu *cpu = &__cpu;
    st_oop  home;
    st_oop  context;
    st_oop  method;
    st_oop *stack;
    st_uint stack_size;

    stack_size = 32;
    
    context = st_memory_allocate (ST_SIZE_OOPS (struct st_block_context) + stack_size);
    st_object_initialize_header (context, st_block_context_class);
    st_object_set_large_context (context, true);
    
    if (ST_OBJECT_CLASS (cpu->context) == st_block_context_class)
	home = ST_BLOCK_CONTEXT_HOME (cpu->context);
    else
	home = cpu->context;

    ST_CONTEXT_PART_SENDER (context) = st_nil;
    ST_CONTEXT_PART_IP (context)     = st_smi_new (0);
    ST_CONTEXT_PART_SP (context)     = st_smi_new (0);

    ST_BLOCK_CONTEXT_INITIALIP (context) = st_smi_new (initial_ip);
    ST_BLOCK_CONTEXT_ARGCOUNT (context)  = st_smi_new (argcount);
    ST_BLOCK_CONTEXT_CALLER (context)    = st_nil;
    ST_BLOCK_CONTEXT_HOME (context)      = home;

    /* don't nil stack, not needed */
    
    return context;
}

static void
create_actual_message (void)
{
    register struct st_cpu *cpu = &__cpu;
    st_oop *elements;
    st_oop  message;
    st_oop array;

    array = st_object_new_arrayed (st_array_class, cpu->message_argcount);
    elements = st_array_elements (array);
    for (st_uint i = 0; i < cpu->message_argcount; i++)
	elements[i] = cpu->stack[cpu->sp - cpu->message_argcount + i];

    cpu->sp -= cpu->message_argcount;
    message = st_message_new (cpu->message_selector, array);
    ST_STACK_PUSH (cpu, message);

    cpu->message_selector = st_selector_doesNotUnderstand;
    cpu->message_argcount = 1;
}

static st_oop
lookup_method (st_oop class)
{
    register struct st_cpu *cpu = &__cpu;
    st_oop method;
    st_oop parent = class;
    st_uint index;

    while (parent != st_nil) {
	method = st_dictionary_at (ST_BEHAVIOR_METHOD_DICTIONARY (parent), cpu->message_selector);
	if (method != st_nil)
	    return method;
	parent = ST_BEHAVIOR_SUPERCLASS (parent);
    }

    if (cpu->message_selector == st_selector_doesNotUnderstand) {
	fprintf (stderr, "panda: error: no method found for #doesNotUnderstand:\n");
	exit(1);
    }

    create_actual_message ();

    return lookup_method (class);
}

st_oop
st_cpu_lookup_method (st_oop class)
{
    return lookup_method (class);
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
activate_method (void)
{
    register struct st_cpu *cpu = &__cpu;
    st_oop  context;
    st_oop *arguments;
    
    context = method_context_new ();

    arguments = ST_METHOD_CONTEXT_STACK (context);
    for (st_uint i = 0; i < cpu->message_argcount; i++)
	arguments[i] =  cpu->stack[cpu->sp - cpu->message_argcount + i];

    cpu->sp -= cpu->message_argcount + 1;

    st_cpu_set_active_context (context);
}

void
st_cpu_execute_method (void)
{
    register struct st_cpu *cpu = &__cpu;
    st_uint primitive_index;
    st_method_flags flags;

    flags = st_method_get_flags (cpu->new_method);
    if (flags == ST_METHOD_PRIMITIVE) {
	primitive_index = st_method_get_primitive_index (cpu->new_method);
	cpu->success = true;
	st_primitives[primitive_index].func (cpu);
	if (ST_LIKELY (cpu->success))
	    return;
    }
    
    activate_method ();
}


void
st_cpu_set_active_context (st_oop context)
{
    register struct st_cpu *cpu = &__cpu;	 
    st_oop home;

    /* save executation state of active context */
    if (ST_UNLIKELY (cpu->context != st_nil)) {
	ST_CONTEXT_PART_IP (cpu->context) = st_smi_new (cpu->ip);
	ST_CONTEXT_PART_SP (cpu->context) = st_smi_new (cpu->sp);
    }
    
    if (ST_OBJECT_CLASS (context) == st_block_context_class) {
	home = ST_BLOCK_CONTEXT_HOME (context);
	cpu->method   = ST_METHOD_CONTEXT_METHOD (home);
	cpu->receiver = ST_METHOD_CONTEXT_RECEIVER (home);
	cpu->literals = st_array_elements (ST_METHOD_LITERALS (cpu->method));
	cpu->temps    = ST_METHOD_CONTEXT_STACK (home);
	cpu->stack    = ST_BLOCK_CONTEXT_STACK (context);
    } else {
	cpu->method   = ST_METHOD_CONTEXT_METHOD (context);
	cpu->receiver = ST_METHOD_CONTEXT_RECEIVER (context);
	cpu->literals = st_array_elements (ST_METHOD_LITERALS (cpu->method));
	cpu->temps    = ST_METHOD_CONTEXT_STACK (context);
	cpu->stack    = ST_METHOD_CONTEXT_STACK (context);
    }

    cpu->context  = context;
    cpu->sp       = st_smi_value (ST_CONTEXT_PART_SP (context));
    cpu->ip       = st_smi_value (ST_CONTEXT_PART_IP (context));
    cpu->bytecode = st_method_bytecode_bytes (cpu->method);
}

void
st_cpu_prologue (void)
{
    if (st_verbose_mode ()) {
	fprintf (stderr, "** gc: totalPauseTime: %.6fs\n", st_timespec_to_double_seconds (&memory->total_pause_time));
    }
}

#define SEND_SELECTOR(selector, argcount)			\
    cpu->message_argcount = argcount;				\
    cpu->message_receiver = sp[- argcount - 1];			\
    cpu->message_selector = selector;				\
    goto common;

#define ACTIVATE_CONTEXT(context)		\
    st_cpu_set_active_context (context);
              
#define SEND_TEMPLATE()							\
    cpu->lookup_class = st_object_class (cpu->message_receiver);	\
    ip += 1;								\
    goto common;

#ifdef __GNUC__
#define HAVE_COMPUTED_GOTO
#endif

#ifdef HAVE_COMPUTED_GOTO
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

#ifdef HAVE_COMPUTED_GOTO
#define CASE(OP) OP:              
#else
#define CASE(OP) case OP:
#endif

#ifdef HAVE_COMPUTED_GOTO
#define NEXT() goto *labels[*ip]
#else
#define NEXT() goto start
#endif

static inline void
install_method_in_cache (void)
{
    register struct st_cpu  *cpu = &__cpu;
    st_uint probe;

    probe = ST_METHOD_CACHE_HASH (cpu->lookup_class, cpu->message_selector);
    cpu->method_cache[probe].class    = cpu->lookup_class;
    cpu->method_cache[probe].selector = cpu->message_selector;
    cpu->method_cache[probe].method   = cpu->new_method;
}

static inline bool
lookup_method_in_cache (void)
{
    register struct st_cpu  *cpu = &__cpu;
    st_uint probe;

    probe = ST_METHOD_CACHE_HASH (cpu->lookup_class, cpu->message_selector);
    if (cpu->method_cache[probe].class    == cpu->lookup_class && 
	cpu->method_cache[probe].selector == cpu->message_selector) {
	cpu->new_method = cpu->method_cache[probe].method;
	return true;
    }
    return false;
}

#define STACK_POP(oop)     (*--sp)
#define STACK_PUSH(oop)    (*sp++ = (oop))
#define STACK_PEEK(oop)    (*(sp-1))
#define STACK_UNPOP(count) (sp += count)

void
st_cpu_main (void)
{
    register struct st_cpu  *cpu = &__cpu;
    register const st_uchar *ip;
    register st_oop *sp = cpu->stack;

    if (setjmp (cpu->main_loop))
	goto out;

    ip = cpu->bytecode + cpu->ip;

    SWITCH (ip) {
	
	CASE (PUSH_TEMP) {

	    STACK_PUSH (cpu->temps[ip[1]]);
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (PUSH_INSTVAR) {
    
	    STACK_PUSH (ST_OBJECT_FIELDS (cpu->receiver)[ip[1]]);

	    ip += 2;
	    NEXT ();
	}

	CASE (STORE_POP_INSTVAR) {
	    
	    ST_OBJECT_FIELDS (cpu->receiver)[ip[1]] = STACK_POP ();
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (STORE_INSTVAR) {
    
	    ST_OBJECT_FIELDS (cpu->receiver)[ip[1]] = STACK_PEEK ();
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (STORE_POP_TEMP) {
	    
	    cpu->temps[ip[1]] = STACK_POP ();
	    
	    ip += 2;
	    NEXT ();
	}    

	CASE (STORE_TEMP) {
	    
	    cpu->temps[ip[1]] = STACK_PEEK ();
	    
	    ip += 2;
	    NEXT ();
	}    
	
	CASE (STORE_LITERAL_VAR) {
	    
	    ST_ASSOCIATION_VALUE (cpu->literals[ip[1]]) = STACK_PEEK ();
	    
	    ip += 2;
	    NEXT ();
	}
	    
	CASE (STORE_POP_LITERAL_VAR) {
	    
	    ST_ASSOCIATION_VALUE (cpu->literals[ip[1]]) = STACK_POP ();
	    
	    ip += 2;
	    NEXT ();
	}
	
	CASE (PUSH_SELF) {
    
	    STACK_PUSH (cpu->receiver);

	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_TRUE) {
    
	    STACK_PUSH (st_true);
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_FALSE) {
	    
	    STACK_PUSH (st_false);
	
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_NIL) {
    
	    STACK_PUSH (st_nil);
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_INTEGER) {
    
	    STACK_PUSH (st_smi_new ((signed char) ip[1]));
	    
	    ip += 2;
	    NEXT ();
	}
	
	CASE (PUSH_ACTIVE_CONTEXT) {
    
	    STACK_PUSH (cpu->context);
	    
	    ip += 1;
	    NEXT ();
	}

	CASE (PUSH_LITERAL_CONST) {
    
	    STACK_PUSH (cpu->literals[ip[1]]);
	    
	    ip += 2;
	    NEXT ();
	}

	CASE (PUSH_LITERAL_VAR) {
	    
	    st_oop var;
	    
	    var = ST_ASSOCIATION_VALUE (cpu->literals[ip[1]]);
	    
	    STACK_PUSH (var);	    
	    
	    ip += 2;
	    NEXT ();
	}
	    
	CASE (JUMP_TRUE) {
	    
	    if (STACK_PEEK () == st_true) {
		(void) STACK_POP ();
		ip += 3 + *((short *) (ip + 1));
	    } else if (ST_LIKELY (STACK_PEEK () == st_false)) {
		(void) STACK_POP ();
		ip += 3;
	    } else {
		ip += 3;
		SEND_SELECTOR (st_selector_mustBeBoolean, 0);
	    }
	    
	    NEXT ();
	}

	CASE (JUMP_FALSE) {
	    
	    if (STACK_PEEK () == st_false) {
		(void) STACK_POP ();
		ip += 3 + *((short *) (ip + 1));
	    } else if (ST_LIKELY (STACK_PEEK () == st_true)) {
		(void) STACK_POP ();
		ip += 3;
	    } else {
		ip += 3;
		SEND_SELECTOR (st_selector_mustBeBoolean, 0);
	    }
	    
	    NEXT ();
	}
    
	CASE (JUMP) {
	    
	    short offset = *((short *) (ip + 1));
	    
	    ip += ((offset >= 0) ? 3 : 0) + offset;
	    
	    NEXT ();    
	}
	
	CASE (SEND_PLUS) {
	    
	    st_oop a, b;
	    
	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_new (st_smi_value (a) + st_smi_value (b)));
		ip++;
		NEXT ();
	    }

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_PLUS;	    
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];

	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
	
	CASE (SEND_MINUS) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_new (st_smi_value (a) - st_smi_value (b)));
		ip++;
		NEXT ();
	    }

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_MINUS;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];

	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
    
	CASE (SEND_MUL) {

	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_new (st_smi_value (a) * st_smi_value (b)));
		ip++;
		NEXT ();
	    }

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_MUL;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	
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

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_MOD;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	    
	}
    
	CASE (SEND_DIV) {

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_DIV;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
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
    
	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_BITSHIFT;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();	    
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

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_BITAND;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();	    
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

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_BITOR;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	
	    SEND_TEMPLATE ();
	    
	    NEXT ();	    
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

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_BITXOR;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
	
	CASE (SEND_LT) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_value (a) < st_smi_value (b) ? st_true : st_false);
		ip++;
		NEXT ();
	    }

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_LT;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
	
	CASE (SEND_GT) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_value (a) > st_smi_value (b) ? st_true : st_false);
		ip++;
		NEXT ();
	    }

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_GT;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	
	    NEXT ();
	}
	
	CASE (SEND_LE) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_value (a) <= st_smi_value (b) ? st_true : st_false);
		ip++;
		NEXT ();
	    }
	    
	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_LE;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();	    
	}
	
	CASE (SEND_GE) {
	    
	    st_oop a, b;

	    if (ST_LIKELY (st_object_is_smi (sp[-1]) &&
			   st_object_is_smi (sp[-2]))) {
		b = STACK_POP ();
		a = STACK_POP ();
		STACK_PUSH (st_smi_value (a) >= st_smi_value (b) ? st_true : st_false);
		ip++;
		NEXT ();
	    }

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_GE;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
	
	CASE (SEND_CLASS) {

	    cpu->message_argcount = 0;
	    cpu->message_selector = ST_SELECTOR_CLASS;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	
	    NEXT ();
	}
	
	CASE (SEND_SIZE) {
	    
	    cpu->message_argcount = 0;
	    cpu->message_selector = ST_SELECTOR_SIZE;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
    
	CASE (SEND_AT) {

	    st_oop  receiver;
	    st_oop  integer;
	    st_uint index;

	    integer = STACK_POP ();
	    receiver = STACK_POP ();
	    if (st_object_is_heap (receiver) &&
		st_object_format (receiver) == ST_FORMAT_ARRAY) {

		if (ST_LIKELY (st_object_is_smi (integer)))
		    index = st_smi_value (integer);
		else if (ST_UNLIKELY (st_object_is_character (integer))) {
		    STACK_UNPOP (2);
		    goto common_at;
		} else if (ST_LIKELY (ST_OBJECT_CLASS (integer) == st_large_integer_class))
		    index = mp_get_int (st_large_integer_value (integer));
		else {
		    STACK_UNPOP (2);
		    goto common_at;
		}

		if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
		    STACK_UNPOP (2);
		    goto common_at;
		}
		
		STACK_PUSH (st_array_at (receiver, index));

		ip += 1;
		NEXT ();
	    } else {
		STACK_UNPOP (2);
	    }


	common_at:

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_AT;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
    
	CASE (SEND_AT_PUT) {

	    st_oop  receiver;
	    st_oop  integer;
	    st_oop   value;
	    st_uint index;

	    value   = STACK_POP ();
	    integer = STACK_POP ();
	    receiver = STACK_POP ();
	    if (st_object_is_heap (receiver) &&
		st_object_format (receiver) == ST_FORMAT_ARRAY) {

		if (ST_LIKELY (st_object_is_smi (integer))) {
		    index = st_smi_value (integer);
		} else if (ST_UNLIKELY (st_object_is_character (integer))) {
		    STACK_UNPOP (3);
		    goto common_atput;
		} else if (ST_LIKELY (ST_OBJECT_CLASS (integer) == st_large_integer_class)) {
		    index = mp_get_int (st_large_integer_value (integer));
		} else {
		    STACK_UNPOP (3);
		    goto common_atput;
		}

		if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
		    STACK_UNPOP (3);
		    goto common_atput;
		}

		st_array_at_put (receiver, index, value);
		STACK_PUSH (value);

		ip += 1;
		NEXT ();

	    } else {
		STACK_UNPOP (3);
	    }

	common_atput:
	    
	    cpu->message_argcount = 2;
	    cpu->message_selector = ST_SELECTOR_ATPUT;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
	
	CASE (SEND_EQ) {

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_EQ;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
	
	CASE (SEND_NE) {

	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_NE;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	
	    NEXT ();
	}
	
	CASE (SEND_IDENTITY_EQ) {

	    st_oop a, b;
	    a = STACK_POP ();
	    b = STACK_POP ();
	    
	    STACK_PUSH ((a == b) ? st_true : st_false);

	    ip += 1;
	    NEXT ();
	}
	
	CASE (SEND_VALUE) {

	    cpu->message_argcount = 0;
	    cpu->message_selector = ST_SELECTOR_VALUE;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];

	    SEND_TEMPLATE ();

	    NEXT ();
	}
    
	CASE (SEND_VALUE_ARG) {
	    
	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_VALUE_ARG;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];

	    SEND_TEMPLATE ();

	    NEXT ();
	}
	
	CASE (SEND_NEW) {
    
	    cpu->message_argcount = 0;
	    cpu->message_selector = ST_SELECTOR_NEW;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
	
	CASE (SEND_NEW_ARG) {
	    
	    cpu->message_argcount = 1;
	    cpu->message_selector = ST_SELECTOR_NEW_ARG;
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    
	    SEND_TEMPLATE ();
	    
	    NEXT ();
	}
	
	CASE (SEND) {
	    
	    st_uint primitive_index;
	    st_method_flags flags;
	    st_oop  context;
	    st_oop *arguments;

	    cpu->message_argcount = ip[1];
	    cpu->message_selector = cpu->literals[ip[2]];
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];
	    cpu->lookup_class = st_object_class (cpu->message_receiver);

	    ip += 3;
	    
	common:

	    if (!lookup_method_in_cache ()) {
		cpu->ip = ip - cpu->bytecode;
		cpu->sp = sp - cpu->stack;
		cpu->new_method = lookup_method (cpu->lookup_class);
		ip = cpu->bytecode + cpu->ip;
		sp = cpu->stack + cpu->sp;
		install_method_in_cache ();
	    }
	    
	    flags = st_method_get_flags (cpu->new_method);
	    if (flags == ST_METHOD_PRIMITIVE) {
		primitive_index = st_method_get_primitive_index (cpu->new_method);

		cpu->success = true;
		cpu->ip = ip - cpu->bytecode;
		cpu->sp = sp - cpu->stack;
		st_primitives[primitive_index].func (cpu);
		ip = cpu->bytecode + cpu->ip;
		sp = cpu->stack + cpu->sp;

		if (ST_LIKELY (cpu->success))
		    NEXT ();
	    }
	    
	    context = method_context_new ();
	    arguments = ST_METHOD_CONTEXT_STACK (context);
	    for (int i = 0; i < cpu->message_argcount; i++)
		arguments[i] =  sp[- cpu->message_argcount + i];
	    sp -= cpu->message_argcount + 1;

	    ST_CONTEXT_PART_IP (cpu->context) = st_smi_new (ip - cpu->bytecode);
	    ST_CONTEXT_PART_SP (cpu->context) = st_smi_new (sp - cpu->stack);
	    cpu->context  = context;
	    cpu->method   = ST_METHOD_CONTEXT_METHOD (context);
	    cpu->receiver = ST_METHOD_CONTEXT_RECEIVER (context);
	    cpu->literals = st_array_elements (ST_METHOD_LITERALS (cpu->method));
	    cpu->temps    = ST_METHOD_CONTEXT_STACK (context);
	    cpu->stack    = ST_METHOD_CONTEXT_STACK (context);
	    cpu->sp       = st_smi_value (ST_CONTEXT_PART_SP (context));
	    cpu->ip       = st_smi_value (0);
	    cpu->bytecode = st_method_bytecode_bytes (cpu->method);
	    ip = cpu->bytecode;
	    sp = cpu->stack + cpu->sp;

	    NEXT ();
	}

	CASE (SEND_SUPER) {
	    
	    st_oop index;

	    cpu->message_argcount = ip[1];
	    cpu->message_selector = cpu->literals[ip[2]];
	    cpu->message_receiver = sp[- cpu->message_argcount - 1];

	    index = st_smi_value (st_arrayed_object_size (ST_METHOD_LITERALS (cpu->method))) - 1;
	    cpu->lookup_class = ST_BEHAVIOR_SUPERCLASS (cpu->literals[index]);

	    ip += 3;

	    goto common;
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
	    
	    initial_ip = ip - cpu->bytecode + 3;
	    
	    cpu->ip = ip - cpu->bytecode;
	    cpu->sp = sp - cpu->stack;

	    block = block_context_new (initial_ip, argcount);

	    /* update ip in case bytecodes moved during gc */ 
	    ip = cpu->bytecode + cpu->ip;
	    sp = cpu->stack + cpu->sp;

	    STACK_PUSH (block);
	    
	    NEXT ();
	}
	
	CASE (RETURN_STACK_TOP) {
	    
	    st_oop sender;
	    st_oop value;
	    st_oop home;
	    
	    value = STACK_PEEK ();
	    
	    if (ST_OBJECT_CLASS (cpu->context) == st_block_context_class)
		sender = ST_CONTEXT_PART_SENDER (ST_BLOCK_CONTEXT_HOME (cpu->context));
	    else {
		sender = ST_CONTEXT_PART_SENDER (cpu->context);
		st_memory_recycle_context (cpu->context);
	    }

	    if (ST_UNLIKELY (sender == st_nil)) {
		STACK_PUSH (cpu->context);
		STACK_PUSH (value);
		SEND_SELECTOR (st_selector_cannotReturn, 1);
		NEXT ();
	    }		

	    if (ST_OBJECT_CLASS (sender) == st_block_context_class) {
		home = ST_BLOCK_CONTEXT_HOME (sender);
		cpu->method   = ST_METHOD_CONTEXT_METHOD (home);
		cpu->receiver = ST_METHOD_CONTEXT_RECEIVER (home);
		cpu->literals = st_array_elements (ST_METHOD_LITERALS (cpu->method));
		cpu->temps    = ST_METHOD_CONTEXT_STACK (home);
		cpu->stack    = ST_BLOCK_CONTEXT_STACK (sender);
	    } else {
		cpu->method   = ST_METHOD_CONTEXT_METHOD (sender);
		cpu->receiver = ST_METHOD_CONTEXT_RECEIVER (sender);
		cpu->literals = st_array_elements (ST_METHOD_LITERALS (cpu->method));
		cpu->temps    = ST_METHOD_CONTEXT_STACK (sender);
		cpu->stack    = ST_METHOD_CONTEXT_STACK (sender);
	    }
	    
	    cpu->context  = sender;
	    cpu->sp       = st_smi_value (ST_CONTEXT_PART_SP (sender));
	    cpu->ip       = st_smi_value (ST_CONTEXT_PART_IP (sender));
	    cpu->bytecode = st_method_bytecode_bytes (cpu->method);
	    ip = cpu->bytecode + cpu->ip;
	    sp = cpu->stack + cpu->sp;

	    STACK_PUSH (value);

	    NEXT ();
	}

	CASE (BLOCK_RETURN) {
	    
	    st_oop caller;
	    st_oop value;
	    st_oop home;
	    
	    caller = ST_BLOCK_CONTEXT_CALLER (cpu->context);
	    value = STACK_PEEK ();

	    if (ST_OBJECT_CLASS (caller) == st_block_context_class) {
		home = ST_BLOCK_CONTEXT_HOME (caller);
		cpu->method   = ST_METHOD_CONTEXT_METHOD (home);
		cpu->receiver = ST_METHOD_CONTEXT_RECEIVER (home);
		cpu->literals = st_array_elements (ST_METHOD_LITERALS (cpu->method));
		cpu->temps    = ST_METHOD_CONTEXT_STACK (home);
		cpu->stack    = ST_BLOCK_CONTEXT_STACK (caller);
	    } else {
		cpu->method   = ST_METHOD_CONTEXT_METHOD (caller);
		cpu->receiver = ST_METHOD_CONTEXT_RECEIVER (caller);
		cpu->literals = st_array_elements (ST_METHOD_LITERALS (cpu->method));
		cpu->temps    = ST_METHOD_CONTEXT_STACK (caller);
		cpu->stack    = ST_METHOD_CONTEXT_STACK (caller);
	    }
	    
	    cpu->context  = caller;
	    cpu->sp       = st_smi_value (ST_CONTEXT_PART_SP (caller));
	    cpu->ip       = st_smi_value (ST_CONTEXT_PART_IP (caller));
	    cpu->bytecode = st_method_bytecode_bytes (cpu->method);
	    ip = cpu->bytecode + cpu->ip;
	    sp = cpu->stack + cpu->sp;
	    
	    /* push returned value onto caller's stack */
	    STACK_PUSH (value);
	    
	    NEXT ();
	}
    }

out:
    st_cpu_prologue ();
}

void
st_cpu_clear_caches (void)
{
    for (st_uint i = 0; i < ST_METHOD_CACHE_SIZE; i++) {
	__cpu.method_cache[i].class    = st_nil;
	__cpu.method_cache[i].selector = st_nil;
	__cpu.method_cache[i].method   = st_nil;
    }
}

void
st_cpu_initialize (void)
{
    st_oop context;
    st_oop method;

    /* clear contents */
    __cpu.context  = st_nil;
    __cpu.receiver = st_nil;
    __cpu.method   = st_nil;

    __cpu.sp = 0;
    __cpu.ip = 0;
    __cpu.stack = NULL;

    st_cpu_clear_caches ();

    __cpu.message_argcount = 0;
    __cpu.message_receiver = st_smalltalk;
    __cpu.message_selector = st_selector_startupSystem;

    __cpu.new_method = lookup_method (st_object_class (__cpu.message_receiver));
    st_assert (st_method_get_flags (__cpu.new_method) == ST_METHOD_NORMAL);

    context = method_context_new ();
    st_cpu_set_active_context (context);
}

