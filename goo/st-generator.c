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
    PUSH_TEMP = 1,
    PUSH_INSTVAR,
    PUSH_LITERAL_CONST,
    PUSH_LITERAL_VAR,

    STORE_LITERAL_VAR,
    STORE_TEMP,
    STORE_INSTVAR,

    STORE_POP_LITERAL_VAR,
    STORE_POP_TEMP,
    STORE_POP_INSTVAR,

    PUSH_SELF,
    PUSH_NIL,
    PUSH_TRUE,
    PUSH_FALSE,

    RETURN_STACK_TOP,
    BLOCK_RETURN,

    POP_STACK_TOP,
    DUPLICATE_STACK_TOP,

    PUSH_ACTIVE_CONTEXT,

    CREATE_BLOCK,

    JUMP_TRUE,
    JUMP_FALSE,
    JUMP,

    SEND,        /* B - B (ARG COUNT) - B (INDEX OF SELECTOR IN LITERAL FRAME) */ 
    SEND_SUPER,

    SEND_PLUS,
    SEND_MINUS,
    SEND_LT,
    SEND_GT,
    SEND_LE,
    SEND_GE,
    SEND_EQ,
    SEND_NE,
    SEND_MUL,
    SEND_DIV,
    SEND_MOD,
    SEND_BITSHIFT,
    SEND_BITAND,
    SEND_BITOR,
    SEND_BITXOR,

    SEND_AT,
    SEND_AT_PUT,
    SEND_SIZE,
    SEND_VALUE,
    SEND_VALUE_ARG,
    SEND_IDENTITY_EQ,
    SEND_CLASS,
    SEND_NEW,
    SEND_NEW_ARG,
    
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
    SPECIAL_VALUE_ARG,
    SPECIAL_IDEQ,
    SPECIAL_CLASS,
    SPECIAL_NEW,
    SPECIAL_NEW_ARG,

    NUM_SPECIALS,
} STSpecialType;

typedef struct 
{
    bool     in_block;

    st_oop   klass;
 
    /* names of temporaries, in order of appearance */
    GList   *temporaries;
    /* names of instvars, in order they were defined */
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
    specials[SPECIAL_AT]        = st_symbol_new ("at:");
    specials[SPECIAL_ATPUT]     = st_symbol_new ("at:put:");
    specials[SPECIAL_SIZE]      = st_symbol_new ("size");
    specials[SPECIAL_VALUE]     = st_symbol_new ("value");
    specials[SPECIAL_VALUE_ARG] = st_symbol_new ("value:");
    specials[SPECIAL_IDEQ]      = st_symbol_new ("==");
    specials[SPECIAL_CLASS]     = st_symbol_new ("class");
    specials[SPECIAL_NEW]       = st_symbol_new ("new");
    specials[SPECIAL_NEW_ARG]   = st_symbol_new ("new:");
}

static void generate_expression (Generator *gt, STNode *node);
static void generate_statements (Generator *gt, STNode *statements);

static void
generation_error (const char *msg, STNode *node)
{
    printf ("error:%i: %s\n", node->line, msg);
    exit (1);
}

static GList *
get_temporaries (GList  *instvars,
		 STNode *arguments,
		 STNode *temporaries)
{
    GList *temps = NULL; 
        
    for (STNode *n = arguments; n; n = n->next) {
	
	for (GList *l = instvars; l; l = l->next) {
	    if (st_object_equal (n->name, (st_oop) l->data))
		generation_error ("name is already defined", arguments);
	}	
	temps = g_list_prepend (temps, (gpointer) n->name);
    }

    for (STNode *n = temporaries; n; n = n->next) {
	
	for (GList *l = instvars; l; l = l->next) {
	    if (st_object_equal (n->name, (st_oop) l->data))
		generation_error ("name is already defined", temporaries);
	}
	temps = g_list_prepend (temps, (gpointer) n->name);
    }

    return g_list_reverse (temps);
}

