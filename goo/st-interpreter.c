
#include "st-types.h"
#include "st-compiler.h"
#include "st-universe.h"
#include "st-hashed-collection.h"
#include "st-symbol.h"
#include "st-object.h"
#include "st-class.h"
#include "st-context.h"
#include "st-primitives.h"
#include "st-compiled-method.h"
#include "st-byte-array.h"
#include "st-array.h"
#include "st-association.h"

/* 3 + 4 ! */
static st_oop
create_doIt_method (void)
{
    STError *error = NULL;
    
    static const char string1[] = 
	"doIt"
	"    | block |"
	"    block := [ :x | x increment ]."
	" ^  block value: 5";

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


static st_oop
send_unary_message (st_oop sender,
		    st_oop receiver,
		    st_oop selector)
{
    st_oop context;
    st_oop method;

    method = lookup_method (receiver, selector);
    g_assert (method != st_nil); 

    context = st_method_context_new (method);

    ST_CONTEXT_PART_SENDER (context) = sender;
    ST_METHOD_CONTEXT_METHOD (context) = method;

    ST_CONTEXT_PART_IP (context) = st_smi_new (0);
    ST_CONTEXT_PART_SP (context) = st_smi_new (0);

    ST_METHOD_CONTEXT_RECEIVER (context) = receiver;

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
	arguments[i] = es->stack[es->sp - argcount + i];

    ST_CONTEXT_PART_SENDER (context) = sender;
    ST_CONTEXT_PART_IP (context) = st_smi_new (0);
    ST_CONTEXT_PART_SP (context) = st_smi_new (0);
    ST_METHOD_CONTEXT_RECEIVER (context) = receiver;
    ST_METHOD_CONTEXT_METHOD (context) = method;

    es->sp -= argcount + 1;

    return context;
}


INLINE guchar *
activate_block_context (STExecutionState *es,
			st_oop context,
			st_oop values,
			guchar *ip)
{
    st_oop home;
    guint argcount;
    guint sp;
    
    argcount = st_smi_value (ST_BLOCK_CONTEXT_ARGCOUNT (context));
    
    /* block arguments can be on the stack or in an array `values' */
    if (values == st_nil) {
	
	st_oop *stack = ST_BLOCK_CONTEXT_STACK (context);
	for (guint i = 0; i < argcount; i++) {
	    stack[i] = es->stack[es->sp - argcount + i];
	}
    } else {
	
	st_oop *stack = ST_BLOCK_CONTEXT_STACK (context);
	for (guint i = 0; i < argcount; i++)
	    stack[i] = st_array_at (values, i + 1);
    }

    es->sp -= argcount + 1;    
    ST_CONTEXT_PART_SP (context) = st_smi_new (argcount);

    /* save executation state of active context */
    ST_CONTEXT_PART_IP (es->context) = st_smi_new (ip - st_compiled_method_code (es->method));
    ST_CONTEXT_PART_SP (es->context) = st_smi_new (es->sp);
    
    /* set caller */
    ST_BLOCK_CONTEXT_CALLER (context) = es->context;
    
    home = ST_BLOCK_CONTEXT_HOME (context);
    
    es->context  = context;
    es->method   = ST_METHOD_CONTEXT_METHOD (home);
    es->receiver = ST_METHOD_CONTEXT_RECEIVER (home);
    es->literals = st_array_element (st_compiled_method_literals (es->method), 1);
    es->temps    = st_method_context_temporary_frame (home);
    es->stack    = ST_BLOCK_CONTEXT_STACK (context);
    es->sp       = st_smi_value (ST_CONTEXT_PART_SP (context));
      
    return st_compiled_method_code (es->method) + st_smi_value (ST_BLOCK_CONTEXT_INITIAL_IP (context));
}

void
st_interpreter_set_active_context (STExecutionState *es,
				   st_oop  context)
{
    st_oop home;

    /* save executation state of active context */
    if (es->context != st_nil) {
	ST_CONTEXT_PART_IP (es->context) = st_smi_new (ip - st_compiled_method_code (es->method));
	ST_CONTEXT_PART_SP (es->context) = st_smi_new (es->sp);
    }
    
    if (st_object_class (context) == st_block_context_class) {

	home = ST_BLOCK_CONTEXT_HOME (context);
	
	es->method   = ST_METHOD_CONTEXT_METHOD (home);
	es->receiver = ST_METHOD_CONTEXT_RECEIVER (home);
	es->literals = st_array_element (st_compiled_method_literals (es->method), 1);
	es->temps    = st_method_context_temporary_frame (home);
	es->stack    = ST_BLOCK_CONTEXT_STACK (context);
    } else {
	es->method   = ST_METHOD_CONTEXT_METHOD (context);
	es->receiver = ST_METHOD_CONTEXT_RECEIVER (context);
	es->literals = st_array_element (st_compiled_method_literals (es->method), 1);
	es->temps    = st_method_context_temporary_frame (context);
	es->stack    = st_method_context_stack_frame (context);
    }

    es->context  = context;
    es->sp       = st_smi_value (ST_CONTEXT_PART_SP (context));
    es->ip       = st_smi_value (ST_CONTEXT_PART_IP (context));
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

static bool
execute_primitive (STExecutionState *es,
		   guint primitive_index,
                   guchar **ip_ret)
{
    es->success = true;

    st_primitives[primitive_index].func (es);

    *ip_ret = st_compiled_method_code (es->method) + es->ip;

    return es->success == true;
}


#define SEND_TEMPLATE(es, receiver, selector, argcount)			\
    st_oop method;							\
    st_oop context;							\
    guint  prim;							\
    STCompiledMethodFlags flags;					\
    									\
    ip += 1;								\
    									\
    method = lookup_method (st_object_class (receiver), selector);	\
									\
    if (method == st_nil) {						\
	context = send_does_not_understand (es, receiver, selector, argcount); \
	ip = activate_context (es, context, ip);			\
	break;								\
    }									\
									\
    flags = st_compiled_method_flags (method);				\
    if (flags == ST_COMPILED_METHOD_PRIMITIVE) {			\
    prim = st_compiled_method_primitive_index (method);			\
									\
        guchar *ip_ret = NULL;						\
    									\
        bool success = execute_primitive (es, prim, &ip_ret);		\
        if (success) {							\
            ip = ip_ret;						\
            break;							\
        }								\
    }									\
									\
    context = new_context (es, es->context,				\
			   receiver, method,				\
			   argcount);					\
    									\
    ip = activate_context (es, context, ip)


static st_oop
interpreter_loop (STExecutionState *es)
{
    register guchar *ip = st_compiled_method_code (es->method);
    
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
	     
	case PUSH_LITERAL_CONST:
	    
	    ST_STACK_PUSH (es, es->literals[ip[1]]);
	    
	    ip += 2;
	    break;
	    
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
	
	case PUSH_LITERAL_VAR:
	{
	     st_oop var;
	     
	     var = st_association_value (es->literals[ip[1]]);
	     
	     ST_STACK_PUSH (es, var);
	     
	     ip += 2;
	     break;
	}
	
	case SEND_PLUS:
	{
	    guint  argcount = 1;
	    st_oop selector = st_specials[ST_SPECIAL_PLUS];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	     
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	}

	case SEND_MINUS:
	{
	    guint  argcount = 1;
	    st_oop selector = st_specials[ST_SPECIAL_MINUS];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	    
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	}
	
	case SEND_MUL:
	{
	    guint  argcount = 1;
	    st_oop selector = st_specials[ST_SPECIAL_MUL];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	    
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	}
	
	case SEND_LT:
	{
	    guint  argcount = 1;
	    st_oop selector = st_specials[ST_SPECIAL_LT];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	    
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	}
	
	case SEND_GT:
	{
	    guint  argcount = 1;
	    st_oop selector = st_specials[ST_SPECIAL_GT];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	    
	    SEND_TEMPLATE (es, receiver, selector, argcount);

	    break;
	}
	
	case SEND_LE:
	{
	    guint  argcount = 1;
	    st_oop selector = st_specials[ST_SPECIAL_LE];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	     
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	}
	
	case SEND_GE:
	{
	    guint  argcount = 1;
	    st_oop selector = st_specials[ST_SPECIAL_GE];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	    
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	}
	
	case SEND_CLASS:
	{
	    guint  argcount = 0;
	    st_oop selector = st_specials[ST_SPECIAL_CLASS];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	    
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	 }
	
	case SEND_AT:
	{
	    guint  argcount = 1;
	    st_oop selector = st_specials[ST_SPECIAL_AT];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	    
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	 }
	
	case SEND_AT_PUT:
	{
	    guint  argcount = 2;
	    st_oop selector = st_specials[ST_SPECIAL_ATPUT];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	    
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	}
	
	case SEND_IDENTITY_EQ:
	{
	    guint  argcount = 1;
	    st_oop selector = st_specials[ST_SPECIAL_IDEQ];
	    st_oop receiver = es->stack[es->sp - argcount - 1];
	    
	    SEND_TEMPLATE (es, receiver, selector, argcount);
	    
	    break;
	}

	case SEND:
	{
	    st_oop receiver;
	    st_oop selector;
	    st_oop method;
	    st_oop context;
	    guint  argcount;
	    guint  prim;
	    STCompiledMethodFlags flags;
	    
	    argcount = ip[1];
	    selector = es->literals[ip[2]];
	    receiver = es->stack[es->sp - argcount - 1];
	    
	    ip += 3;
	    
	    method = lookup_method (st_object_class (receiver), selector);
	    
	    if (method == st_nil) {
		context = send_does_not_understand (es, receiver, selector, argcount);
		ip = activate_context (es, context, ip);
		break;
	    }
	    
	    /* call primitive function if there is one */
	    flags = st_compiled_method_flags (method);
	    if (flags == ST_COMPILED_METHOD_PRIMITIVE) {
		prim = st_compiled_method_primitive_index (method);

        guchar *ip_ret = NULL;

        bool success = execute_primitive (es, prim, &ip_ret);
        if (success) {
            ip = ip_ret;
            break;
        }
	    }
	    
	    context = new_context (es, es->context,
				   receiver, method,
				   argcount);
	    
	    ip = activate_context (es, context, ip);
	    
	    break;
	}
	
	case SEND_VALUE:
	{
	    st_oop block_context;
	    
	    block_context = ST_STACK_PEEK (es);
	    
	    ip += 1;
	    
	    ip = activate_block_context (es,
					 block_context,
					 st_nil,
					 ip);
	    
	    break;
	}
	
	case SEND_VALUE_ARG:
	 {
	     st_oop block;

	     block = es->stack[es->sp - 1 - 1];
	     ip += 1;

	     ip = activate_block_context (es,
					  block,
					  st_nil,
					  ip);
	     
	     break;
	 }
	 
	case POP_STACK_TOP:
	    
	    (void) ST_STACK_POP (es);
	    
	    ip += 1;
	    break;
	    
	case BLOCK_COPY:
	{
	    st_oop block;
	    guint argcount = ip[1];
	    guint initial_ip;
	    
	    ip += 2;
	    
	    initial_ip = ip - st_compiled_method_code (es->method) + 3;
	    
	    block = st_block_context_new (es->context, initial_ip, argcount);

	    ST_STACK_PUSH (es, block);
	    
	    break;
	}
	
	case RETURN_STACK_TOP:
	{
	    st_oop sender;
	    st_oop value;
	    
	    sender = ST_CONTEXT_PART_SENDER (es->context);
	    value = ST_STACK_PEEK (es);
	    
	    /* exit loop if sender is nil */
	    if (sender == st_nil)
		return value;
	    
	    ip = activate_context (es, sender, ip);
	    
	    /* push returned value onto stack */
	    ST_STACK_PUSH (es, value);
	    
	    break;
	}
	
	case BLOCK_RETURN:
	{
	    st_oop caller;
	    st_oop value;
	    
	    caller = ST_BLOCK_CONTEXT_CALLER (es->context);
	    value = ST_STACK_PEEK (es);

	    ip = activate_context (es, caller, ip);
	    
	    /* push returned value onto sender's stack */
	    ST_STACK_PUSH (es, value);
	    
	    break;
	}
	
	default:
	    
	    g_assert_not_reached ();
	}
	
    }
    
    return st_nil;
}

void
st_interpreter_main (void)
{
    st_oop context;
    st_oop result;
    STExecutionState es;

    create_doIt_method ();

    context = send_unary_message (st_nil,
				  st_undefined_object_class,
				  st_symbol_new ("doIt"));
    es.context = st_nil;

    activate_context (&es, context, NULL);

    result = interpreter_loop (&es);
    g_assert (st_object_is_smi (result));

    printf ("result: %i\n", st_smi_value (result));
}
