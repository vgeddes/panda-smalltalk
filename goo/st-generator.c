/*
 * st-generator.c
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

#include "st-generator.h"

#include "st-types.h"
#include "st-object.h"
#include "st-symbol.h"
#include "st-hashed-collection.h"
#include "st-compiled-code.h"
#include "st-byte-array.h"
#include "st-universe.h"
#include "st-class.h"

#include <glib.h>
#include <string.h>
#include <stdlib.h>


#define DEFAULT_CODE_SIZE 50

typedef enum
{
    push_temp_n = 1,
    store_pop_temp_n,

    push_instvar_n,
    store_pop_instvar_n,

    push_literal_const_n,
    push_literal_var_n,

    store_literal_var_n,
    store_temp_n,
    store_instvar_n,

    push_self,
    push_nil,
    push_true,
    push_false,

    return_stack_top,

    pop_stack_top,
    duplicate_stack_top,

    push_active_context,

    create_block,

    jump_far_true,
    jump_far_false,
    jump_far,

    send,        /* B - B (arg count) - B (index of selector in literal frame) */ 
    send_super,

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
    send_identity_eq,
    send_class,
    send_new,
    send_new_arg,
    
} Code;

enum {
    SPECIAL_PLUS,
    SPECIAL_MINUS,
    SPECIAL_LT,
    SPECIAL_GT,
    SPECIAL_LE,
    SPECIAL_GE,
    SPECIAL_EQ,
    SPECIAL_NE,
    SPECIAL_MUL,
    SPECIAL_DIV,
    SPECIAL_MOD,
    SPECIAL_BITSHIFT,
    SPECIAL_BITAND,
    SPECIAL_BITOR,
    SPECIAL_BITXOR,
    
    SPECIAL_AT,
    SPECIAL_ATPUT,
    SPECIAL_SIZE,
    SPECIAL_VALUE,
    SPECIAL_IDEQ,
    SPECIAL_CLASS,
    SPECIAL_HASH,
    SPECIAL_NEW,
    SPECIAL_NEW_ARG,

    NUM_SPECIALS,
} STSpecialType;

typedef struct 
{
    st_oop method;
 
    GList   *temporaries;
    /* names of instvars, in order of location in the object */
    GList   *instvars;
    /* literal frame for the compiled code */
    GList   *literals;

    guchar  *code;
    guint    size;
    guint    alloc;
    
    guint    stack_depth;

} Generator;

st_oop specials[NUM_SPECIALS] = { 0 };
			      

static void
init_specials (void)
{
    /* arithmetic specials */
    specials[SPECIAL_PLUS]     = st_symbol_new ("+");
    specials[SPECIAL_MINUS]    = st_symbol_new ("-");
    specials[SPECIAL_LT]       = st_symbol_new ("<");
    specials[SPECIAL_GT]       = st_symbol_new (">");
    specials[SPECIAL_LE]       = st_symbol_new ("<=");
    specials[SPECIAL_GE]       = st_symbol_new (">=");
    specials[SPECIAL_EQ]       = st_symbol_new ("=");
    specials[SPECIAL_NE]       = st_symbol_new ("~=");
    specials[SPECIAL_MUL]      = st_symbol_new ("*");
    specials[SPECIAL_DIV]      = st_symbol_new ("/");
    specials[SPECIAL_MOD]      = st_symbol_new ("\\");
    specials[SPECIAL_BITSHIFT] = st_symbol_new ("bitShift:");
    specials[SPECIAL_BITAND]   = st_symbol_new ("bitAnd:");
    specials[SPECIAL_BITOR]    = st_symbol_new ("bitOr:");
    specials[SPECIAL_BITXOR]   = st_symbol_new ("bitXor:");
    
    /* message specials */
    specials[SPECIAL_AT]       = st_symbol_new ("at:");
    specials[SPECIAL_ATPUT]    = st_symbol_new ("at:put:");
    specials[SPECIAL_SIZE]     = st_symbol_new ("size");
    specials[SPECIAL_VALUE]    = st_symbol_new ("value");
    specials[SPECIAL_IDEQ]     = st_symbol_new ("==");
    specials[SPECIAL_CLASS]    = st_symbol_new ("class");
    specials[SPECIAL_HASH]     = st_symbol_new ("hash");
    specials[SPECIAL_NEW]      = st_symbol_new ("new");
    specials[SPECIAL_NEW_ARG]  = st_symbol_new ("new:");
}