static Generator *
generator_new (st_oop klass, STNode *method)
{
    Generator *gt;

    gt = g_slice_new0 (Generator);

    gt->klass    = klass;
    gt->instvars = st_behavior_all_instance_variables (klass);
    gt->literals = NULL;
    gt->temporaries = get_temporaries (gt->instvars, method->arguments, method->temporaries);

    gt->code  = g_malloc (DEFAULT_CODE_SIZE);
    gt->alloc = DEFAULT_CODE_SIZE;
    gt->size  = 0;
    gt->stack_depth = 0;
   
    return gt;
}

static void
generator_destroy (Generator *gt)
{
    g_list_free (gt->instvars);
    g_list_free (gt->temporaries);
    g_list_free (gt->literals);

    g_free (gt->code);
    g_slice_free (Generator, gt);
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
push (Generator *gt, guchar code)
{
    gt->size++;

    if (gt->size > gt->alloc) {
	gt->alloc += gt->alloc;
	gt->code = g_realloc (gt->code, gt->alloc);
    }

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
assign_temp (Generator *gt, int index, bool pop)
{
    g_assert (index <= 255);

    if (pop)
	push (gt, STORE_POP_TEMP);
    else
	push (gt, STORE_TEMP);
    push (gt, (guchar) index);
}

static void
assign_instvar (Generator *gt, int index, bool pop)
{
    g_assert (index <= 255);

    if (pop)
	push (gt, STORE_POP_INSTVAR);
    else
	push (gt, STORE_INSTVAR);
    push (gt, (guchar) index); 
}

static void
assign_literal_var (Generator *gt, int index, bool pop)
{
    g_assert (index <= 255);
    
    if (pop)
	push (gt, STORE_POP_LITERAL_VAR);
    else
	push (gt, STORE_LITERAL_VAR);
    push (gt, (guchar) index); 
}

static void
push_temporary (Generator *gt, int index)
{
    g_assert (index <= 255);

    push (gt, PUSH_TEMP);
    push (gt, (guchar) index);

    gt->stack_depth++;
}

static void
push_instance_var (Generator *gt, int index)
{
    g_assert (index <= 255);

    push (gt, PUSH_INSTVAR);
    push (gt, (guchar) index);
    
    gt->stack_depth++;
}

static void
push_literal_var (Generator *gt, int index)
{
    g_assert (index <= 255);
    
    push (gt, PUSH_LITERAL_VAR);
    push (gt, (guchar) index);
    
    gt->stack_depth++;
}

static void
push_literal_const (Generator *gt, int index)
{
    g_assert (index <= 255);
  
    push (gt, PUSH_LITERAL_CONST);
    push (gt, (guchar) index);
    
    gt->stack_depth++;
}

static void
generate_assign (Generator *gt, STNode *node, bool pop)
{
    int index;
    
    generate_expression (gt, node->expression);
    
    index = find_temporary (gt, node->assignee->name);
    if (index >= 0) {
	assign_temp (gt, index, pop); 
	return;
    }

    index = find_instvar (gt, node->assignee->name);
    if (index >= 0) {
	assign_instvar (gt, index, pop);
	return;
    }
 
    index = find_literal_var (gt, node->assignee->name);
    if (index >= 0) {
	assign_literal_var (gt, index, pop);
	return;
    }
    
    generation_error ("unknown variable", node);
}

static void
generate_return (Generator *gt, STNode *node)
{
    generate_expression (gt, node->expression);

    push (gt, RETURN_STACK_TOP);
}

static void
create_block (Generator *gt, int index)
{
    g_assert (index <= 255);

   push (gt, CREATE_BLOCK);
   push (gt, (guchar) index);
} 


static GList *
get_block_temporaries (Generator *gt, STNode *arguments, STNode *temporaries)
{
    GList *temps = NULL;

    for (STNode *node = arguments; node; node = node->next) {
	
	for (GList *l = gt->instvars; l; l = l->next) {
	    if (st_object_equal (node->name, (st_oop) l->data))
		generation_error ("name is already defined", node);
	}
	for (GList *l = gt->temporaries; l; l = l->next) {
	    if (st_object_equal ( node->name, (st_oop) l->data))
		generation_error ("name already used in method", node);
	}
	temps = g_list_prepend (temps, (void *) node->name);
    }
    
    for (STNode *node = temporaries; node; node = node->next) {
	
	for (GList *l = gt->instvars; l; l = l->next) {
	    if (st_object_equal (node->name, (st_oop) l->data))
		generation_error ("name is already defined", node);
	}
	for (GList *l = gt->temporaries; l; l = l->next) {
	    if (st_object_equal ( node->name, (st_oop) l->data))
		generation_error ("name already used in method", node);
	}
	temps = g_list_prepend (temps, (void *) node->name);
    }

    return g_list_reverse (temps);
}

static int
generate_block (Generator *gt, STNode *node)
{
    st_oop block;
    bool nested;

    GList *literals;
    guchar *code;
    int size, alloc, stack_depth;

    /* save previous state */
    literals = gt->literals;
    code     = gt->code;
    size     = gt->size;
    alloc    = gt->alloc;
    stack_depth = gt->stack_depth;

    gt->literals = NULL;
    gt->code  = g_malloc (DEFAULT_CODE_SIZE);
    gt->alloc = DEFAULT_CODE_SIZE;
    gt->size  = 0;
    gt->stack_depth = 0;

    nested = gt->in_block;
    gt->in_block = true;

    gt->temporaries = g_list_append (gt->temporaries,
				     get_block_temporaries (gt, node->arguments,
							    node->temporaries));

    generate_statements (gt, node->statements);

    block = st_object_new (st_compiled_block_class);
    
    st_compiled_code_set_bytecodes (block, create_bytecode_array (gt));
    st_compiled_code_set_literals  (block, create_literals_array (gt));

    st_compiled_code_set_arg_count (block, st_node_length (node->arguments));
    st_compiled_code_set_temp_count (block, st_node_length (node->temporaries));
    st_compiled_code_set_stack_depth (block, gt->stack_depth);

    g_free (gt->code);
    g_list_free (gt->literals);

    /* restore previous state */ 
    gt->literals = literals;
    gt->code = code;
    gt->size = size;
    gt->alloc = alloc;
    gt->stack_depth = stack_depth;

    if (nested == false)
	gt->in_block = false;

    gt->literals = g_list_append (gt->literals, (void *) block);
    
    return g_list_length (gt->literals) - 1;
}


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
	    push (gt, SEND_PLUS + i);  
		return;
	}
    }

    /* send type */
    if (super_send)
	push (gt, SEND_SUPER);
    else
	push (gt, SEND);

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
	    push (gt, PUSH_SELF);
	    gt->stack_depth++;
	    break;
	} else if (st_object_equal (node->name, st_string_new ("true"))) {
	    push (gt, PUSH_TRUE);
	    gt->stack_depth++;
	    break;
	} else if (st_object_equal (node->name, st_string_new ("false"))) {
	    push (gt, PUSH_FALSE);
	    gt->stack_depth++;
	    break;
	} else if (st_object_equal (node->name, st_string_new ("nil"))) {
	    push (gt, PUSH_NIL);
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
	    push_instance_var (gt, index);
	    break;
	}
	index = find_literal_var (gt, node->name);
	if (index >= 0) {
	    push_literal_var (gt, index);
	    break;
	}
	generation_error ("unknown variable", node);
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
	generate_assign (gt, node, false);
	break;
    }
    case ST_BLOCK_NODE:
    {	
	int index;
	index = generate_block (gt, node);
	create_block (gt, index);
	break;
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
    if (gt->in_block && statements == NULL) {
	push (gt, PUSH_NIL);
    }

    for (STNode *node = statements; node; node = node->next) {

	switch (node->type) {

	case ST_VARIABLE_NODE:
	case ST_LITERAL_NODE:
	case ST_BLOCK_NODE:
	    /*
	     * don't generate anything since we would end up with a constant
	     * expression with no side-effects.
             * 
             * However, in a block with no explicit return, the value of the last statement is the implicit
             * value of the block.   
	     */
	    if (gt->in_block && node->next == NULL)
		generate_expression (gt, node);

	    break;

	case ST_ASSIGN_NODE:

	    if (gt->in_block && node->next == NULL)
		generate_assign (gt, node, false);
	    else
		generate_assign (gt, node, true);		

	    break;
	    
	case ST_RETURN_NODE:
	    
	    generate_return (gt, node);
	    
	    g_assert (node->next == NULL);

	    /* break out and don't pop stack top */
	    return;
	    
	case ST_MESSAGE_NODE:
	    
	    generate_message (gt, node);
	    
	    if (!gt->in_block || !(node->next == NULL))
		push (gt, POP_STACK_TOP);
	    break;
	    
	default:
	    g_assert_not_reached ();
	}
    }

    /* default return bytecodes */
    if (!gt->in_block) {
	gt->stack_depth++;
	push (gt, PUSH_SELF);
	push (gt, RETURN_STACK_TOP);
    } else {
	gt->stack_depth++;
	push (gt, BLOCK_RETURN);
    }
}

