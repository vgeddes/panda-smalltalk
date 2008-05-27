
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
create_actual_message (STExecutionState *es)
{
    st_oop array;

    array = st_object_new_arrayed (st_array_class, es->message_argcount);
    for (guint i = 0; i < es->message_argcount; i++)
	(* st_array_element (array, i + 1)) =  es->stack[es->sp - es->message_argcount + i];

    es->sp -= es->message_argcount;

    ST_STACK_PUSH (es, st_message_new (es->message_selector, array));

    es->message_selector = st_selector_doesNotUnderstand;
    es->message_argcount = 1;
}

st_oop
st_interpreter_lookup_method (STExecutionState *es, st_oop klass)
{
    st_oop method;
    st_oop parent = klass;

    while (parent != st_nil) {
	method = st_dictionary_at (st_behavior_method_dictionary (parent), es->message_selector);
	if (method != st_nil)
	    return method;
	parent = st_behavior_superclass (parent);
    }

    if (es->message_selector == st_selector_doesNotUnderstand) {
	g_critical ("no method found for #doesNotUnderstand:");
	exit(1);
    }

    create_actual_message (es);
    
    return st_interpreter_lookup_method (es, klass);
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
activate_method (STExecutionState *es, st_oop method)
{
    st_oop  context;
    st_oop *arguments;
    
    context = st_method_context_new (es->context, es->message_receiver,  method);

    arguments = st_method_context_temporary_frame (context);
    for (guint i = 0; i < es->message_argcount; i++)
	arguments[i] =  es->stack[es->sp - es->message_argcount + i];


    es->sp -= es->message_argcount + 1;

    st_interpreter_set_active_context (es, context);
}

void
st_interpreter_execute_method (STExecutionState *es, st_oop method)
{
    guint primitive_index;
    STMethodFlags flags;

    flags = st_method_flags (method);
    if (flags == ST_METHOD_PRIMITIVE) {
	primitive_index = st_method_primitive_index (method);
	es->success = true;
	st_primitives[primitive_index].func (es);
	if (G_LIKELY (es->success))
	    return;
    }

    activate_method (es, method);
}

void
st_interpreter_set_active_context (STExecutionState *es,
				   st_oop  context)
{
    st_oop home;

    /* save executation state of active context */
    if (es->context != st_nil) {
	st_context_part_ip (es->context) = st_smi_new (es->ip);
	st_context_part_sp (es->context) = st_smi_new (es->sp);
    }
    
    if (st_object_class (context) == st_block_context_class) {

	home = st_block_context_home (context);

	es->method   = st_method_context_method (home);
	es->receiver = st_method_context_receiver (home);
	es->literals = st_array_element (st_method_literals (es->method), 1);
	es->temps    = st_method_context_temporary_frame (home);
	es->stack    = st_block_context_stack (context);
    } else {
	es->method   = st_method_context_method (context);
	es->receiver = st_method_context_receiver (context);
	es->literals = st_array_element (st_method_literals (es->method), 1);
	es->temps    = st_method_context_temporary_frame (context);
	es->stack    = st_method_context_stack_frame (context);
    }

    es->context  = context;
    es->sp       = st_smi_value (st_context_part_sp (context));
    es->ip       = st_smi_value (st_context_part_ip (context));
    es->bytecode = st_method_bytecode_bytes (es->method);
}

void
st_interpreter_send_selector (STExecutionState *es,
			      st_oop            selector,
			      guint             argcount)
{
    st_oop method;
    guint primitive_index;
    STMethodFlags flags;

    es->message_argcount = argcount;
    es->message_receiver = es->stack[es->sp - argcount - 1];
    es->message_selector = selector;
    
    method = st_interpreter_lookup_method (es, st_object_class (es->message_receiver));
    
    flags = st_method_flags (method);
    if (flags == ST_METHOD_PRIMITIVE) {
	primitive_index = st_method_primitive_index (method);
	es->success = true;
	st_primitives[primitive_index].func (es);
	if (G_LIKELY (es->success))
	    return;
    }

    activate_method (es, method);
}

#define SEND_SELECTOR(es, selector, argcount)				\
    es->ip = ip - es->bytecode;						\
    st_interpreter_send_selector (es, selector, argcount);		\
    ip = es->bytecode + es->ip;

#define ACTIVATE_CONTEXT(es, context_oop)				\
    es->ip = ip - es->bytecode;						\
    st_interpreter_set_active_context (es, context_oop);		\
    ip = es->bytecode + es->ip;

#define ACTIVATE_METHOD(es, method_oop)			\
    es->ip = ip - es->bytecode;				\
    activate_method (es, method_oop);			\
    ip = es->bytecode + es->ip;

#define EXECUTE_PRIMITIVE(es, index)		\
    es->success = true;				\
    es->ip = ip - es->bytecode;			\
    st_primitives[index].func (es);		\
    ip = es->bytecode + es->ip;

#define SEND_TEMPLATE(es)						\
    st_oop method;							\
    guint  prim;							\
    STMethodFlags flags;						\
    									\
    ip += 1;								\
    									\
    method = st_interpreter_lookup_method (es, st_object_class (es->message_receiver));\
    									\
    flags = st_method_flags (method);					\
    if (flags == ST_METHOD_PRIMITIVE) {					\
	prim = st_method_primitive_index (method);			\
									\
	EXECUTE_PRIMITIVE (es, prim);					\
	if (G_LIKELY (es->success))					\
            break;							\
    }									\
    									\
    ACTIVATE_METHOD (es, method);

void
st_interpreter_main (STExecutionState *es)
{
    register guchar *ip;

    if (setjmp (es->main_loop))
	return;

    ip = es->bytecode + es->ip;

    for (;;) {
	
	switch (*ip) {
	    
	case STORE_POP_TEMP:

	    es->temps[ip[1]] = ST_STACK_POP (es);
	    
	    ip += 2;
	    break;
	    
	case STORE_TEMP:

	    es->temps[ip[1]] = ST_STACK_PEEK (es);
	    
	    ip += 2;
	    break;
	    
	case PUSH_TEMP:

	    ST_STACK_PUSH (es, es->temps[ip[1]]);
	    
	    ip += 2;
	    break;

	case PUSH_INSTVAR:
	    
	    ST_STACK_PUSH (es, st_heap_object_body (es->receiver)[ip[1]]);

	    ip += 2;
	    break;

	case STORE_POP_INSTVAR:
	    
	    st_heap_object_body (es->receiver)[ip[1]] = ST_STACK_POP (es);

	    ip += 2;
	    break;
	    
	case STORE_INSTVAR:

	    st_heap_object_body (es->receiver)[ip[1]] = ST_STACK_PEEK (es);
	    
	    ip += 2;
	    break;
	    
	case PUSH_SELF:
	    
	    ST_STACK_PUSH (es, es->receiver);
	    
	    ip += 1;
	    break;
	    
	case PUSH_TRUE:
	    
	    ST_STACK_PUSH (es, st_true);

	    ip += 1;
	    break;
	    
	case PUSH_FALSE:
	    
	    ST_STACK_PUSH (es, st_false);

	    ip += 1;
	    break;
	    
	case PUSH_NIL:
	    
	    ST_STACK_PUSH (es, st_nil);
	    
	    ip += 1;
	    break;

	case PUSH_ACTIVE_CONTEXT:
	    
	    ST_STACK_PUSH (es, es->context);
	    
	    ip += 1;
	    break;
	     
	case PUSH_LITERAL_CONST:

	    ST_STACK_PUSH (es, es->literals[ip[1]]);
	    
	    ip += 2;
	    break;

	case PUSH_LITERAL_VAR:
	{
	    st_oop var;

	    var = st_association_value (es->literals[ip[1]]);
	    
	    ST_STACK_PUSH (es, var);
	    
	    ip += 2;
	    break;
	}
	    
	case JUMP_TRUE:
	{
	    if (ST_STACK_PEEK (es) == st_true) {
		(void) ST_STACK_POP (es);
		ip += 3 + ((ip[1] << 8) | ip[2]);
	    } else if (ST_STACK_PEEK (es) == st_false) {
		(void) ST_STACK_POP (es);
		ip += 3;
	    } else {
		ip += 3;
		SEND_SELECTOR (es, st_selector_mustBeBoolean, 0);
	    }
	    
	    break;
	}

	case JUMP_FALSE:
	{
	    if (ST_STACK_PEEK (es) == st_false) {
		(void) ST_STACK_POP (es);
		ip += 3 + ((ip[1] << 8) | ip[2]);
	    } else if (ST_STACK_PEEK (es) == st_true) {
		(void) ST_STACK_POP (es);
		ip += 3;
	    } else {
		ip += 3;
		SEND_SELECTOR (es, st_selector_mustBeBoolean, 0);
	    }
	    
	    break;
	}

	case JUMP:
	{
	    short offset =  ((ip[1] << 8) | ip[2]);    

	    ip += ((offset >= 0) ? 3 : 0) + offset;

	    break;
	}
	
	case SEND_PLUS:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_PLUS];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	    
	    break;
	}

	case SEND_MINUS:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_MINUS];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_MUL:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_MUL];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_MOD:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_MOD];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_DIV:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_DIV];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_BITSHIFT:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_BITSHIFT];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_BITAND:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_BITAND];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_BITOR:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_BITOR];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_BITXOR:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_BITOR];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_LT:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_LT];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_GT:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_GT];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);

	    break;
	}
	
	case SEND_LE:
	{   
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_LE];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);

	    break;
	}
	
	case SEND_GE:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_GE];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];

	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_CLASS:
	{
	    es->message_argcount = 0;
	    es->message_selector = st_specials[ST_SPECIAL_CLASS];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	 }

	case SEND_SIZE:
	{
	    es->message_argcount = 0;
	    es->message_selector = st_specials[ST_SPECIAL_SIZE];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	 }
	
	case SEND_AT:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_AT];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	 }
	
	case SEND_AT_PUT:
	{
	    es->message_argcount = 2;
	    es->message_selector = st_specials[ST_SPECIAL_ATPUT];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_EQ:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_EQ];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_NE:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_NE];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_IDENTITY_EQ:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_IDEQ];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_VALUE:
	{
	    es->message_argcount = 0;
	    es->message_selector = st_specials[ST_SPECIAL_VALUE];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_VALUE_ARG:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_VALUE_ARG];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_NEW:
	{
	    es->message_argcount = 0;
	    es->message_selector = st_specials[ST_SPECIAL_NEW];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_NEW_ARG:
	{
	    es->message_argcount = 1;
	    es->message_selector = st_specials[ST_SPECIAL_NEW_ARG];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND:
	{
	    st_oop method;
	    guint  primitive_index;
	    STMethodFlags flags;
	    
	    es->message_argcount = ip[1];
	    es->message_selector = es->literals[ip[2]];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    ip += 3;
	    
	    method = st_interpreter_lookup_method (es, st_object_class (es->message_receiver));
	    
	    flags = st_method_flags (method);
	    if (flags == ST_METHOD_PRIMITIVE) {
		primitive_index = st_method_primitive_index (method);

		EXECUTE_PRIMITIVE (es, primitive_index);
		if (G_LIKELY (es->success))
		    break;
	    }
	    
	    ACTIVATE_METHOD (es, method);
	    break;
	}

	case SEND_SUPER:
	{
	    st_oop method;
	    st_oop literal_index;
	    guint  primitive_index;
	    STMethodFlags flags;
	    
	    es->message_argcount = ip[1];
	    es->message_selector = es->literals[ip[2]];
	    es->message_receiver = es->stack[es->sp - es->message_argcount - 1];
	    
	    ip += 3;

	    literal_index = st_smi_value (st_array_size (st_method_literals (es->method))) - 1;

	    method = st_interpreter_lookup_method (es, st_behavior_superclass (es->literals[literal_index]));
	    
	    flags = st_method_flags (method);
	    if (flags == ST_METHOD_PRIMITIVE) {
		primitive_index = st_method_primitive_index (method);

		EXECUTE_PRIMITIVE (es, primitive_index);
		if (G_LIKELY (es->success))
		    break;
	    }
	    	    
	    ACTIVATE_METHOD (es, method);
	    break;
	}
	
	case POP_STACK_TOP:

	    (void) ST_STACK_POP (es);
	    
	    ip += 1;
	    break;

	case DUPLICATE_STACK_TOP:
 
	    ST_STACK_PUSH (es, ST_STACK_PEEK (es));
	    
	    ip += 1;
	    break;
	    
	case BLOCK_COPY:
	{

	    st_oop block;
	    st_oop home;
	    guint argcount = ip[1];
	    guint initial_ip;
	    
	    ip += 2;
	    
	    initial_ip = ip - es->bytecode + 3;

	    if (st_object_class (es->context) == st_block_context_class)
		home = st_block_context_home (es->context);
	    else
		home = es->context;

	    block = st_block_context_new (home, initial_ip, argcount);

	    ST_STACK_PUSH (es, block);
	    
	    break;
	}
	
	case RETURN_STACK_TOP:
	{
	    st_oop sender;
	    st_oop value;
	    
	    value = ST_STACK_PEEK (es);

	    if (st_object_class (es->context) == st_block_context_class)
		sender = st_context_part_sender (st_block_context_home (es->context));
	    else
		sender = st_context_part_sender (es->context);

	    if (sender == st_nil) {
		ST_STACK_PUSH (es, es->context);
		ST_STACK_PUSH (es, value);
		SEND_SELECTOR (es, st_selector_cannotReturn, 1);
		break;
	    }
	    
	    ACTIVATE_CONTEXT (es, sender);
	    ST_STACK_PUSH (es, value);

	    break;
	}
	
	case BLOCK_RETURN:
	{
	    st_oop caller;
	    st_oop value;
	    
	    caller = st_block_context_caller (es->context);
	    value = ST_STACK_PEEK (es);
	    ACTIVATE_CONTEXT (es, caller);

	    /* push returned value onto caller's stack */
	    ST_STACK_PUSH (es, value);
	    g_assert (es->context == caller);
	    
	    break;
	}
	
	default:
	    g_assert_not_reached ();
	}
	
    }
    
    return;
}

void
st_interpreter_initialize (STExecutionState *es)
{
    st_oop context;
    st_oop method;

    /* clear contents */
    memset (es, 0, sizeof (STExecutionState));
    es->context = st_nil;
    es->receiver = st_nil;
    es->method = st_nil;

    es->message_argcount = 0;
    es->message_receiver = st_nil;
    es->message_selector = st_selector_startupSystem;
    
    method = st_interpreter_lookup_method (es, st_object_class (es->message_receiver));
    g_assert (st_method_flags (method) == ST_METHOD_NORMAL);

    context = st_method_context_new (es->context, es->message_receiver,  method);
    st_interpreter_set_active_context (es, context);
}