static void generate_expression (Generator *gt, STNode *node);

static Generator *
generator_new (st_oop klass, GList *temps, GList *args)
{
    Generator *gt;

    gt = g_slice_new0 (Generator);

    gt->temporaries = g_list_concat (args, temps);
    gt->instvars    = st_behavior_all_instance_variables (klass);
    gt->literals    = NULL;

    gt->code  = g_malloc (DEFAULT_CODE_SIZE);
    gt->alloc = DEFAULT_CODE_SIZE;
    gt->size  = 0;

    gt->stack_depth = 0;
   
    return gt;
}

static st_oop
create_literals_array (Generator *gt)
{
    int    count;
    st_oop literals = st_nil;

    count = g_list_length (gt->literals);

    if (count > 0) {
	literals = st_object_new_arrayed (st_array_class, count); 

	int i = 1;
	for (GList *l = gt->literals; l; l = l->next) {
	    st_array_at_put (literals, i, (st_oop) l->data);
	    i++;
	}
    }

    return literals;
}

static st_oop
create_bytecode_array (Generator *gt)
{
    if (gt->size == 0)
	return st_nil;

    st_oop  bytecode;
    guchar   *bytes;

    bytecode = st_object_new_arrayed (st_byte_array_class, gt->size);
    bytes = st_byte_array_bytes (bytecode);
    memcpy (bytes, gt->code, gt->size);

    return bytecode;
}

static void
generation_error (const char *msg, STNode *node)
{
    printf ("error:%i: %s\n", node->line, msg);
    exit (1);
}

static void
resize (Generator *gt)
{
    gt->alloc += gt->alloc;
    gt->code = g_realloc (gt->code, gt->alloc);
}

static void
push (Generator *gt, guchar code)
{
    gt->size++;
    if (gt->size > gt->alloc)
	resize (gt);

    gt->code[gt->size - 1] = code;
}


static int
find_instvar (Generator *gt, st_oop name)
{   
    int i = 0;
    for (GList *l = gt->instvars; l; l = l->next) {

	if (st_object_equal (name, (st_oop) l->data))
	    return i;
	i++;
    }
    return -1;
}

static int
find_temporary (Generator *gt, st_oop name)
{   
    int i = 0;
    for (GList *l = gt->temporaries; l; l = l->next) {

	if (st_object_equal (name, (st_oop) l->data))
	    return i;
	i++;
    }
    return -1;
}

static int
find_literal_const (Generator *gt, st_oop literal)
{   
    int i = 0;
    for (GList *l = gt->literals; l; l = l->next) {

	if (st_object_equal (literal, (st_oop) l->data))
	    return i;
	i++;
    }
    gt->literals = g_list_append (gt->literals, (void *) literal);
    return i;
}

static int
find_literal_var (Generator *gt, st_oop name)
{
    /* check that variable binding exists */
    st_oop assoc = st_dictionary_association_at (st_smalltalk, name);
    if (assoc == st_nil)
	return -1;
    
    int i = 0;
    for (GList *l = gt->literals; l; l = l->next) {
	if (st_object_equal (assoc, (st_oop) l->data))
	    return i;
	i++;
    }
    gt->literals = g_list_append (gt->literals, (void *) assoc);
    return i;
}

static void
assign_temp (Generator *gt, int index)
{
    g_assert (index <= 255);

    push (gt, store_temp_n);
    push (gt, (guchar) index);
}

static void
assign_instvar (Generator *gt, int index)
{
    g_assert (index <= 255);

    push (gt, store_instvar_n);
    push (gt, (guchar) index); 
}

static void
assign_literal_var (Generator *gt, int index)
{
    g_assert (index <= 255);
    
    push (gt, store_literal_var_n);
    push (gt, (guchar) index); 
}

