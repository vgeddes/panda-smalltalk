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

#include "st-compiler.h"

#include "st-types.h"
#include "st-object.h"
#include "st-symbol.h"
#include "st-hashed-collection.h"
#include "st-compiled-method.h"
#include "st-byte-array.h"
#include "st-universe.h"
#include "st-class.h"

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#define DEFAULT_CODE_SIZE 50
	
#define CSTR(string) ((char *) st_byte_array_bytes (string))



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

    jmp_buf  jmploc;
    STError **error;

    guint    max_stack_depth;
 
    /* names of temporaries, in order of appearance */
    GList   *temporaries;
    /* names of instvars, in order they were defined */
    GList   *instvars;
    /* literal frame for the compiled code */
    GList   *literals;

    guchar  *code;
    guint    size;
    guint    alloc;
    
} Generator;

static st_oop specials[NUM_SPECIALS] = { 0 };

static guint sizes[255] = {  0, };

// setup global data for compiler
static void
check_init (void)
{
    static bool initialized = false;

    if (initialized)
	return;
    initialized = true;

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


    /* The size (in bytes) of each bytecode instruction
     */
    sizes[PUSH_TEMP]             = 2;
    sizes[PUSH_INSTVAR]          = 2;
    sizes[PUSH_LITERAL_CONST]    = 2;
    sizes[PUSH_LITERAL_VAR]      = 2;
    sizes[PUSH_SELF]             = 1;
    sizes[PUSH_NIL]              = 1;
    sizes[PUSH_TRUE]             = 1;
    sizes[PUSH_FALSE]            = 1;
    sizes[STORE_LITERAL_VAR]     = 2;
    sizes[STORE_TEMP]            = 2;
    sizes[STORE_INSTVAR]         = 2; 
    sizes[STORE_POP_LITERAL_VAR] = 2;
    sizes[STORE_POP_TEMP]        = 2;
    sizes[STORE_POP_INSTVAR]     = 2;
    sizes[RETURN_STACK_TOP]      = 1;
    sizes[BLOCK_RETURN]          = 1;
    sizes[POP_STACK_TOP]         = 1;
    sizes[PUSH_ACTIVE_CONTEXT]   = 1;
    sizes[BLOCK_COPY]       = 2;
    sizes[JUMP_TRUE]        = 3;
    sizes[JUMP_FALSE]       = 3;
    sizes[JUMP]             = 3;
    sizes[SEND]             = 3;    
    sizes[SEND_SUPER]       = 3;
    sizes[SEND_PLUS]        = 1;
    sizes[SEND_MINUS]       = 1;
    sizes[SEND_LT]          = 1;
    sizes[SEND_GT]          = 1;
    sizes[SEND_LE]          = 1;
    sizes[SEND_GE]          = 1;
    sizes[SEND_EQ]          = 1;
    sizes[SEND_NE]          = 1;
    sizes[SEND_MUL]         = 1;
    sizes[SEND_DIV]         = 1;
    sizes[SEND_MOD]         = 1;
    sizes[SEND_BITSHIFT]    = 1;
    sizes[SEND_BITAND]      = 1;
    sizes[SEND_BITOR]       = 1;
    sizes[SEND_BITXOR]      = 1;
    sizes[SEND_AT]          = 1; 
    sizes[SEND_AT_PUT]      = 1;
    sizes[SEND_SIZE]        = 1;
    sizes[SEND_VALUE]       = 1;
    sizes[SEND_VALUE_ARG]   = 1;
    sizes[SEND_IDENTITY_EQ] = 1;
    sizes[SEND_CLASS]       = 1;
    sizes[SEND_NEW]         = 1;
    sizes[SEND_NEW_ARG]     = 1;

}

static int size_message (Generator *gt, STNode *node);
static int size_expression (Generator *gt, STNode *node);
static int size_statements (Generator *gt, STNode *node, bool optimized_block);


static void generate_expression (Generator *gt, STNode *node);
static void generate_statements (Generator *gt, STNode *statements, bool optimized_block);

static void
generation_error (Generator *gt, const char *msg, STNode *node)
{
    if (gt->error) {

	st_error_set (gt->error,
		      ST_COMPILER_ERROR,
		      msg);

	st_error_set_data (*gt->error, "line", (gpointer) node->line);
    }

    longjmp (gt->jmploc, 0);
}

