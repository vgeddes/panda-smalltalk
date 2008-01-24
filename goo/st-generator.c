

#include "st-generator.h"

#include "st-types.h"

typedef enum
{
    push_temp_0 = 1,
    push_temp_1,
    push_temp_2,
    push_temp_3,
    push_temp_4,
    push_temp_5,
    push_temp_6,
    push_temp_7,
    push_temp_8,
    push_temp_9,
    push_temp_10,
    push_temp_11,
    push_temp_12,
    push_temp_13,
    push_temp_14,
    push_temp_15,

    push_temp_n,

    store_pop_temp_0,


    store_pop_temp_n,

    push_instvar_0,
    push_instvar_1,
    push_instvar_2,
    push_instvar_3,
    push_instvar_4,
    push_instvar_5,
    push_instvar_6,
    push_instvar_7,
    push_instvar_8,
    push_instvar_9,
    push_instvar_10,
    push_instvar_11,
    push_instvar_12,
    push_instvar_13,
    push_instvar_14,
    push_instvar_15,

    push_instvar_n,

    store_pop_instvar_0,
    store_pop_instvar_1,
    store_pop_instvar_2,
    store_pop_instvar_3,
    store_pop_instvar_4,
    store_pop_instvar_5,
    store_pop_instvar_6,
    store_pop_instvar_7,
    store_pop_instvar_8,
    store_pop_instvar_9,
    store_pop_instvar_10,
    store_pop_instvar_11,
    store_pop_instvar_12,
    store_pop_instvar_13,
    store_pop_instvar_14,
    store_pop_instvar_15,

    store_pop_instvar_n,

    push_literal_const_0,
    push_literal_const_1,
    push_literal_const_2,
    push_literal_const_3,
    push_literal_const_4,
    push_literal_const_5,
    push_literal_const_6,
    push_literal_const_7,
    push_literal_const_8,
    push_literal_const_9,
    push_literal_const_10,
    push_literal_const_11,
    push_literal_const_12,
    push_literal_const_13,
    push_literal_const_14,
    push_literal_const_15,
    push_literal_const_16,
    push_literal_const_17,
    push_literal_const_18,
    push_literal_const_19,
    push_literal_const_20,
    push_literal_const_21,
    push_literal_const_22,
    push_literal_const_23,
    push_literal_const_24,
    push_literal_const_25,
    push_literal_const_26,
    push_literal_const_27,
    push_literal_const_28,
    push_literal_const_29,
    push_literal_const_30,
    push_literal_const_31,
    push_literal_const_n,

    push_literal_var_0,
    push_literal_var_1,
    push_literal_var_2,
    push_literal_var_3,
    push_literal_var_4,
    push_literal_var_5,
    push_literal_var_6,
    push_literal_var_7,
    push_literal_var_8,
    push_literal_var_9,
    push_literal_var_10,
    push_literal_var_11,
    push_literal_var_12,
    push_literal_var_13,
    push_literal_var_14,
    push_literal_var_15,
    push_literal_var_16,
    push_literal_var_17,
    push_literal_var_18,
    push_literal_var_19,
    push_literal_var_20,
    push_literal_var_21,
    push_literal_var_22,
    push_literal_var_23,
    push_literal_var_24,
    push_literal_var_25,
    push_literal_var_26,
    push_literal_var_27,
    push_literal_var_28,
    push_literal_var_29,
    push_literal_var_30,
    push_literal_var_31,
    
    push_literal_var_n,

    store_literal_var_n,
    store_temp_n,
    store_instvar_n,

    push_self,
    push_nil,
    push_true,
    push_false,
    push_minus_one,
    push_zero,
    push_one,
    push_two,

    return_stack_top,
    return_self,
    return_nil,
    return_true,
    return_false,

    pop_stack_top,
    duplicate_stack_top,

    push_active_context,

    create_block,

    jump_true,
    jump_false,
    jump,
    jump_far_true,
    jump_far_false,
    jump_far,

    send_0_args,
    send_1_args,
    send_2_args,
    send_n_args,
    send_n_args_super,

    send_plus,
    send_minus,
    send_lt,
    send_gt,
    send_le,
    send_ge,
    send_eq,
    send_ne,
    send_mul,
    send_div,
    send_mod,
    send_bitshift,
    send_bitand,
    send_bitor,
    send_bitxor,

    send_at,
    send_at_put,
    send_size,
    send_value,
    send_value_arg,
    send_identity_eq,
    send_class,
    send_hash,
    send_do,
    send_new,
    send_new_arg,
    
} code_t;