st_oop
st_generate_method (st_oop klass, STNode *node)
{
    Generator *gt;
    st_oop     method;

    g_assert (klass != st_nil);
    g_assert (node != NULL && node->type == ST_METHOD_NODE);
    
    init_specials ();

    gt = generator_new (klass, node);

    // generate bytecode
    generate_statements (gt, node->statements);

    method = st_object_new (st_compiled_method_class);

    st_compiled_code_set_arg_count   (method, st_node_length (node->arguments));
    st_compiled_code_set_temp_count  (method, g_list_length (gt->temporaries) - st_node_length (node->arguments));
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
    guchar *ip = codes;

    while (*ip) {

	printf ("%3li ", ip - codes);

	switch (*ip) {
	    
	case PUSH_TEMP:
	    printf ("pushTemp: %i", *(++ip));
	    
	    ip++;
	    break;
	    
	case PUSH_INSTVAR:
	    printf ("pushInstvar: %i", *(++ip));

	    ip++;
	    break;
	    
	case PUSH_LITERAL_CONST:
	    printf ("pushConst: %i", *(++ip));

	    ip++;
	    break;

	case PUSH_LITERAL_VAR:
	    printf ("pushLit: %i", *(++ip));

	    ip++;
	    break;

	case PUSH_SELF:
	    printf ("push: self");

	    ip++;
	    break;

	case PUSH_TRUE:
	    printf ("pushConst: true");

	    ip++;
	    break;
	    
	case PUSH_FALSE:
	    printf ("pushConst: false");

	    ip++;
	    break;
	   
	case PUSH_NIL:
	    printf ("pushConst: nil");

	    ip++;
	    break; 

	case STORE_TEMP:
	    printf ("storeTemp: %i", *(++ip));

	    ip++;
	    break;

	case STORE_INSTVAR:
	    printf ("storeInstvar: %i", *(++ip));

	    ip++;
	    break;

	case STORE_LITERAL_VAR:
	    printf ("storeLiteral: %i", *(++ip));

	    ip++;
	    break;

	case STORE_POP_TEMP:
	    printf ("popIntoTemp: %i", *(++ip));

	    ip++;
	    break;

	case STORE_POP_INSTVAR:
	    printf ("popIntoInstvar: %i", *(++ip));

	    ip++;
	    break;

	case STORE_POP_LITERAL_VAR:
	    printf ("popIntoLiteral: %i", *(++ip));

	    ip++;
	    break;

	case CREATE_BLOCK:
	    printf ("createBlock: %i", *(++ip));

	    ip++;
	    break;

	case POP_STACK_TOP:
	    printf ("pop");

	    ip++;
	    break;

	case RETURN_STACK_TOP:
	    printf ("returnTop");

	    ip++;
	    break;

	case BLOCK_RETURN:
	    printf ("blockReturn");

	    ip++;
	    break;

	case SEND:
	{
	    st_oop selector;
	    ip += 2;

	    selector = st_array_at (literals, *ip + 1);
	    printf ("send: #%s", (char *) st_byte_array_bytes (selector));

	    ip++;
	    break;
	}
	case SEND_SUPER:
	{
	    st_oop selector;
	    ip += 2;

	    selector = st_array_at (literals, *ip + 1);
	    printf ("sendSuper: #%s", (char *) st_byte_array_bytes (selector));

	    ip++;
	    break;
	}

	case SEND_PLUS:
	case SEND_MINUS:
	case SEND_LT:
	case SEND_GT:
	case SEND_LE:
	case SEND_GE:
	case SEND_EQ:
	case SEND_NE:
	case SEND_MUL:
	case SEND_DIV:
	case SEND_MOD:
	case SEND_BITSHIFT:
	case SEND_BITAND:
	case SEND_BITOR:
	case SEND_BITXOR:
	case SEND_AT:
	case SEND_AT_PUT:
	case SEND_SIZE:
	case SEND_VALUE:
	case SEND_VALUE_ARG:
	case SEND_IDENTITY_EQ:
	case SEND_CLASS:
	case SEND_NEW:
	case SEND_NEW_ARG:

	    printf ("sendSpecial: #%s", st_byte_array_bytes (specials[*ip - SEND_PLUS]));

	    ip++;
	    break;


	default:

	g_assert_not_reached ();
	}
	
	printf ("\n");
    }
    
}