static GList *
get_temporaries (Generator *gt,
		 GList  *instvars,
		 STNode *arguments,
		 STNode *temporaries)
{
    GList *temps = NULL; 
        
    for (STNode *n = arguments; n; n = n->next) {
	
	for (GList *l = instvars; l; l = l->next) {
	    if (st_object_equal (n->name, (st_oop) l->data))
		generation_error (gt, "name is already defined", arguments);
	}	
	temps = g_list_prepend (temps, (gpointer) n->name);
    }

    for (STNode *n = temporaries; n; n = n->next) {
	
	for (GList *l = instvars; l; l = l->next) {
	    if (st_object_equal (n->name, (st_oop) l->data))
		generation_error (gt, "name is already defined", temporaries);
	}
	temps = g_list_prepend (temps, (gpointer) n->name);
    }

    return g_list_reverse (temps);
}

static Generator *
generator_new (void)
{
    Generator *gt;

    gt = g_slice_new0 (Generator);

    gt->klass       = 0;
    gt->instvars    = NULL;
    gt->literals    = NULL;
    gt->temporaries = NULL;

    gt->max_stack_depth = 0;

    gt->code  = g_malloc (DEFAULT_CODE_SIZE);
    gt->alloc = DEFAULT_CODE_SIZE;
    gt->size  = 0;
   
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
emit (Generator *gt, guchar code)
{
    if (++gt->size > gt->alloc) {
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
jump_offset (Generator *gt, int offset)
{
    g_assert (offset <= G_MAXINT16);
    
    emit (gt, JUMP);

    /* push high byte */
    emit (gt, (offset >> 8) & 0xFF);
    /* push low byte */
    emit (gt, offset & 0xFF);

}


static void
assign_temp (Generator *gt, int index, bool pop)
{
    g_assert (index <= 255);

    if (pop)
	emit (gt, STORE_POP_TEMP);
    else
	emit (gt, STORE_TEMP);
    emit (gt, (guchar) index);
}

static void
assign_instvar (Generator *gt, int index, bool pop)
{
    g_assert (index <= 255);

    if (pop)
	emit (gt, STORE_POP_INSTVAR);
    else
	emit (gt, STORE_INSTVAR);
    emit (gt, (guchar) index); 
}

static void
assign_literal_var (Generator *gt, int index, bool pop)
{
    g_assert (index <= 255);
    
    if (pop)
	emit (gt, STORE_POP_LITERAL_VAR);
    else
	emit (gt, STORE_LITERAL_VAR);
    emit (gt, (guchar) index); 
}

static void
push (Generator *gt, guchar code, guchar index)
{
    emit (gt, code);
    emit (gt, index);
    
    gt->max_stack_depth++;
}


static void
push_special (Generator *gt, guchar code)
{
    emit (gt, code);
    gt->max_stack_depth++;
}

static int
size_assign (Generator *gt, STNode *node)
{
    return size_expression (gt, node->expression) + 2;
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
    
    generation_error (gt, "unknown variable", node);
}

static int
size_return (Generator *gt, STNode *node)
{
    int size = 0;

    size += size_expression (gt, node->expression);

    size += sizes[RETURN_STACK_TOP];

    return size;
}

static void
generate_return (Generator *gt, STNode *node)
{
    generate_expression (gt, node->expression);

    emit (gt, RETURN_STACK_TOP);
}

static GList *
get_block_temporaries (Generator *gt, STNode *temporaries)
{
    GList *temps = NULL;
    
    for (STNode *node = temporaries; node; node = node->next) {
      
      for (GList *l = gt->instvars; l; l = l->next) {
	  if (st_object_equal (node->name, (st_oop) l->data))
	      generation_error (gt, "name is already defined", node);
	}
	for (GList *l = gt->temporaries; l; l = l->next) {
	  if (st_object_equal (node->name, (st_oop) l->data))
	    generation_error (gt, "name already used in method", node);
	}
	temps = g_list_prepend (temps, (void *) node->name);
	
    }
    
    return g_list_reverse (temps);
}


static int
size_block (Generator *gt, STNode *node)
{
    int size = 0;
    
    /* BLOCKCOPY instruction */
    size += sizes[BLOCK_COPY];

    /* JUMP instruction */
    size += sizes[JUMP];
    
    /* block statements */
    size += size_statements (gt, node->statements, false);

    return size;
}

static void
generate_block (Generator *gt, STNode *node)
{
    bool   in_block;
    int    size;

    in_block = gt->in_block;
    gt->in_block = true;

    /* add block temporaries to current known temporaries */
    gt->temporaries = g_list_concat (gt->temporaries,
				     get_block_temporaries (gt, node->arguments));
    gt->temporaries = g_list_concat (gt->temporaries,
				     get_block_temporaries (gt, node->temporaries));

    emit (gt, BLOCK_COPY);
    emit (gt, st_node_list_length (node->arguments));

    // get size of block code and then jump around that code
    size = size_statements (gt, node->statements, false);
    jump_offset (gt, size);

    generate_statements (gt, node->statements, false);

    if (in_block == false)
	gt->in_block = false;
    
    return;
} 

static bool
is_optimization_candidate (STNode *msg)
{
    const char *csel = CSTR (msg->selector);

    if (streq (csel, "ifTrue:") || streq (csel, "ifFalse:")) {
	if (msg->arguments->type == ST_BLOCK_NODE)
	    return true;
    }

    if (streq (csel, "ifTrue:ifFalse:") || streq (csel, "ifFalse:ifTrue:")) {

	if (msg->arguments->type == ST_BLOCK_NODE
	    && msg->arguments->next->type == ST_BLOCK_NODE)
	    return true;
    }

    if (streq (csel, "whileTrue") || streq (csel, "whileFalse")) {

	if (msg->receiver->type == ST_BLOCK_NODE)
	    return true;
    }

    if (streq (csel, "whileTrue:") || streq (csel, "whileFalse:")) {

	if (msg->receiver->type == ST_BLOCK_NODE
	    && msg->arguments->type == ST_BLOCK_NODE)
	    return true;
    }
    
    if (streq (csel, "and:") || streq (csel, "or:")) {
	
	if (msg->arguments->type == ST_BLOCK_NODE)
	    return true;
    }
    
    return false;
}

static int size_optimized_message (Generator *gt, STNode *msg, bool is_expr);

static int
size_optimized_message (Generator *gt, STNode *msg, bool is_expr)
{
    int size = 0;
    
    const char *csel = CSTR (msg->selector);

    if (streq (csel, "ifTrue:") || streq (csel, "ifFalse:")) {   
	
	size += size_expression (gt, msg->receiver);
	
	size += sizes[JUMP_TRUE];

	size += size_statements (gt, msg->arguments->statements, true);
	
	if (is_expr) {
	    size += sizes[JUMP];
	    size += sizes[PUSH_NIL];
	} else {
	    size += sizes[POP_STACK_TOP];
	}
	
    }  else if (streq (csel, "ifTrue:ifFalse:") || streq (csel, "ifFalse:ifTrue:")) {
	
	size += size_expression (gt, msg->receiver);
	
	size += sizes[JUMP_TRUE];
	
	// true block
	size += size_statements (gt, msg->arguments->statements, true);
	
	size += sizes[JUMP];
	
	size += size_statements (gt, msg->arguments->next->statements, true);
	
	if (!is_expr)
	   size += sizes[POP_STACK_TOP];
	
    } else if (streq (csel, "whileTrue") || streq (csel, "whileFalse")) {
	
	size += size_statements (gt, msg->receiver->statements, true);

	size += sizes[JUMP_TRUE];
	
	size += sizes[JUMP];

	if (is_expr)
	    size += sizes[PUSH_NIL];
	
    } else if (streq (csel, "whileTrue:") || streq (csel, "whileFalse:")) {

	size += size_statements (gt, msg->receiver->statements, true);
	
	size += sizes[JUMP_TRUE];
	
	size += size_statements (gt, msg->arguments->statements, true);

	size += sizes[JUMP];
	
	if (is_expr)
	    size += sizes[PUSH_NIL];
	
    } else if (streq (csel, "and:") || streq (csel, "or:")) {
	
	size += size_expression (gt, msg->receiver);
	
	size += sizes[JUMP_TRUE];
      
	size += size_statements (gt, msg->arguments->statements, true);
	
	size += sizes[JUMP];
	
	size += sizes[PUSH_TRUE];
	
	if (!is_expr)
	    size += sizes[POP_STACK_TOP];

    }
    
    return size;
}


static void
generate_optimized_message (Generator *gt, STNode *msg, bool is_expr)
{
    STNode *block;
    const char *csel;
    int size;

    csel = CSTR (msg->selector);

    if (streq (csel, "ifTrue:") || streq (csel, "ifFalse:")) {
	
	block = msg->arguments;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL)
	    generation_error (gt, "argument of ifTrue: message must be a 0-argument block", block);  
	
	generate_expression (gt, msg->receiver);
	
	size = size_statements (gt, block->statements, true);
	
	if (is_expr)
	    size += 3;
	else
	    size += 1;

	if (streq (csel, "ifTrue:"))
	    emit (gt, JUMP_FALSE);
	else
	    emit (gt, JUMP_TRUE);
	emit (gt, (size >> 8) & 0xFF);
	emit (gt, size & 0xFF);
	
	generate_statements (gt, block->statements, true);
	
	if (is_expr) {
	    emit (gt, JUMP);
	    emit (gt, 0);
	    emit (gt, 1);
	    emit (gt, PUSH_NIL);
	} else {
	    emit (gt, POP_STACK_TOP);
	}

	return;
	
    } else if (streq (csel, "ifTrue:ifFalse:") || streq (csel, "ifFalse:ifTrue")) {
	
	STNode *true_block, *false_block;
	
	true_block = msg->arguments;
	if (true_block->type != ST_BLOCK_NODE || true_block->arguments != NULL)
	    generation_error (gt, "first argument of ifTrue:ifFalse message must be a 0-argument block", true_block);
	false_block = msg->arguments->next;
	if (false_block->type != ST_BLOCK_NODE || false_block->arguments != NULL)
	    generation_error (gt, "second argument of ifTrue:ifFalse message must be a 0-argument block", false_block);

	generate_expression (gt, msg->receiver);

	// add 3 for size of jump instr at end of true block
	size = size_statements (gt, true_block->statements, true);
	size += 3; 
	
	if (streq (csel, "ifTrue:ifFalse:"))
	    emit (gt, JUMP_FALSE);
	else
	    emit (gt, JUMP_TRUE);
	emit (gt, (size >> 8) & 0xFF);
	emit (gt, size & 0xFF);

	generate_statements (gt, true_block->statements, true);

	size = size_statements (gt, false_block->statements, true);
      	emit (gt, JUMP);
	emit (gt, (size >> 8) & 0xFF);
	emit (gt, size & 0xFF);

	generate_statements (gt, false_block->statements, true);

	/* if this is a statement we pop the last value on the stack */
	if (!is_expr)
	    emit (gt, POP_STACK_TOP);

	return;
    }

    if (streq (csel, "whileTrue") || streq (csel, "whileFalse")) {

	block = msg->receiver;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL) {
	    if (streq (csel, "whileTrue"))
		generation_error (gt, "receiver of whileTrue message must be a 0-argument block", block);
	    else
		generation_error (gt, "receiver of whileFalse message must be a 0-argument block", block);
	}

	generate_statements (gt, block->statements, true);
	
	// jump around jump statement
	if (streq (csel, "whileTrue"))
	    emit (gt, JUMP_FALSE);
	else
	    emit (gt, JUMP_TRUE);
	emit (gt, 0);
	emit (gt, 3);

	size = size_statements (gt, block->statements, true);
	// size of JUMP_FALSE instr
	size += 3;
	// we jump backwards
	size = - size;
	
	emit (gt, JUMP);
	emit (gt, (size >> 8) & 0xFF);
	emit (gt, size & 0xFF);
	
	if (is_expr)
	    emit (gt, PUSH_NIL);

	return;
    }
    
    if (streq (csel, "whileTrue:") || streq (csel, "whileFalse:")) {

	block = msg->receiver;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL) {
	    if (streq (csel, "whileTrue:"))
		generation_error (gt, "receiver of whileTrue: message must be a 0-argument block", block);
	    else
		generation_error (gt, "receiver of whileFalse: message must be a 0-argument block", block);
	}

	block = msg->arguments;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL) {
	    if (streq (csel, "whileTrue:"))
		generation_error (gt, "argument of whileTrue: message must be a 0-argument block", block);
	    else
		generation_error (gt, "argument of whileFalse: message must be a 0-argument block", block);

	}
	
	generate_statements (gt, msg->receiver->statements, true);
	
	if (streq (csel, "whileTrue:"))
	    emit (gt, JUMP_FALSE);
	else
	    emit (gt, JUMP_TRUE);

	size = size_statements (gt, msg->arguments->statements, true);
	// include size of POP and JUMP statement
	size += 1 + 3;

	emit (gt, (size >> 8) & 0xFF);
	emit (gt, size & 0xFF);

	generate_statements (gt, msg->arguments->statements, true);
	
	size += size_statements (gt, msg->receiver->statements, true);
	size = - size;

	emit (gt, POP_STACK_TOP);

	emit (gt, JUMP);
	emit (gt, (size >> 8) & 0xFF);
	emit (gt, size & 0xFF);

	if (is_expr)
	    emit (gt, PUSH_NIL);

	return;
    }

    if (streq (csel, "and:") || streq (csel, "or:")) {

	block = msg->arguments;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL) {
	    if (streq (csel, "and:"))
		generation_error (gt, "argument of and: message must be a 0-argument block", block);
	    else
		generation_error (gt, "argument of or: message must be a 0-argument block", block);
	}

	generate_expression (gt, msg->receiver);

	size = size_statements (gt, block->statements, true);
	// size of JUMP instr after block statements
	size += 3;

	if (streq (csel, "and:"))
	    emit (gt, JUMP_FALSE);
	else
	    emit (gt, JUMP_TRUE);
	emit (gt, (size >> 8) & 0xFF);
	emit (gt, size & 0xFF);
	
	generate_statements (gt, block->statements, true);

	emit (gt, JUMP);
	emit (gt, 0);
	emit (gt, 1);

	if (streq (csel, "and:"))
	    emit (gt, PUSH_FALSE);
	else
	    emit (gt, PUSH_TRUE);

	if (!is_expr)
	    emit (gt, POP_STACK_TOP);

	return;
    }

    g_assert_not_reached ();
}