static void
push_temporary (Generator *gt, int index)
{
    g_assert (index <= 255);

    push (gt, push_temp_n);
    push (gt, (guchar) index);

    gt->stack_depth++;
}

static void
push_instvar (Generator *gt, int index)
{
    g_assert (index <= 255);

    push (gt, push_instvar_n);
    push (gt, (guchar) index);
    
    gt->stack_depth++;
}

static void
push_literal_var (Generator *gt, int index)
{
    g_assert (index <= 255);
    
    push (gt, push_literal_var_n);
    push (gt, (guchar) index);
    
    gt->stack_depth++;
}

static void
push_literal_const (Generator *gt, int index)
{
    g_assert (index <= 255);
  
    push (gt, push_literal_const_n);
    push (gt, (guchar) index);
    
    gt->stack_depth++;
}

static void
generate_assign (Generator *gt, STNode *node)
{
    int index;
    
    generate_expression (gt, node->expression);
    
    index = find_temporary (gt, node->assignee->name);
    if (index >= 0) {
	assign_temp (gt, index); 
	return;
    }

    index = find_instvar (gt, node->assignee->name);
    if (index >= 0) {
	assign_instvar (gt, index);
	return;
    }
 
    index = find_literal_var (gt, node->assignee->name);
    if (index >= 0) {
	assign_literal_var (gt, index);
	return;
    }
    
    generation_error ("could not resolve identifier", node);
}

static void
generate_return (Generator *gt, STNode *node)
{
    generate_expression (gt, node->expression);

    push (gt, return_stack_top);
}
#if 0			      
static int
generate_block (Generator *gt, STNode *node)
{
    int argcount = 0, tempcount = 0;
    st_oop block;

    GList *literals;
    guchar *code;
    int size, alloc, stack_depth;

    literals = gt->literals;
    code = gt->code;
    size = gt->size;
    alloc = gt->alloc;
    stack_depth = gt->stack_depth;

    for (STNode *l = node->arguments; l; l = l->next) {
	for (GList *t = gt->temporaries; t; t = t->next) {
	    if (st_object_equal ((st_oop) t->data, l->name)) {
		generation_error ("temporary name already used", l);
	    }
	}	
	gt->temporaries = g_list_append (gt->temporaries, (void *) l->name);
	argcount++;
    }

   for (STNode *l = node->temporaries; l; l = l->next) {
	for (GList *t = gt->temporaries; t; t = t->next) {
	    if (st_object_equal ((st_oop) t->data, l->name)) {
		generation_error ("temporary name already used", l);
	    }
	}	
	gt->temporaries = g_list_append (gt->temporaries, (void *) l->name);
	tempcount++;
    }

    generate_statements (generator, node->statements);

    block = st_object_new (st_compiled_block_class);
    
    st_compiled_code_set_bytecodes (block, create_bytecode_array (gt));
    st_compiled_code_set_literals  (block, create_literals_array (gt));

    st_compiled_code_set_arg_count (block, argcount);
    st_compiled_code_set_temp_count (block, tempcount);
 

    return -1;
}
#endif

static void
generate_message (Generator *gt, STNode *msg)
{
    STNode *args;
    guint   argcount = 0;
    bool    super_send = false;

    /* generate receiver */
    generate_expression (gt, msg->receiver);

    /* generate arguments */
    args = msg->arguments;
    for (; args; args = args->next) {
	generate_expression (gt, args);
	argcount++;
    }

    /* check if receiver is pseudo-variable 'super' */
    if (msg->receiver->type == ST_VARIABLE_NODE
	&& st_object_equal (msg->receiver->name, st_string_new ("super")))
	super_send = true;

    /* check if message is a special */
    for (int i = 0; i < G_N_ELEMENTS (specials); i++) {
	if (!super_send && msg->selector == specials[i]) {
	    push (gt, send_plus + i);  
		return;
	}
    }

    /* send type */
    if (super_send)
	push (gt, send_super);
    else
	push (gt, send);

    /* argument count */
    push (gt, (guchar) argcount);
	
    /* index of selector in literal frame */
    int index = find_literal_const (gt, msg->selector);
    push (gt, (guchar) index);
}