typedef struct 
{
    st_oop_t method;

    GList  *temporary_nams; 
    GList  *instvar_names;
    GList  *literals;
    int     primitive;

    guchar  code;
    guint   size;
    guint   alloc
    
    guint   stack_depth;

} generator_t;


#define DEFAULT_CODE_SIZE 50


static const * const arithmetic_specials[] = {
    "+", "-", "<", ">", "<=", ">=", "=", "~=",
    "*", "/", "\\", "bitShift:", "bitAnd:", "bitOr:", "bitXor:", 
};
static const * const message_specials[] = {
    "at:", "at:put:", "size", "value", "value:", "==", "class", "hash", "do:", "new", "new:",
};

static generator_t *
generator_new (st_oop_t klass, GList *temps, GList *args)
{
    generator_t *gt;

    gt = g_slice_new0 (generator_t);

    gt->temporary_names = g_list_concat (temps, args);
    gt->instvar_names = st_behavior_all_instance_variables (object);
    gt->literals  = NULL;
    gt->flags     = 0;
    gt->primitive = 0;

    gt->code  = g_malloc (DEFAULT_CODE_SIZE);
    gt->alloc = DEFAULT_CODE_SIZE;
    gt->size  = 0;

    gt->stack_depth = 0;
   
    return gt;
}

static void
generation_error (const char *msg, st_node_t *node)
{
    printf ("error:%i: %s\n", msg, node->line);
    abort ();
}

static void
resize (generator_t *gt)
{
    gt->alloc += gt->alloc;
    gt->code = g_realloc (gt->code, gt->alloc);
}

static void
push (generator_t *gt, guchar code)
{
    gt->size++;
    if (gt->size > gt->alloc)
	resize (gt);

    gt->code[gt->size - 1] = code;
}

static char *
node_identifier_name (st_node_t *node)
{
    g_assert (node->type == ST_NODE_IDENTIFIER);
    return ST_NODE_IDENTIFIER_CAST (node)->name;
}

static int
find_temporary (generator_t *gt, const char *name)
{   
    int i = 0;
    for (GList *l = gt->temporary_names; l; l = l->next) {

	if (streq (name, (char *) l->data))
	    return i;
	i++;
    }
    return -1;
}

static int
find_instvar (generator_t *gt, const char *name)
{    
    int i = 0;
    for (GList *l = gt->instvar_names; l; l = l->next) {

	if (streq (name, (char *) l->data))
	    return i;
	i++;
    }
    return -1;
}

static int
find_literal_var (generator_t *gt, const char *name)
{    
    st_oop_t assoc = st_dictionary_association_at (st_smalltalk, st_symbol_new (name));
    if (assoc == st_nil)
	return -1;
    
    int i = 0;
    for (GList *l = gt->literals; l; l = l->next) {
	if (st_object_equal (assoc, (st_oop_t) l->data))
	    return i;
	i++;
    }
    gt->literals = g_list_append (gt->literals, assoc);
    return i;
}

static int
find_literal_const (generator_t *gt, st_node_literal_t *node)
{
    st_node_type_t type = ((st_node_t *) node)->type;

    switch (type) {
    case ST_NODE_LITERAL_INTEGER:
    {
	int i = 0;
	for (GList *l = gt->literals; l; l = l->next) {
	    if (st_object_equal (node->integer,(st_oop_t) l->data))
		return i;
	    ++i;
	}
	gt->literals = g_list_append (gt->literals, node->integer);
	return i;
    }
    case ST_NODE_LITERAL_STRING:
    {
	st_oop_t string = st_string_new (node->string);
	int i = 0;   
	for (GList *l = gt->literals; l; l = l->next) {
	    if (st_object_equal (string, (st_oop_t) l->data))
		return i;
	    ++i;
	}
	gt->literals = g_list_append (gt->literals, string);
	return i;

    }
    case ST_NODE_LITERAL_SYMBOL:
    {
	st_oop_t symbol = st_symbol_new (node->string);
	int i = 0;
	for (GList *l = gt->literals; l; l = l->next) {
	    if (st_object_equal (symbol, (st_oop_t) l->data))
		return i;
	    ++i;
	}	
	gt->literals = g_list_append (gt->literals, symbol);
	return i;
    }
    case ST_NODE_LITERAL_CHARACTER:
    {
	st_oop_t character = st_object_new (st_character_class);
	st_character_set_value (character, node->character);
	int i = 0;
	for (GList *l = gt->literals; l; l = l->next) {
	    if (st_object_equal (character, (st_oop_t) l->data))
		return i;
	    ++i;
	}	
	gt->literals = g_list_append (gt->literals, character);
	return i;
    }
    default:
	g_assert_not_reached ();
	return -1;
    }
}