static int
size_message (Generator *gt, STNode *msg)
{
    bool    super_send = false;
    STNode *args;
    int size = 0;

    size += size_expression (gt, msg->receiver);

    args = msg->arguments;
    for (; args; args = args->next)
	size += size_expression (gt, args);

    /* check if receiver is pseudo-variable 'super' */
    if (msg->receiver->type == ST_VARIABLE_NODE
	&& st_object_equal (msg->receiver->name, st_string_new ("super")))
	super_send = true;

    /* check if message is a special */
    for (int i = 0; i < G_N_ELEMENTS (specials); i++) {
	if (!super_send && msg->selector == specials[i]) {
	    size += 1;
	    return size;
	}
    }

    g_assert (sizes[SEND] == sizes[SEND_SUPER]);
    size += sizes[SEND];
    
    return size;
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
	    emit (gt, SEND_PLUS + i);  
	    return;
	}
    }

    /* send type */
    if (super_send)
	emit (gt, SEND_SUPER);
    else
	emit (gt, SEND);

    /* argument count */
    emit (gt, (guchar) argcount);
	
    /* index of selector in literal frame */
    int index = find_literal_const (gt, msg->selector);
    emit (gt, (guchar) index);
}


static int
size_expression (Generator *gt, STNode *node)
{
    int size = 0;

    switch (node->type) {
    case ST_VARIABLE_NODE:
    {
	const char *name = CSTR (node->name);

	if (streq (name, "self") || streq (name, "super")) {
	    size += sizes[PUSH_SELF];
	    break;
	} else if (streq (name, "true")) {
	    size += sizes[PUSH_TRUE];
	    break;
	} else if (streq (name, "false")) {
	    size += sizes[PUSH_FALSE];
	    break;
	} else if (streq (name, "nil")) {
	    size += sizes[PUSH_NIL];
	    break;
	} else if (streq (name, "thisContext")) {
	    size += sizes[PUSH_ACTIVE_CONTEXT];
	    break;
	}

	int index;	
	index = find_temporary (gt, node->name);
	if (index >= 0) {
	    size += 2;
	    break;
	}
	index = find_instvar (gt, node->name);
	if (index >= 0) {
	    size += 2;
	    break;
	}
	index = find_literal_var (gt, node->name);
	if (index >= 0) {
	    size += 2;
	    break;
	}
	generation_error (gt, "unknown variable", node);
    }
    case ST_LITERAL_NODE:
	size += sizes[PUSH_LITERAL_CONST];
	break;

    case ST_ASSIGN_NODE:
	size += size_assign (gt, node);
	break;

    case ST_BLOCK_NODE:	
	size += size_block (gt, node);
	break;

    case ST_MESSAGE_NODE:

	if (is_optimization_candidate (node))
	    size += size_optimized_message (gt, node, true);
	else
	    size += size_message (gt, node);
	break;

    default:
	g_assert_not_reached ();
    }

    return size;
}