static void
generate_expression (Generator *gt, STNode *node)
{    
    switch (node->type) {
    case ST_VARIABLE_NODE:
    {		
	if (st_object_equal (node->name, st_string_new ("self"))
	    || st_object_equal (node->name, st_string_new ("super"))) {
	    push (gt, push_self);
	    gt->stack_depth++;
	    break;
	} else if (st_object_equal (node->name, st_string_new ("true"))) {
	    push (gt, push_true);
	    gt->stack_depth++;
	    break;
	} else if (st_object_equal (node->name, st_string_new ("false"))) {
	    push (gt, push_false);
	    gt->stack_depth++;
	    break;
	} else if (st_object_equal (node->name, st_string_new ("nil"))) {
	    push (gt, push_nil);
	    gt->stack_depth++;
	    break;
	}

	int index;	
	index = find_temporary (gt, node->name);
	if (index >= 0) {
	    push_temporary (gt, index);
	    break;
	}
	index = find_instvar (gt, node->name);
	if (index >= 0) {
	    push_instvar (gt, index);
	    break;
	}
	index = find_literal_var (gt, node->name);
	if (index >= 0) {
	    push_literal_var (gt, index);
	    break;
	}
	generation_error ("could not resolve identifier", node);
    }
    case ST_LITERAL_NODE:
    {
	int index;
	index = find_literal_const (gt, node->literal);
	push_literal_const (gt, index);
	break;
    }
    case ST_ASSIGN_NODE:
    {
	generate_assign (gt, node);
	break;
    }
    case ST_BLOCK_NODE:
    {	
	/*	int index;
	index = generate_block (gt, node);
	push_literal_const (gt, index);
	break;
	*/
    }
    case ST_MESSAGE_NODE:
    {
	generate_message (gt, node);
	break;
    }
    default:
	g_assert_not_reached ();
    }
} 

static void
generate_statements (Generator *gt, STNode *statements)
{
    for (STNode *stm = statements; stm; stm = stm->next) {

	switch (stm->type) {

	case ST_VARIABLE_NODE:
	case ST_LITERAL_NODE:
	case ST_BLOCK_NODE:
	    /*
	     * don't generate anything since we would end up with a constant
	     * expression with no side-effects
	     */
	    break;

	case ST_ASSIGN_NODE:
	    generate_assign (gt, stm);
	    break;
	    
	case ST_RETURN_NODE:
	    
	    generate_return (gt, stm);
	    
	    g_assert (stm->next == NULL);

	    /* break out and don't pop stack top */
	    return;
	    
	case ST_MESSAGE_NODE:
	    
	    generate_message (gt, stm);
	    break;
	    
	default:
	    g_assert_not_reached ();

	}

	push (gt, pop_stack_top);
    }

    gt->stack_depth++;

    push (gt, push_self);
    push (gt, return_stack_top);
}

st_oop
st_generate_method (st_oop klass, STNode *node)
{
    Generator *gt;
    st_oop     method;
    int argcount, tempcount;

    g_assert (klass != st_nil);
    g_assert (node != NULL && node->type == ST_METHOD_NODE);
    
    init_specials ();

    GList *temp_l = NULL;
    for (STNode *l = node->temporaries; l; l = l->next) {
	temp_l = g_list_append (temp_l, (void *) l->name);
    }
    GList *arg_l = NULL;
    for (STNode *l = node->arguments; l; l = l->next) {
	arg_l = g_list_append (arg_l, (void *) l->name);
    }

    gt = generator_new (klass, temp_l, arg_l);

    // generate bytecode
    generate_statements (gt, node->statements);

    method = st_object_new (st_compiled_method_class);

    argcount  = st_node_length (node->arguments);
    tempcount = st_node_length (node->temporaries);

    st_compiled_code_set_arg_count   (method, argcount);
    st_compiled_code_set_temp_count  (method, tempcount);
    st_compiled_code_set_stack_depth (method, gt->stack_depth);

    if (node->primitive >= 0) {
	st_compiled_code_set_flags (method, 1);	
	st_compiled_code_set_primitive_index (method, node->primitive);
    } else {
	st_compiled_code_set_flags (method, 0);	
    }

    st_compiled_code_set_literals (method, create_literals_array (gt));
    st_compiled_code_set_bytecodes (method, create_bytecode_array (gt)); 

    return method;
}