static void
assign_temp (generator_t *gt, int index, bool pop)
{
    g_assert (index <= 255);

    if (!pop) {
	push (gt, store_temp_n);
	push (gt, (char) index); 
    } else if (index > 15) {
	push (gt, store_pop_temp_n);
	push (gt, (char) index);
    } else {
	push (gt, (char) (store_pop_temp_0 + index));
    }
}

static void
assign_instvar (generator_t *gt, int index, bool pop)
{
    g_assert (index <= 255);

    if (!pop) {
	push (gt, store_instvar_n);
	push (gt, (char) index); 
    } else if (index > 15) {
	push (gt, store_pop_instvar_n);
	push (gt, (char) index);
    } else {
	push (gt, (char) (store_pop_instvar_0 + index));
    }
}

static void
assign_literal_var (generator_t *gt, int index, bool pop)
{
    g_assert (index <= 255);

    if (!pop) {
	push (gt, store_literal_var_n);
	push (gt, (guchar) index); 
    } else if (index > 31) {
	push (gt, store_pop_literal_var_n);
	push (gt, (guchar) index);
    } else {
	push (gt, (guchar) (store_pop_literal_var_0 + index));
    }
}

static void
push_temp (generator_t *gt, int index)
{
    g_assert (index <= 255);
    if (index > 15) {
	push (gt, push_temp_n);
	push (gt, (guchar) index);
    } else {
	push (gt, (guchar) (push_temp_0 + index));
    }
    gt->stack_depth++;
}

static void
push_instvar (generator_t *gt, int index)
{
    g_assert (index <= 255);
    if (index > 15) {
	push (gt, push_instvar_n);
	push (gt, (guchar) index);
    } else {
	push (gt, (guchar) (push_instvar_0 + index));
    }
    gt->stack_depth++;
}

static void
push_literal_var (generator_t *gt, int index)
{
    g_assert (index <= 255);
    if (index > 31) {
	push (gt, push_literal_var_n);
	push (gt, (guchar) index);
    } else {
	push (gt, (guchar) (push_literal_var_0 + index));
    }
    gt->stack_depth++;
}

static void
push_literal_const (generator_t *gt, int index)
{
    g_assert (index <= 255);
    if (index > 31) {
	push (gt, push_literal_const_n);
	push (gt, (guchar) index);
    } else {
	push (gt, (guchar) (push_literal_const_0 + index));
    }
    gt->stack_depth++;
}

static void
generate_assign (generator_t *gt, st_node_t *node, bool pop)
{
    st_node_assign_t *assign; = (st_node_assign_t *) node;
    char *identifier;
    int   index = 0;

    gen_expression (gt, assign->variable);

    identifier = node_identifier_name (assign->assignee);
    
    index = find_temporary (gt, identifier);
    if (index >= 0) {
	assign_temp (gt, index, pop); 
	return;
    }

    index = find_instvar (gt, identifier);
    if (index >= 0) {
	assign_instvar (gt, index, pop);
	return;
    }
 
    index = find_literal_var (gt, indentifier);
    if (index >= 0) {
	assign_literal_var (gt, index, pop);
	return;
    }
    
    gen_error ("could not resolve identifier", node);
}