static void
generate_expression (Generator *gt, STNode *node)
{   
    int index;

    switch (node->type) {
    case ST_VARIABLE_NODE:
    {
	const char *name = CSTR (node->name);

	if (streq (name, "self") || streq (name, "super")) {
	    push_special (gt, PUSH_SELF);
	    break;
	} else if (streq (name, "true")) {
	    push_special (gt, PUSH_TRUE);
	    break;
	} else if (streq (name, "false")) {
	    push_special (gt, PUSH_FALSE);
	    break;
	} else if (streq (name, "nil")) {
	    push_special (gt, PUSH_NIL);
	    break;
	} else if (streq (name, "thisContext")) {
	    push_special (gt, PUSH_ACTIVE_CONTEXT);
	    break;
	}

	index = find_temporary (gt, node->name);
	if (index >= 0) {
	    push (gt, PUSH_TEMP, index);
	    break;
	}
	index = find_instvar (gt, node->name);
	if (index >= 0) {
	    push (gt, PUSH_INSTVAR, index);
	    break;
	}
	index = find_literal_var (gt, node->name);
	if (index >= 0) {
	    push (gt, PUSH_LITERAL_VAR, index);
	    break;
	}
	generation_error (gt, "unknown variable", node);

    }
    case ST_LITERAL_NODE:
	index = find_literal_const (gt, node->literal);
	push (gt, PUSH_LITERAL_CONST, index);
	break;

    case ST_ASSIGN_NODE:
	generate_assign (gt, node, false);
	break;

    case ST_BLOCK_NODE:	
	generate_block (gt, node);
	break;

    case ST_MESSAGE_NODE:

	if (is_optimization_candidate (node))
	    generate_optimized_message (gt, node, true);
	else
	    generate_message (gt, node);

	break;

    default:
	g_assert_not_reached ();
    }
}