static void
print_bytecodes (st_oop literals, guchar *codes, int len) 
{
    guchar *end = codes + len;

    guchar *ip = codes;

    while (*ip) {

	printf ("%3i ", ip - codes);

	switch (*ip) {
	    
	case push_temp_n:
	    printf ("pushTemp: %i", *(++ip));
	    
	    ip++;
	    break;
	    
	case push_instvar_n:
	    printf ("pushInstvar: %i", *(++ip));

	    ip++;
	    break;
	    
	case push_literal_const_n:
	    printf ("pushConst: %i", *(++ip));

	    ip++;
	    break;
	    

	case push_literal_var_n:
	    printf ("pushLit: %i", *(++ip));

	    ip++;
	    break;
	    

	case push_self:
	    printf ("push: self");

	    ip++;
	    break;
	    

	case push_true:
	    printf ("pushConst: true");

	    ip++;
	    break;
	    

	case push_false:
	    printf ("pushConst: false");

	    ip++;
	    break;
	    

	case push_nil:
	    printf ("pushConst: nil");

	    ip++;
	    break;
	    

	case store_temp_n:
	    printf ("storeTemp: %i", *(++ip));

	    ip++;
	    break;
	    

	case store_instvar_n:
	    printf ("storeInstvar: %i", *(++ip));

	    ip++;
	    break;

	case store_pop_temp_n:
	    printf ("popIntoTemp: %i", *(++ip));

	    ip++;
	    break;

	case store_pop_instvar_n:
	    printf ("popIntoInstvar: %i", *(++ip));

	    ip++;
	    break;


	case pop_stack_top:
	    printf ("pop");

	    ip++;
	    break;

	case return_stack_top:
	    printf ("returnTop");

	    ip++;
	    break;

	case send:
	{
	    st_oop selector;
	    ip += 2;

	    selector = st_array_at (literals, *ip + 1);
	    printf ("send: #%s", (char *) st_byte_array_bytes (selector));

	    ip++;
	    break;
	}
	case send_super:
	{
	    st_oop selector;
	    ip += 2;

	    selector = st_array_at (literals, *ip + 1);
	    printf ("sendSuper: #%s", (char *) st_byte_array_bytes (selector));

	    ip++;
	    break;
	}


	case send_plus:
	case send_minus:
	case send_lt:
	case send_gt:
	case send_le:
	case send_ge:
	case send_eq:
	case send_ne:
	case send_mul:
	case send_div:
	case send_mod:
	case send_bitshift:
	case send_bitand:
	case send_bitor:
	case send_bitxor:
	case send_at:
	case send_at_put:
	case send_size:
	case send_identity_eq:
	case send_class:
	case send_new:
	case send_new_arg:

	    printf ("sendSpecial: #%s", st_byte_array_bytes (specials[*ip - send_plus]));

	    ip++;
	    break;
	default:

	g_assert_not_reached ();
	}
	
	printf ("\n");
    }
    
}


void st_generator_print_method (st_oop method)
{
    printf ("flags:      %3i\n", st_compiled_code_flags (method));
    printf ("arg-count:  %3i\n", st_compiled_code_arg_count (method));
    printf ("temp-count: %3i\n", st_compiled_code_temp_count (method));
    printf ("stack-depth:%3i\n", st_compiled_code_stack_depth (method));
    printf ("primitive:  %3i\n", st_compiled_code_primitive_index (method));
    
    printf ("\n");

    st_oop literals = st_compiled_code_literals (method);
    char *bytecodes = (guchar *) st_byte_array_bytes (st_compiled_code_bytecodes (method));
    int size = st_byte_array_size (st_compiled_code_bytecodes (method));

    print_bytecodes (literals, bytecodes, size);
    

}
