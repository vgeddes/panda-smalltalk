
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

/* 3 + 4 ! */
static st_oop
create_doIt_method (void)
{
    STError *error = NULL;
    
    static const char string1[] = 
	"doIt"
	"    | x y |"
	"    x := 2 + 1."
	"    y := 2 - 1."
	"    ^ (x + y) increment";

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

static void
execute_primitive (STInterpreter *state,
		   guint index,
		   st_oop method)
{
    
    STPrimitiveFunc func;

    func = st_primitives[index].func;

    func (state);
}

static st_oop
send_unary_message (st_oop sender,
		    st_oop receiver,
		    st_oop selector)
{
    st_oop context;
    st_oop method;
    int stack_size;

    method = lookup_method (receiver, selector);
    g_assert (method != st_nil); 

    context = st_method_context_new (method);

    ST_CONTEXT_PART_SENDER (context) = sender;
    ST_CONTEXT_PART_METHOD (context) = method;

    ST_CONTEXT_PART_IP (context) = st_smi_new (0);
    ST_CONTEXT_PART_SP (context) = st_smi_new (0);

    ST_METHOD_CONTEXT_RECEIVER (context) = receiver;

    return context;
}

static st_oop
create_new_context (st_oop parent, st_oop receiver, st_oop method)
{
    st_oop context;

    context = st_method_context_new (method);

    ST_CONTEXT_PART_SENDER (context) = parent;
    ST_CONTEXT_PART_METHOD (context) = method;

    ST_CONTEXT_PART_IP (context) = st_smi_new (0);
    ST_CONTEXT_PART_SP (context) = st_smi_new (0);

    ST_METHOD_CONTEXT_RECEIVER (context) = receiver;

    return context;
}

/* static void */
/* execute_method (STInterpreter *state, */
/* 		st_oop         receiver, */
/* 		st_oop         method) */
/* { */
/*     int prim; */
/*     st_oop context; */

/*     prim = st_compiled_method_primitive_index (method); */

/*     if (prim < 255) { */
/* 	execute_primitive (state, prim, method); */
/* 	return; */
/*     } */

/*     /\* */
/*     context = create_new_context (state, receiver, method); */

/*     activate_context (state, context); */

/*     *\/ */
/* } */

static st_oop
interpreter_loop (STInterpreter *state)
{
    register guchar *ip = state->bytecodes;

    for (;;) {

	switch (*ip) {

	case STORE_POP_TEMP:
	
	    state->temps[ip[1]] = ST_STACK_POP (state);
	    
	    ip += 2;
	    break;

	case STORE_TEMP:
	
	    state->temps[ip[1]] = ST_STACK_PEEK (state);
	    
	    ip += 2;
	    break;

	case PUSH_TEMP:
	
	    ST_STACK_PUSH (state, state->temps[ip[1]]);
	    
	    ip += 2;
	    break;

	case PUSH_SELF:
	    
	    ST_STACK_PUSH (state, state->receiver);
	    
	    ip++;
	    break;
	    
	case PUSH_TRUE:
	    
	    ST_STACK_PUSH (state, st_true);

	    ip++;
	    break;

	case PUSH_FALSE:
	    
	    ST_STACK_PUSH (state, st_false);

	    ip++;
	    break;

	case PUSH_NIL:
	    
	    ST_STACK_PUSH (state, st_nil);

	    ip++;
	    break;

	case PUSH_LITERAL_CONST:
	{     
	    ST_STACK_PUSH (state, state->literals[ip[1]]);

	    ip += 2;
	    break;

	}
	case SEND_PLUS:
	{
	    st_oop receiver;
	    st_oop method;
	    STCompiledMethodFlags flags;
	    guint  prim;

	    receiver = state->stack[state->sp - 1];
	    
	    method = lookup_method (st_object_class (receiver), st_symbol_new ("+"));
	    g_assert (method != st_nil);

	    flags = st_compiled_method_flags (method);
	    prim = st_compiled_method_primitive_index (method);

	    if (flags == ST_COMPILED_METHOD_PRIMITIVE)
		execute_primitive (state, prim, method);
	    else
		g_assert_not_reached ();

	    ip++;
	    break;
	}
	case SEND_MINUS:
	{
	    st_oop receiver;
	    st_oop method;
	    STCompiledMethodFlags flags;
	    guint  prim;

	    receiver = state->stack[state->sp - 1];
	    
	    method = lookup_method (st_object_class (receiver), st_symbol_new ("-"));
	    g_assert (method != st_nil);

	    flags = st_compiled_method_flags (method);
	    prim = st_compiled_method_primitive_index (method);

	    if (flags == ST_COMPILED_METHOD_PRIMITIVE)
		execute_primitive (state, prim, method);
	    else
		g_assert_not_reached ();

	    ip++;
	    break;
	}
	case SEND:
	{
	    st_oop receiver;
	    guint argcount;
	    st_oop selector;
	    guint prim;
	    st_oop method;

	    argcount = ip[1];
	    selector = state->literals[ip[2]];

	    receiver = state->stack[state->sp - argcount];

	    method = lookup_method (st_object_class (receiver), selector);
	    g_assert (method != st_nil);
	    
	    /* create new context */
	    st_oop context;
	    
	    context = create_new_context (state->context, receiver, method);
	    
	    ip += 3;

	    /* pop receiver and args off the stack */
	    state->sp -= argcount + 1;

	    /* activate context */
	    
	    state->receiver = ST_METHOD_CONTEXT_RECEIVER (context);
	    state->method = method;
	    
	    /* store current executation state */ 
	    ST_CONTEXT_PART_IP (state->context) = st_smi_new (ip - state->bytecodes);
	    ST_CONTEXT_PART_SP (state->context) = st_smi_new (state->sp);
	    	    
	    state->sp = 0;
	    state->ip = 0;

	    state->bytecodes = st_byte_array_bytes (st_compiled_method_bytecodes (method));
	    state->literals = st_array_element (st_compiled_method_literals (method), 1);
	    
	    state->temps = st_method_context_temporary_frame (context);
	    state->stack = st_method_context_stack_frame (context);

	    state->context = context;

	    ip = state->bytecodes;
	    break;
	}
	
	case POP_STACK_TOP:

	    ST_STACK_POP (state);

	    ip++;
	    break;

	case RETURN_STACK_TOP:
	{
	    st_oop sender;
	    st_oop value;

	    sender = ST_CONTEXT_PART_SENDER (state->context);
	    value = ST_STACK_PEEK (state);

	    if (sender == st_nil)
		return value;

	    state->context = sender;

	    state->receiver = ST_METHOD_CONTEXT_RECEIVER (sender);
	    state->method = ST_CONTEXT_PART_METHOD (sender);
	    
	    state->bytecodes = st_byte_array_bytes (st_compiled_method_bytecodes (state->method));

	    /* resume executation state */ 
	    state->ip = st_smi_value (ST_CONTEXT_PART_IP (sender));
	    state->sp = st_smi_value (ST_CONTEXT_PART_SP (sender));

	    state->bytecodes = st_byte_array_bytes (st_compiled_method_bytecodes (state->method));
	    state->literals = st_array_element (st_compiled_method_literals (state->method), 1);
	    
	    state->temps = st_method_context_temporary_frame (sender);
	    state->stack = st_method_context_stack_frame (sender);

	    state->context = sender;

	    /* push value onto stack */
	    ST_STACK_PUSH (state, value);


	    /* restore instruction pointer */
	    ip = state->bytecodes + st_smi_value (ST_CONTEXT_PART_IP (sender));
	    break;
	}
	default:
	    
	    g_assert_not_reached ();
	}

 
    }
    
    return st_nil;
}

static void
switch_context (STInterpreter *state,
		st_oop         context)
{
    state->context = context;

    state->receiver = ST_METHOD_CONTEXT_RECEIVER (context);

    state->method = ST_CONTEXT_PART_METHOD (context);

    state->ip = st_smi_value (ST_CONTEXT_PART_IP (context));
    state->sp = st_smi_value (ST_CONTEXT_PART_SP (context));
    
    state->bytecodes = st_byte_array_bytes (st_compiled_method_bytecodes (state->method));
    state->literals = st_array_element (st_compiled_method_literals (state->method), 1);

    state->temps = st_method_context_temporary_frame (context);
    state->stack = st_method_context_stack_frame (context);
}


void
st_interpreter_main (void)
{
    st_oop context;

    STInterpreter *state;

    state = g_slice_new0 (STInterpreter);

    create_doIt_method ();

    context = send_unary_message (st_nil,
				  st_undefined_object_class,
				  st_symbol_new ("doIt"));

    switch_context (state, context);

    st_oop result;

    result = interpreter_loop (state);

    g_assert (st_object_is_smi (result));

    printf ("%i\n", st_smi_value (result));

    g_slice_free (STInterpreter, state);
}
