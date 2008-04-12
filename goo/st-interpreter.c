
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
    
    static const char string[] = 
	"doIt"
	"    ^ 3 - 12 + 2";

    st_compile_string (st_undefined_object_class, string, &error);
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
    if (!method)
	return st_nil;

    stack_size = st_compiled_method_stack_depth (method); 

    context = st_method_context_new (stack_size);

    ST_CONTEXT_PART_SENDER (context) = sender;
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
	    guint  prim;

	    receiver = state->stack[state->sp - 1];
	    
	    method = lookup_method (st_object_class (receiver), st_symbol_new ("+"));
	    g_assert (method != st_nil);

	    prim = st_compiled_method_primitive_index (method);
	    
	    if (prim < 255)
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
	    guint  prim;

	    receiver = state->stack[state->sp - 1];
	    
	    method = lookup_method (st_object_class (receiver), st_symbol_new ("-"));
	    g_assert (method != st_nil);

	    prim = st_compiled_method_primitive_index (method);
	    
	    if (prim < 255)
		execute_primitive (state, prim, method);
	    else
		g_assert_not_reached ();

	    ip++;
	    break;
	}
	case POP_STACK_TOP:

	    ST_STACK_POP (state);

	    ip++;
	    break;

	case RETURN_STACK_TOP:
	
	    return ST_STACK_PEEK (state);
    
	    ip++;
	    break;
	
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
    state->stack = ST_METHOD_CONTEXT_STACK (context);
}

#define DUMMY_CONTEXT_STACK_SIZE 1


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