static int
size_statements (Generator *gt, STNode *statements, bool optimized_block)
{
    int size = 0;

    if (statements == NULL) {
	size += sizes[PUSH_NIL];
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
	    if (node->next == NULL)
		size += size_expression (gt, node);

	    break;

	case ST_ASSIGN_NODE:
	    
	    size += size_assign (gt, node);

	    break;
	    
	case ST_RETURN_NODE:
	    
	    size += size_return (gt, node);
	    g_assert (node->next == NULL);

	    /* break out and don't pop stack top */
	    return size;
	    
	case ST_MESSAGE_NODE:

	    if (node->next != NULL) {
		if (is_optimization_candidate (node)) {
		    size += size_optimized_message (gt, node, false);
		} else {
		    size += size_message (gt, node);
		    size += sizes[POP_STACK_TOP];
		}
	    } else {
		if (is_optimization_candidate (node)) {
		    size += size_optimized_message (gt, node, true);
		} else {
		    size += size_message (gt, node);
		}
	    }

	    break;
	    
	default:
	    g_assert_not_reached ();
	}
    }

    if (!optimized_block)
	size += sizes[BLOCK_RETURN];

    return size;
}

static void
generate_statements (Generator *gt, STNode *statements, bool optimized_block)
{
    if (statements == NULL) {
	emit (gt, PUSH_NIL);
	return;
    }

    for (STNode *node = statements; node; node = node->next) {

	switch (node->type) {

	case ST_VARIABLE_NODE:
	case ST_LITERAL_NODE:
	case ST_BLOCK_NODE:
	    /*
	     * don't generate anything in this case since we would end up with a constant
	     * expression with no side-effects.
             * 
             * However, in a block with no explicit return, the value of the last statement is the implicit
             * value of the block.   
	     */
	    if (node->next == NULL)
		generate_expression (gt, node);
	    break;

	case ST_ASSIGN_NODE:

	    /* don't use STORE_POP if this is the last statement */
	    if (node->next == NULL)
		generate_assign (gt, node, false);
	    else
		generate_assign (gt, node, true);		
	    break;
	    
	case ST_RETURN_NODE:
	    
	    /* break out and don't pop stack top */
	    generate_return (gt, node);
	    g_assert (node->next == NULL);
	    return;
	    
	case ST_MESSAGE_NODE:

	    /* if the message is the last statement in a block then we must
             * not pop the message result as it is the value of the block
	     */
	    if (node->next != NULL) {
		if (is_optimization_candidate (node)) {
		    generate_optimized_message (gt, node, false);
		} else {
		    generate_message (gt, node);
		    emit (gt, POP_STACK_TOP);
		}
	    } else {
		if (is_optimization_candidate (node)) {
		    generate_optimized_message (gt, node, true);
		} else {
		    generate_message (gt, node);
		}
	    }
	    break;
	    
	default:
	    g_assert_not_reached ();
	}
    }

    if (!optimized_block)
	emit (gt, BLOCK_RETURN);
}