static void
generate_return (generator_t *gt, st_node_return_t *node)
{
    st_node_t *variable = node->variable;

    // quick return case
    if (gt->top == 0 && variable->type == ST_NODE_IDENTIFIER) {

	char *identifier = node_identifier_name (variable);

	if (streq (identifier, "self")) {
	    st_compiled_code_set_flag (gt->method, ST_COMPILED_CODE_RETURN_RECEIVER);
	    return;

	}
	if (streq (identifier, "nil")) {
	    st_compiled_code_set_flag (gt->method, ST_COMPILED_CODE_RETURN_LITERAL);
	    st_compiled_code_set_literal_type (gt->method, ST_COMPILED_CODE_LITERAL_NIL);
	    return;

	}
	if (streq (identifier, "true")) {
	    st_compiled_code_set_flag (gt->method, ST_COMPILED_CODE_RETURN_LITERAL);
	    st_compiled_code_set_literal_type (gt->method, ST_COMPILED_CODE_LITERAL_TRUE);
	    return;

	}
       if (streq (identifier, "false")) {
	    st_compiled_code_set_flag (gt->method, ST_COMPILED_CODE_RETURN_LITERAL);
	    st_compiled_code_set_literal_type (gt->method, ST_COMPILED_CODE_LITERAL_FALSE);
	    return;
	}

       int index = find_identifier (gt, identifier);
       if (index >= 0) {
	    st_compiled_code_set_flag (gt->method, ST_COMPILED_CODE_RETURN_INSTVAR);
	    st_compiled_code_set_instvar_index (gt->method, index);
	    return;
       }
    }

    // quick return case
    if (gt->top == 0 && value->type == ST_NODE_INTEGER) {

	st_smi_t integer = ((st_node_literal_t *) variable)->integer;

	if (integer == -1) {
	    st_compiled_code_set_flag (gt->method, ST_COMPILED_CODE_RETURN_LITERAL);
	    st_compiled_code_set_literal_type (gt->method, ST_COMPILED_CODE_LITERAL_MINUS_ONE);
	    return;
	}
	if (integer == 0) {
	    st_compiled_code_set_flag (gt->method, ST_COMPILED_CODE_RETURN_LITERAL);
	    st_compiled_code_set_literal_type (gt->method, ST_COMPILED_CODE_LITERAL_ZERO);
	    return;
	}
	if (integer == 1) {
	    st_compiled_code_set_flag (gt->method, ST_COMPILED_CODE_RETURN_LITERAL);
	    st_compiled_code_set_literal_type (gt->method, ST_COMPILED_CODE_LITERAL_ONE);
	    return;
	}
	if (integer == 2) {
	    st_compiled_code_set_flag (gt->method, ST_COMPILED_CODE_RETURN_LITERAL);
	    st_compiled_code_set_literal_type (gt->method, ST_COMPILED_CODE_LITERAL_TWO);
	    return;
	}
    }

    if (variable->type == ST_NODE_IDENTIFIER) {

	char *identifier = node_identifier_name (variable);

	if (streq (identifier, "self")) {
	    append (gt, return_self);
	    return;
	}
	if (streq (identifier, "nil")) {
	    append (gt, return_nil);
	    return;
	}
	if (streq (identifier, "true")) {
	    append (gt, return_true);
	    return;
	}
	if (streq (identifier, "false")) {
	    append (gt, return_false);
	    return;
	}
    }
    
    generate_expression (gt, variable);
    push (gt, return_stack_top);
}

static int

static st_oop_t
generate_block (st_oop_t object, st_node_block_t *node)
{
    st_oop_t *block;

    block = st_object_new (st_compiled_block_class);
}




static void
generate_message_send (generator_t *gt, st_node_expression_t *expression)
{
    st_node_t *receiver;
    bool super_send = false;


    if (expression->message->type == ST_NODE_MESSAGE_KEYWORD) {

	st_node_keyword_message_t *message = (st_node_keyword_message_t *) expression->message;
	GList *args = message->arguments;
	st_node_block_t *true_block, *false_block;


	

    }

    
}

