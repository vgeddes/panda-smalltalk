
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

/* 3 + 4 ! */
static st_oop
create_doIt_method (void)
{
    STError *error = NULL;

    /* static const char string1[] = */
    /* 	"doIt" */
    /* 	"   | object selected |" */
    /* 	"   object := #(1 1 2 3 5)." */
    /* 	"   selected := object select: [ :element | element = 1 or: [element = 2] ]." */
    /* 	" ^ selected size"; */

    static const char string1[] =
    	"doIt"
    	"   | object selected |"
	" ^ 56 increment";

    static const char string2[] = 
	"increment"
	"    ^ self + 1";

    st_compile_string (st_undefined_object_class, string1, &error);
    g_assert (error == NULL);

    st_compile_string (st_smi_class, string2, &error);
    g_assert (error == NULL);

    return st_dictionary_at (st_behavior_method_dictionary (st_undefined_object_class), st_symbol_new ("doIt"));
}

static st_oop
lookup_method (st_oop class, st_oop selector)
{
    st_oop dict;
    st_oop method = st_nil;

    while (method == st_nil) {

	dict = st_behavior_method_dictionary (class);
	
	method = st_dictionary_at (dict, selector);
	
	class = st_behavior_superclass (class);
	if (class == st_nil)
	    break;
    }

    return method;
}


st_oop
st_send_unary_message (st_oop sender,
	  	       st_oop receiver,
		       st_oop selector)
{
    st_oop context;
    st_oop method;

    method = lookup_method (st_object_class (receiver), selector);
    g_assert (method != st_nil); 

    context = st_method_context_new (method);

    st_context_part_sender (context) = sender;
    st_context_part_ip (context) = st_smi_new (0);
    st_context_part_sp (context) = st_smi_new (0);

    st_method_context_receiver (context) = receiver;
    st_method_context_method (context) = method;

    return context;
}

INLINE void
message_not_understand (STExecutionState *es, st_oop selector, guint argcount)
{
    st_oop message;

    message = st_message_new (selector, es->stack + es->sp + 1, argcount);
   
}