static void
generate_method_statements (Generator *gt, STNode *statements)
{
   for (STNode *node = statements; node; node = node->next) {

       switch (node->type) {
	   
       case ST_VARIABLE_NODE:
       case ST_LITERAL_NODE:
       case ST_BLOCK_NODE:
	    break;
	    
       case ST_ASSIGN_NODE:
	   
	    generate_assign (gt, node, true);		
	    break;
	    
	case ST_RETURN_NODE:
	    
	    /* break out and don't pop stack top */	    
	    generate_return (gt, node);
	    g_assert (node->next == NULL);
	    return;
	    
	case ST_MESSAGE_NODE:

	    if (is_optimization_candidate (node))
		generate_optimized_message (gt, node, false);
	    else {
		generate_message (gt, node);
		emit (gt, POP_STACK_TOP);
	    }
	    break;
	    
       default:
	   g_assert_not_reached ();
       }
    }
   
   emit (gt, PUSH_SELF);
   emit (gt, RETURN_STACK_TOP);
}

GQuark
st_compilation_error_quark (void)
{
  return g_quark_from_static_string ("st-compilation-error-quark");
}

st_oop
st_generate_method (st_oop klass, STNode *node, STError **error)
{
    Generator *gt;
    st_oop     method;

    g_assert (klass != st_nil);
    g_assert (node != NULL && node->type == ST_METHOD_NODE);
    
    check_init ();

    gt = generator_new ();
    gt->error = error;

    if (setjmp (gt->jmploc))
	goto error;

    gt->klass = klass;
    gt->instvars = st_behavior_all_instance_variables (klass);
    gt->temporaries = get_temporaries (gt, gt->instvars, node->arguments, node->temporaries);

    // generate bytecode
    generate_method_statements (gt, node->statements);

    method = st_object_new (st_compiled_method_class);

    st_compiled_method_set_arg_count   (method, st_node_list_length (node->arguments));
    st_compiled_method_set_temp_count  (method, g_list_length (gt->temporaries) - st_node_list_length (node->arguments));
    st_compiled_method_set_stack_depth (method, gt->max_stack_depth);

    if (node->primitive < 0xFF) {
	st_compiled_method_set_flags (method, 1);	
	st_compiled_method_set_primitive_index (method, node->primitive);
    } else {
	st_compiled_method_set_primitive_index (method, 0xFF);
	st_compiled_method_set_flags (method, 0);	
    }

    st_compiled_method_set_literals (method, create_literals_array (gt));
    st_compiled_method_set_bytecodes (method, create_bytecode_array (gt)); 

    generator_destroy (gt);

    return method;
    
 error:

    generator_destroy (gt);    

    return st_nil;
}