static void
generate_expression (generator_t *gt, st_node_t *node)
{
    st_node_expression_t *expression = (st_node_expression_t*) node;
    
    st_node_t *receiver = expression->receiver;
    st_node_t *message = expression->message;
    
    switch (node->type) {

    case ST_NODE_IDENTIFIER:
    {	
	int   index;
	char *identifier = node_identifier_name (receiver);
	
	if (streq (identifier, "self") || streq (identifier, "super")) {
	    push (gt, push_self);
	    gt->stack_depth++;
	    break;
	} else if (streq (identifier, "true")) {
	    push (gt, push_true);
	    gt->stack_depth++;
	    break;
	} else if (streq (identifier, "false")) {
	    push (gt, push_false);
	    gt->stack_depth++;
	    break;
	}
	
	index = find_temporary (gt, identifier);
	if (index >= 0) {
	    push_temporary (gt, index);
	    break;
	}
	index = find_instvar (gt, identifier);
	if (index >= 0) {
	    push_instvar (gt, index);
	    break;
	}
	index = find_literal_var (gt, identifier);
	if (index >= 0) {
	    push_literal_var (gt, index);
	    break;
	}
	generation_error ("could not resolve identifier", receiver);
    }
    case ST_NODE_LITERAL_INTEGER:
    case ST_NODE_LITERAL_FLOAT:
    case ST_NODE_LITERAL_STRING:
    case ST_NODE_LITERAL_SYMBOL:
    case ST_NODE_LITERAL_CHARACTER:
    {
	st_node_literal_t *literal = (st_node_literal_t *) receiver;

	if (node->type = ST_NODE_LITERAL_INTEGER && literal->integer == -1) {
	    push (gt, push_minus_one);
	    gt->stack_depth++;
	    break;
	} else if (node->type = ST_NODE_LITERAL_INTEGER && literal->integer == 0) {
	    push (gt, push_zero);
	    gt->stack_depth++;
	    break;
	} else if (node->type = ST_NODE_LITERAL_INTEGER && literal->integer == 1) {
	    push (gt, push_one);
	    gt->stack_depth++;
	    break;
	} else if (node->type = ST_NODE_LITERAL_INTEGER && literal->integer == 2) {
	    push (gt, push_two);
	    gt->stack_depth++;
	    break;
	}

	int index;
	index = find_literal_const (gt, node);
	push_literal_const (gt, index);
	break;
    }
    case ST_NODE_ASSIGN:
    {
	generate_assign (gt, );
	


    }
    case ST_NODE_BLOCK:
    {	
	int index;
	index = generate_block (gt, (st_node_block_t *) node);
	push_literal_const (gt, index);
    }
    case ST_NODE_EXPRESSION:
    {
	st_node_expression_t *expression = (st_node_expression_t *) node;
	
	generate_message_send (gt, expression);
    }
    default:
	// foo
    }
} 

static void
generate_statements (generator_t *gt, GList *statements)
{
    for (GList *l = statements; l; l = l->next) {

	st_node_t *node = (st_node_t *) l->data;

	switch (node->type) {

	case ST_NODE_IDENTIFIER:
	case ST_NODE_LITERAL_SYMBOL:
	case ST_NODE_LITERAL_STRING:
	case ST_NODE_LITERAL_INTEGER:
        case ST_NODE_LITERAL_LARGE_INTEGER:
        case ST_NODE_LITERAL_FLOAT:
        case ST_NODE_LITERAL_CHARACTER:
	    /*
	     * don't generate anything since no work is being done
	     */
	    break;

	case ST_NODE_ASSIGN:
	    generate_assign (gt, node, true);
	    break;
	    
	case ST_NODE_RETURN:

	    generate_return (gt, node);
	    break;

	}

	gen_expression (gt, node);

	/* end of statement, so we can discard the last value on the stack */
	append (gt, pop_stack_top);
	gt->stack_depth--;}
}

st_oop_t
st_generate_method (st_oop_t klass, st_node_method_t *node)
{
    generator_t *gt;
    st_oop_t     method;

    g_assert (object != st_nil);
    g_assert (node != NULL);
    
    gt = generator_new (klass, node->temps, node->args);

    // generate bytecode
    generate_statements (gt, node->statements);

    method = st_object_new (st_compiled_method_class);

    int arg_count = g_list_length (node->args);
    int temp_count = g_list_length (node->temps);

    st_compiled_code_set_arg_count   (method, arg_count);
    st_compiled_code_set_temp_count  (method, temp_count);
    st_compiled_code_set_stack_depth (method, gt->stack_depth);

    int literal_count = g_list_length (gt->literals);
    if (literal_count > 0) {
	st_oop_t literals;
	literals = st_object_new_arrayed (st_array_class, literal_count); 

	int i = 1;
	for (GList *l = gt->literals; l; l = l->next)
	    st_array_at_put (literals, i, (st_oop_t) l->data);
	
	st_compiled_code_set_literals (method, literals);
    }

    if ((gt->flags == 0 || gt->flags == 4) && gt->top > 0) {
	st_oop_t  bytecode;
	guchar   *bytes;
	int       size;

	size = gt->top + 1;
	bytecode = st_object_new_array (st_byte_array_class, size);
	bytes = st_byte_array_bytes (bytecode);
	memcpy (bytes, gt->code, size);
	
	st_compiled_method_set_bytecode (method, bytecode);
    }

    return method;
}