INLINE st_oop
new_context (STExecutionState *es,
	     st_oop  sender,
	     st_oop  receiver,
	     st_oop  method,
	     guint   argcount)
{
    st_oop context;

    context = st_method_context_new (method);

    /* transfer arguments to context */
    st_oop *arguments = st_method_context_temporary_frame (context);
    for (guint i = 0; i < argcount; i++)
	arguments[i] =  es->stack[es->sp - argcount + i];

    st_context_part_sender (context) = sender;
    st_context_part_ip (context) = st_smi_new (0);
    st_context_part_sp (context) = st_smi_new (0);
    st_method_context_receiver (context) = receiver;
    st_method_context_method (context) = method;

    es->sp -= argcount + 1;

    return context;
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

INLINE guchar *
set_active_context (STExecutionState *es, guchar *ip, st_oop context)
{
    es->ip = ip - es->bytecode;

    st_interpreter_set_active_context (es, context);
    
    return es->bytecode + es->ip;
} 

INLINE st_oop
send_does_not_understand (STExecutionState *es,
			  st_oop            receiver,
			  st_oop            selector,
			  guint             argcount)
{
    st_oop message;
    st_oop method;
    st_oop method_selector;
    
    method_selector = st_symbol_new ("doesNotUnderstand:");
    
    method = lookup_method (st_object_class (receiver), method_selector);
    g_assert (method != st_nil);
    
    message = st_message_new (method_selector,
			      es->stack + es->sp - argcount,
			      argcount);    

    /* pop arguments off stack and replace them with a Message object */
    es->sp -= argcount;
    ST_STACK_PUSH (es, message);
    
    return new_context (es, es->context,
			receiver, method, 1);
}

INLINE void
execute_primitive (STExecutionState *es,
		   guint    pindex,
		   guchar  *ip,
                   guchar **ip_ret)
{
    es->success = true;

    es->ip = ip - es->bytecode;

    st_primitives[pindex].func (es);

    *ip_ret = es->bytecode + es->ip;
}


#define SEND_TEMPLATE(es)						\
    st_oop method;							\
    st_oop context;							\
    guint  prim;							\
    guchar *ip_ret = NULL;						\
    STMethodFlags flags;						\
    									\
    ip += 1;								\
    									\
    method = lookup_method (st_object_class (es->msg_receiver), es->msg_selector);	\
									\
    if (method == st_nil) {						\
	context = send_does_not_understand (es, es->msg_receiver, es->msg_selector, es->msg_argcount); \
	g_debug ("selector: %s", st_byte_array_bytes (es->msg_selector)); \
	ip = set_active_context (es, ip, context);			\
	break;								\
    }									\
    									\
    flags = st_method_flags (method);					\
    if (flags == ST_METHOD_PRIMITIVE) {					\
    									\
	prim = st_method_primitive_index (method);			\
									\
	execute_primitive (es, prim, ip, &ip_ret);			\
									\
        if (es->success) {						\
	    ip = ip_ret;						\
            break;							\
        }								\
    }									\
    									\
    context = new_context (es, es->context,				\
			   es->msg_receiver, method,		       	\
			   es->msg_argcount);			       	\
    									\
    ip = set_active_context (es, ip, context)


static st_oop
interpreter_loop (STExecutionState *es)
{
    register guchar *ip = st_method_bytecode_bytes (es->method);
    
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
	    
	    if (ST_STACK_POP (es) == st_true)
		ip += 3 + ((ip[1] << 8) | ip[2]);
	    else
		ip += 3;
	    
	    break;
	     
	case JUMP_FALSE:
	    
	    if (ST_STACK_POP (es) == st_false)
		ip += 3 + ((ip[1] << 8) | ip[2]);
	    else
		ip += 3;
	    
	    break;
	    
	case JUMP:
	{
	    short offset = (ip[1] << 8) | ip[2];
	    
	    ip += ((offset >= 0) ? 3 : 0) + offset;
	    break;
	}
	
	case SEND_PLUS:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_PLUS];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	    
	    break;
	}

	case SEND_MINUS:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_MINUS];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_MUL:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_MUL];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_MOD:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_MOD];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_DIV:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_DIV];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_BITSHIFT:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_BITSHIFT];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_BITAND:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_BITAND];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_BITOR:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_BITOR];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_BITXOR:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_BITOR];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_LT:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_LT];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_GT:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_GT];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);

	    break;
	}
	
	case SEND_LE:
	{   
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_LE];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);

	    break;
	}
	
	case SEND_GE:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_GE];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];

	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_CLASS:
	{
	    es->msg_argcount = 0;
	    es->msg_selector = st_specials[ST_SPECIAL_CLASS];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	 }

	case SEND_SIZE:
	{
	    es->msg_argcount = 0;
	    es->msg_selector = st_specials[ST_SPECIAL_SIZE];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	 }
	
	case SEND_AT:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_AT];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	 }
	
	case SEND_AT_PUT:
	{
	    es->msg_argcount = 2;
	    es->msg_selector = st_specials[ST_SPECIAL_ATPUT];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_EQ:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_EQ];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_NE:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_NE];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_IDENTITY_EQ:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_IDEQ];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_VALUE:
	{
	    es->msg_argcount = 0;
	    es->msg_selector = st_specials[ST_SPECIAL_VALUE];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}
	
	case SEND_VALUE_ARG:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_VALUE_ARG];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_NEW:
	{
	    es->msg_argcount = 0;
	    es->msg_selector = st_specials[ST_SPECIAL_NEW];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND_NEW_ARG:
	{
	    es->msg_argcount = 1;
	    es->msg_selector = st_specials[ST_SPECIAL_NEW_ARG];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    SEND_TEMPLATE (es);
	    
	    break;
	}

	case SEND:
	{
	    st_oop method;
	    st_oop context;
	    guint  pindex;
	    guchar *ip_ret = NULL;
	    STMethodFlags flags;
	    
	    es->msg_argcount = ip[1];
	    es->msg_selector = es->literals[ip[2]];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    ip += 3;
	    
	    method = lookup_method (st_object_class (es->msg_receiver), es->msg_selector);
   
	    if (G_UNLIKELY (method == st_nil)) {
		context = send_does_not_understand (es,
						    es->msg_receiver,
						    es->msg_selector,
						    es->msg_argcount);
		ip = set_active_context (es, ip, context);
		break;
	    }
	    
	    /* call primitive function if there is one */
	    flags = st_method_flags (method);

	    if (flags == ST_METHOD_PRIMITIVE) {
		pindex = st_method_primitive_index (method);
		
		execute_primitive (es, pindex, ip, &ip_ret);
		if (G_LIKELY (es->success)) {
		    ip = ip_ret;
		    break;
		}
	    }
	    
	    context = new_context (es, es->context,
				   es->msg_receiver, method,
				   es->msg_argcount);
	    
	    ip = set_active_context (es, ip, context);
	    
	    break;
	}

	case SEND_SUPER:
	{
	    st_oop method;
	    st_oop context;
	    guint  pindex;
	    guchar *ip_ret = NULL;
	    STMethodFlags flags;
	    
	    es->msg_argcount = ip[1];
	    es->msg_selector = es->literals[ip[2]];
	    es->msg_receiver = es->stack[es->sp - es->msg_argcount - 1];
	    
	    ip += 3;
	    
	    method = lookup_method (st_behavior_superclass (st_object_class (es->msg_receiver)), es->msg_selector);
   
	    if (G_UNLIKELY (method == st_nil)) {
		context = send_does_not_understand (es,
						    es->msg_receiver,
						    es->msg_selector,
						    es->msg_argcount);
		ip = set_active_context (es, ip, context);
		break;
	    }
	    
	    /* call primitive function if there is one */
	    flags = st_method_flags (method);

	    if (flags == ST_METHOD_PRIMITIVE) {
		pindex = st_method_primitive_index (method);
		
		execute_primitive (es, pindex, ip, &ip_ret);
		if (G_LIKELY (es->success)) {
		    ip = ip_ret;
		    break;
		}
	    }
	    
	    context = new_context (es, es->context,
				   es->msg_receiver, method,
				   es->msg_argcount);
	    
	    ip = set_active_context (es, ip, context);
	    
	    break;
	}
	 
	case POP_STACK_TOP:
	    
	    (void) ST_STACK_POP (es);
	    
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
	    
	    sender = st_context_part_sender (es->context);
	    value = ST_STACK_PEEK (es);
	    
	    /* exit loop if sender is nil */
	    if (sender == st_nil)
		return value;
	    
	    ip = set_active_context (es, ip, sender);
	    
	    /* push returned value onto stack */
	    ST_STACK_PUSH (es, value);
	    
	    break;
	}
	
	case BLOCK_RETURN:
	{
	    st_oop caller;
	    st_oop value;
	    
	    caller = st_block_context_caller (es->context);
	    value = ST_STACK_PEEK (es);

	    ip = set_active_context (es, ip, caller);

	    /* push returned value onto caller's stack */
	    ST_STACK_PUSH (es, value);

	    g_assert (es->context == caller);
	    
	    break;
	}
	
	default:
	    g_debug ("%i\n", *ip);
	    g_assert_not_reached ();
	}
	
    }
    
    return st_nil;
}


void
st_interpreter_initialize_state (STExecutionState *es)
{
    memset (es, 0, sizeof (STExecutionState));
    
    es->context = st_nil;
    es->receiver = st_nil;
    es->method = st_nil;
}

void
st_interpreter_enter (STExecutionState *es)
{
    interpreter_loop (es);
}


void
st_interpreter_main (void)
{
    st_oop context;
    st_oop result;
    STExecutionState es;

    create_doIt_method ();

    context = st_send_unary_message (st_nil,
				  st_nil,
				  st_symbol_new ("doIt"));
    es.context = st_nil;
    st_interpreter_set_active_context (&es, context);

    result = interpreter_loop (&es);
    g_assert (st_object_is_smi (result));

    printf ("result: %i\n", st_smi_value (result));
}