#define NEXT(ip)      \
    ip += sizes[*ip]; \
    break

#define FORMAT(ip) (formats[sizes[*ip]-1])

static void
print_bytecodes (st_oop literals, guchar *codes, int len) 
{
    static const char * const formats[] = {
	"<%02x>       ",
	"<%02x %02x>    ",
	"<%02x %02x %02x> ",
    };

    guchar *ip = codes;

    while (*ip) {

	printf ("%3li ", ip - codes);

	switch ((Code) *ip) {
	    
	case PUSH_TEMP:
	    printf (FORMAT (ip), ip[0], ip[1]); 
	    printf ("pushTemp: %i", ip[1]);
	    
	    NEXT (ip);
	    
	case PUSH_INSTVAR:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("pushInstvar: %i", ip[1]);

	    NEXT (ip);
	    
	case PUSH_LITERAL_CONST:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("pushConst: %i", ip[1]);

	    NEXT (ip);

	case PUSH_LITERAL_VAR:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("pushLit: %i", ip[1]);

	    NEXT (ip);

	case PUSH_SELF:
	    printf (FORMAT (ip), ip[0]);
	    printf ("push: self");

	    NEXT (ip);

	case PUSH_TRUE:
	    printf (FORMAT (ip), ip[0]);
	    printf ("pushConst: true");

	    NEXT (ip);
	    
	case PUSH_FALSE:
	    printf (FORMAT (ip), ip[0]);
	    printf ("pushConst: false");

	    NEXT (ip);

	case PUSH_NIL:
	    printf (FORMAT (ip), ip[0]);
	    printf ("pushConst: nil");

	    NEXT (ip);

	case PUSH_ACTIVE_CONTEXT:
	    printf (FORMAT (ip), ip[0]);
	    printf ("push: thisContext");

	    NEXT (ip);

	case STORE_TEMP:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("storeTemp: %i", ip[1]);

	    NEXT (ip);

	case STORE_INSTVAR:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("storeInstvar: %i", ip[1]);

	    NEXT (ip);

	case STORE_LITERAL_VAR:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("storeLiteral: %i", ip[1]);

	    NEXT (ip);

	case STORE_POP_TEMP:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("popIntoTemp: %i", ip[1]);

	    NEXT (ip);

	case STORE_POP_INSTVAR:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("popIntoInstvar: %i", ip[1]);

	    NEXT (ip);

	case STORE_POP_LITERAL_VAR:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("popIntoLiteral: %i", ip[1]);

	    NEXT (ip);

	case BLOCK_COPY:
	    printf (FORMAT (ip), ip[0], ip[1]);
	    printf ("blockCopy: %i", ip[1]);

	    NEXT (ip);

	case JUMP:
	    printf (FORMAT (ip), ip[0], ip[1], ip[2]);

	    short offset = ((ip[1] << 8) | ip[2]);
	    printf ("jump: %li", (offset >= 0 ? 3 : 0) + (ip - codes) + offset);

	    NEXT (ip);

	case JUMP_TRUE:
	    printf (FORMAT (ip), ip[0], ip[1], ip[2]);
	    printf ("jumpTrue: %li", 3 + (ip - codes) + ((ip[1] << 8) | ip[2]));

	    NEXT (ip);

	case JUMP_FALSE:
	    printf (FORMAT (ip), ip[0], ip[1], ip[2]);
	    printf ("jumpFalse: %li", 3 + (ip - codes) + ((ip[1] << 8) | ip[2]));

	    NEXT (ip);

	case POP_STACK_TOP:
	    printf (FORMAT (ip), ip[0]);
	    printf ("pop");

	    NEXT (ip);

	case RETURN_STACK_TOP:
	    printf (FORMAT (ip), ip[0]);
	    printf ("returnTop");

	    ip += 1;
	    break;

	    NEXT (ip);

	case BLOCK_RETURN:
	    printf (FORMAT (ip), ip[0]);
	    printf ("blockReturn");

	    NEXT (ip);

	case SEND:
	{
	    st_oop selector;

	    selector = st_array_at (literals, ip[2] + 1);

	    printf (FORMAT (ip), ip[0], ip[1], ip[2]);

	    printf ("send: #%s", (char *) st_byte_array_bytes (selector));

	    NEXT (ip);
	}
	case SEND_SUPER:
	{
	    st_oop selector;

	    selector = st_array_at (literals, ip[2] + 1);

	    printf (FORMAT (ip), ip[0], ip[1], ip[2]);

	    printf ("sendSuper: #%s", (char *) st_byte_array_bytes (selector));

	    NEXT (ip);
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

	    printf (FORMAT (ip), ip[0]);
	    printf ("sendSpecial: #%s", st_byte_array_bytes (specials[ip[0] - SEND_PLUS]));

	    NEXT (ip);

	}
	printf ("\n");
    }
    
}

static void
print_literal (st_oop lit)
{
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
}


static void
print_literals (st_oop literals)
{
    if (literals == st_nil)
	return;

    printf ("literals: ");

    for (int i = 1; i <= st_smi_value (st_array_size (literals)); i++) {
	
	st_oop lit = st_array_at (literals, i);

	print_literal (lit);

	printf (" ");
    }
    
    printf ("\n");
}

void
st_print_method (st_oop method)
{
    st_oop  literals;
    guchar *bytecodes;
    int     size;

    printf ("flags: %i; ", st_compiled_method_flags (method));
    printf ("arg-count: %i; ", st_compiled_method_arg_count (method));
    printf ("temp-count: %i; ", st_compiled_method_temp_count (method));
    printf ("stack-depth: %i; ", st_compiled_method_stack_depth (method));
    printf ("primitive: %i;\n", st_compiled_method_primitive_index (method));
    
    printf ("\n");

    literals = st_compiled_method_literals (method);
    bytecodes = (guchar *) st_byte_array_bytes (st_compiled_method_bytecodes (method));
    size = st_byte_array_size (st_compiled_method_bytecodes (method));

    print_bytecodes (literals, bytecodes, size);
    
    print_literals (literals);
}