void st_generator_print_method (st_oop method);
static void print_block (st_oop block);

static void
print_literals (st_oop literals)
{
    printf ("literals: ");

    for (int i = 1; i <= st_smi_value (st_array_size (literals)); i++) {
	
	st_oop lit = st_array_at (literals, i);

	if (st_object_is_smi (lit)) {
	    
	    printf ("%li", st_smi_value (lit));
	    
	} else if (st_object_is_symbol (lit)) {

	    printf ("#%s", (char *) st_byte_array_bytes (lit));
 
	} else if (st_object_class (lit) == st_string_class) {

	    printf ("'%s'", (char *) st_byte_array_bytes (lit));

	} else if (st_object_class (lit) == st_character_class) {

	    char outbuf[6] = { 0 };
	    g_unichar_to_utf8 (st_character_value (lit), outbuf);
	    printf ("$%s", outbuf);
	}

	printf (" ");
    }
    
    printf ("\nblock literals:\n");
    
    for (int i = 1; i <= st_smi_value (st_array_size (literals)); i++) {
		
	st_oop lit = st_array_at (literals, i);
	
	if (st_object_is_compiled_block (lit)) {
	   
	    printf ("[\n");
	    print_block (lit);
	    printf ("\n]\n");
	    
	}
    }
}

static void
print_block (st_oop block)
{
    st_oop  literals;
    guchar *bytecodes;
    int     size;

    literals = st_compiled_code_literals (block);
    bytecodes = (guchar *) st_byte_array_bytes (st_compiled_code_bytecodes (block));
    size = st_byte_array_size (st_compiled_code_bytecodes (block));

    print_bytecodes (literals, bytecodes, size);
    
    print_literals (literals);
}

void
st_generator_print_method (st_oop method)
{
    st_oop  literals;
    guchar *bytecodes;
    int     size;

    printf ("flags: %i; ", st_compiled_code_flags (method));
    printf ("arg-count: %i; ", st_compiled_code_arg_count (method));
    printf ("temp-count: %i; ", st_compiled_code_temp_count (method));
    printf ("stack-depth: %i; ", st_compiled_code_stack_depth (method));
    printf ("primitive: %i;\n", st_compiled_code_primitive_index (method));
    
    printf ("\n");

    literals = st_compiled_code_literals (method);
    bytecodes = (guchar *) st_byte_array_bytes (st_compiled_code_bytecodes (method));
    size = st_byte_array_size (st_compiled_code_bytecodes (method));

    print_bytecodes (literals, bytecodes, size);
    
    print_literals (literals);
}
