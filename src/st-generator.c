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
#include "st-dictionary.h"
#include "st-method.h"
#include "st-array.h"
#include "st-array.h"
#include "st-universe.h"
#include "st-behavior.h"
#include "st-character.h"
#include "st-unicode.h"

#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#define DEFAULT_CODE_SIZE 50

#define CSTRING(string) ((char *) st_byte_array_bytes (string))

typedef struct 
{
    bool     in_block;

    st_oop   class;

    jmp_buf  jmploc;

    st_compiler_error *error;

    st_uint    max_stack_depth;

    bool     references_super;
 
    /* names of temporaries, in order of appearance */
    st_list   *temporaries;
    /* names of instvars, in order they were defined */
    st_list   *instvars;
    /* literal frame for the compiled code */
    st_list   *literals;

    st_uchar  *code;
    st_uint    size;
    st_uint    alloc;
    
} Generator;

static st_uint sizes[255] = {  0, };


// setup global data for compiler
static void
check_init (void)
{
    static bool initialized = false;

    if (initialized)
	return;
    initialized = true;

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
    sizes[DUPLICATE_STACK_TOP]   = 1;
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

static int size_message    (Generator *gt, st_node *node);
static int size_expression (Generator *gt, st_node *node);
static int size_statements (Generator *gt, st_node *node);


static void generate_expression (Generator *gt, st_node *node);
static void generate_statements (Generator *gt, st_node *statements);

static void
generation_error (Generator *gt, const char *message, st_node *node)
{
    if (gt->error) {
	strncpy (gt->error->message, message, 255);
	gt->error->line   = node->line;
	gt->error->column = 0;
    }

    longjmp (gt->jmploc, 0);
}

static st_list *
get_temporaries (Generator *gt,
		 st_list  *instvars,
		 st_node *arguments,
		 st_node *temporaries)
{
    st_list *temps = NULL; 
        
    for (st_node *n = arguments; n; n = n->next) {
	
	for (st_list *l = instvars; l; l = l->next) {
	    if (streq (n->variable.name, (char *) l->data))
		generation_error (gt, "name is already defined", arguments);
	}	
	temps = st_list_prepend (temps, (st_pointer) n->variable.name);
    }

    for (st_node *n = temporaries; n; n = n->next) {
	
	for (st_list *l = instvars; l; l = l->next) {
	    if (streq (n->variable.name, (char *) l->data))
		generation_error (gt, "name is already defined", temporaries);
	}
	temps = st_list_prepend (temps, (st_pointer) n->variable.name);
    }

    return st_list_reverse (temps);
}

static Generator *
generator_new (void)
{
    Generator *gt;

    gt = st_new0 (Generator);

    gt->class       = 0;
    gt->instvars    = NULL;
    gt->literals    = NULL;
    gt->temporaries = NULL;

    gt->max_stack_depth = 0;
    gt->references_super = false;

    gt->code  = st_malloc (DEFAULT_CODE_SIZE);
    gt->alloc = DEFAULT_CODE_SIZE;
    gt->size  = 0;
   
    return gt;
}

static void
generator_destroy (Generator *gt)
{
    st_list_destroy (gt->instvars);
    st_list_destroy (gt->temporaries);
    st_list_destroy (gt->literals);

    st_free (gt->code);
    st_free (gt);
}

static st_oop
create_literals_array (Generator *gt)
{
    int    count;
    st_oop literals = st_nil;

    if (gt->references_super)
	gt->literals = st_list_append (gt->literals, (st_pointer) gt->class);

    count = st_list_length (gt->literals);

    if (count > 0) {
	literals = st_object_new_arrayed (st_array_class, count); 

	int i = 1;
	for (st_list *l = gt->literals; l; l = l->next) {
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
    st_uchar   *bytes;

    bytecode = st_object_new_arrayed (st_byte_array_class, gt->size);
    bytes = st_byte_array_bytes (bytecode);
    memcpy (bytes, gt->code, gt->size);

    return bytecode;
}

static void
emit (Generator *gt, st_uchar code)
{
    if (++gt->size > gt->alloc) {
	gt->alloc += gt->alloc;
	gt->code = st_realloc (gt->code, gt->alloc);
    }

    gt->code[gt->size - 1] = code;
}


static int
find_instvar (Generator *gt, char *name)
{   
    int i = 0;
    for (st_list *l = gt->instvars; l; l = l->next) {

	if (streq (name, (char *) l->data))
	    return i;
	i++;
    }
    return -1;
}

static int
find_temporary (Generator *gt, char *name)
{   
    int i = 0;
    
    for (st_list *l = gt->temporaries; l; l = l->next) {

	if (streq (name, (char *) l->data))
	    return i;
	i++;
    }
    return -1;
}

static int
find_literal_const (Generator *gt, st_oop literal)
{   
    int i = 0;
    for (st_list *l = gt->literals; l; l = l->next) {

	if (st_object_equal (literal, (st_oop) l->data))
	    return i;
	i++;
    }
    gt->literals = st_list_append (gt->literals, (void *) literal);
    return i;
}

static int
find_literal_var (Generator *gt, char *name)
{
    st_oop assoc;

    assoc = st_dictionary_association_at (st_globals, st_symbol_new (name));
    if (assoc == st_nil)
	return -1;

    int i = 0;
    for (st_list *l = gt->literals; l; l = l->next) {
	if (st_object_equal (assoc, (st_oop) l->data))
	    return i;
	i++;
    }
    gt->literals = st_list_append (gt->literals, (st_pointer) assoc);
    return i;
}

static void
jump_offset (Generator *gt, int offset)
{
    st_assert (offset <= INT16_MAX);
    
    emit (gt, JUMP);

    /* push high byte */
    emit (gt, (offset >> 8) & 0xFF);
    /* push low byte */
    emit (gt, offset & 0xFF);

}


static void
assign_temp (Generator *gt, int index, bool pop)
{
    st_assert (index <= 255);

    if (pop)
	emit (gt, STORE_POP_TEMP);
    else
	emit (gt, STORE_TEMP);
    emit (gt, (st_uchar) index);
}

static void
assign_instvar (Generator *gt, int index, bool pop)
{
    st_assert (index <= 255);

    if (pop)
	emit (gt, STORE_POP_INSTVAR);
    else
	emit (gt, STORE_INSTVAR);
    emit (gt, (st_uchar) index); 
}

static void
assign_literal_var (Generator *gt, int index, bool pop)
{
    st_assert (index <= 255);
    
    if (pop)
	emit (gt, STORE_POP_LITERAL_VAR);
    else
	emit (gt, STORE_LITERAL_VAR);
    emit (gt, (st_uchar) index); 
}

static void
push (Generator *gt, st_uchar code, st_uchar index)
{
    emit (gt, code);
    emit (gt, index);
    
    gt->max_stack_depth++;
}


static void
push_special (Generator *gt, st_uchar code)
{
    emit (gt, code);
    gt->max_stack_depth++;
}

static int
size_assign (Generator *gt, st_node *node)
{
    return size_expression (gt, node->assign.expression) + 2;
}

static void
generate_assign (Generator *gt, st_node *node, bool pop)
{
    int index;
    
    generate_expression (gt, node->assign.expression);
    
    index = find_temporary (gt, node->assign.assignee->variable.name);
    if (index >= 0) {
	assign_temp (gt, index, pop); 
	return;
    }

    index = find_instvar (gt, node->assign.assignee->variable.name);
    if (index >= 0) {
	assign_instvar (gt, index, pop);
	return;
    }
 
    index = find_literal_var (gt, node->assign.assignee->variable.name);
    if (index >= 0) {
	assign_literal_var (gt, index, pop);
	return;
    }
    
    generation_error (gt, "unknown variable", node);
}

static int
size_return (Generator *gt, st_node *node)
{
    int size = 0;

    size += size_expression (gt, node->retrn.expression);

    size += sizes[RETURN_STACK_TOP];

    return size;
}

static void
generate_return (Generator *gt, st_node *node)
{
    generate_expression (gt, node->retrn.expression);

    emit (gt, RETURN_STACK_TOP);
}

static st_list *
get_block_temporaries (Generator *gt, st_node *temporaries)
{
    st_list *temps = NULL;
    
    for (st_node *node = temporaries; node; node = node->next) {
      
	for (st_list *l = gt->instvars; l; l = l->next) {
	  if (streq (node->variable.name, (char *) l->data))
	      generation_error (gt, "name is already defined", node);
	}

	for (st_list *l = gt->temporaries; l; l = l->next) {
	  if (streq (node->variable.name, (char *) l->data))
	      generation_error (gt, "name already used in method", node);
	}

	temps = st_list_prepend (temps, (st_pointer) node->variable.name);
	
    }
    
    return st_list_reverse (temps);
}


static int
size_block (Generator *gt, st_node *node)
{
    int size = 0;
    
    size += sizes[BLOCK_COPY];
    size += sizes[JUMP];
    size += sizes[STORE_POP_TEMP] * st_node_list_length (node->block.arguments);
    size += size_statements (gt, node->block.statements);
    size += sizes[BLOCK_RETURN];

    return size;
}

static void
generate_block (Generator *gt, st_node *node)
{
    bool  in_block;
    int   index, size = 0;

    in_block = gt->in_block;
    gt->in_block = true;

    emit (gt, BLOCK_COPY);
    emit (gt, st_node_list_length (node->block.arguments));

    // get size of block code and then jump around that code
    size = 2 * st_node_list_length (node->block.arguments);
    size += size_statements (gt, node->block.statements);
    size += sizes[BLOCK_RETURN];
    jump_offset (gt, size);

    /* store all block arguments into the temporary frame */
    for (st_node *l = node->block.arguments; l; l = l->next) {
	index = find_temporary (gt, l->variable.name);
	st_assert (index >= 0);
	assign_temp (gt, index, true); 	
    }

    generate_statements (gt, node->block.statements);

    emit (gt, BLOCK_RETURN);

    if (in_block == false)
	gt->in_block = false;
} 

static void
generation_func_1 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    st_node *block;
    int     size;

    block = node->message.arguments;
    if (block->type != ST_BLOCK_NODE || block->block.arguments != NULL)
	generation_error (gt, "argument of ifTrue: message must be a 0-argument block", block);  
	
    generate_expression (gt, node->message.receiver);
	
    size = size_statements (gt, block->block.statements);
	
    if (!node->message.is_statement)
	size += sizes[JUMP];
    else
	size += sizes[POP_STACK_TOP];

    if (subpattern_index == 0)
	emit (gt, JUMP_FALSE);
    else
	emit (gt, JUMP_TRUE);
    emit (gt, (size >> 8) & 0xFF);
    emit (gt, size & 0xFF);
	
    generate_statements (gt, block->block.statements);
	
    if (!node->message.is_statement) {
	emit (gt, JUMP);
	emit (gt, 0);
	emit (gt, 1);
	emit (gt, PUSH_NIL);
    } else {
	emit (gt, POP_STACK_TOP);
    }

}

static int
size_func_1 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    st_node *block;
    int size = 0;

    block = node->message.arguments;
    if (block->type != ST_BLOCK_NODE || block->block.arguments != NULL)
	generation_error (gt, "argument of ifTrue: message must be a 0-argument block", block);


    size += size_expression (gt, node->message.receiver);
    size += sizes[JUMP_TRUE];
    size += size_statements (gt, node->message.arguments->block.statements);
    
    if (!node->message.is_statement) {
	size += sizes[JUMP];
	size += sizes[PUSH_NIL];
    } else {
	size += sizes[POP_STACK_TOP];
    }

    return size;
}


static void
generation_func_2 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    int     size;

    st_node *true_block, *false_block;
	
    true_block = node->message.arguments;
    if (true_block->type != ST_BLOCK_NODE || true_block->block.arguments != NULL)
	generation_error (gt, "first argument of ifTrue:ifFalse message must be a 0-argument block", true_block);
    false_block = node->message.arguments->next;
    if (false_block->type != ST_BLOCK_NODE || false_block->block.arguments != NULL)
	generation_error (gt, "second argument of ifTrue:ifFalse message must be a 0-argument block", false_block);

    generate_expression (gt, node->message.receiver);

    // add 3 for size of jump instr at end of true block
    size = size_statements (gt, true_block->block.statements);
    size += sizes[JUMP]; 
	
    if (subpattern_index == 0)
	emit (gt, JUMP_FALSE);
    else
	emit (gt, JUMP_TRUE);

    emit (gt, (size >> 8) & 0xFF);
    emit (gt, size & 0xFF);

    generate_statements (gt, true_block->block.statements);

    size = size_statements (gt, false_block->block.statements);
    emit (gt, JUMP);
    emit (gt, (size >> 8) & 0xFF);
    emit (gt, size & 0xFF);

    generate_statements (gt, false_block->block.statements);

    if (node->message.is_statement)
	emit (gt, POP_STACK_TOP);
}

static int
size_func_2 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    int size= 0;

    st_node *true_block, *false_block;
	
    true_block = node->message.arguments;
    if (true_block->type != ST_BLOCK_NODE || true_block->block.arguments != NULL)
	generation_error (gt, "first argument of ifTrue:ifFalse message must be a 0-argument block", true_block);
    false_block = node->message.arguments->next;
    if (false_block->type != ST_BLOCK_NODE || false_block->block.arguments != NULL)
	generation_error (gt, "second argument of ifTrue:ifFalse message must be a 0-argument block", false_block);

    size += size_expression (gt, node->message.receiver);
    size += sizes[JUMP_TRUE];
    
    // true block
    size += size_statements (gt, node->message.arguments->block.statements);
    size += sizes[JUMP];
    size += size_statements (gt, node->message.arguments->next->block.statements);
    
    if (node->message.is_statement)
	size += sizes[POP_STACK_TOP];

    return size;
}

static void
generation_func_3 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    st_node *block;
    int size;

    block = node->message.receiver;
    if (block->type != ST_BLOCK_NODE || block->block.arguments != NULL)
	generation_error (gt, st_strdup_printf ("receiver of %s message must be a zero argument block",
					       subpattern_index == 0 ? "#whileTrue" : "#whileFalse"), block);

    generate_statements (gt, block->block.statements);
	
    // jump around jump statement
    if (subpattern_index == 0)
	emit (gt, JUMP_FALSE);
    else
	emit (gt, JUMP_TRUE);
    emit (gt, 0);
    emit (gt, 3);

    size = size_statements (gt, block->block.statements);
    size += sizes[JUMP_FALSE];
    // we jump backwards
    size = - size;
	
    emit (gt, JUMP);
    emit (gt, (size >> 8) & 0xFF);
    emit (gt, size & 0xFF);
	
    if (!node->message.is_statement)
	emit (gt, PUSH_NIL);
}

static int
size_func_3 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    st_node *block;
    int size = 0;

    block = node->message.receiver;
    if (block->type != ST_BLOCK_NODE || block->block.arguments != NULL)
	generation_error (gt, st_strdup_printf ("receiver of %s message must be a zero argument block",
					       subpattern_index == 0 ? "#whileTrue" : "#whileFalse"), block);

    size += size_statements (gt, node->message.receiver->block.statements);
    size += sizes[JUMP_TRUE];	
    size += sizes[JUMP];
    
    if (!node->message.is_statement)
	size += sizes[PUSH_NIL];

    return size;
}

static void
generation_func_4 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    st_node *block;
    int size;

    block = node->message.receiver;
    if (block->type != ST_BLOCK_NODE || block->block.arguments != NULL)
	generation_error (gt, st_strdup_printf ("receiver of %s message must be a zero argument block",
					       subpattern_index == 0 ? "#whileTrue:" : "#whileFalse:"), block);

    block = node->message.arguments;
    if (block->type != ST_BLOCK_NODE || block->block.arguments != NULL)
	generation_error (gt, st_strdup_printf ("argument of %s message must be a zero argument block",
					       subpattern_index == 0 ? "#whileTrue:" : "#whileFalse:"), block);
	
    generate_statements (gt, node->message.receiver->block.statements);
	
    if (subpattern_index == 0)
	emit (gt, JUMP_FALSE);
    else
	emit (gt, JUMP_TRUE);

    size = size_statements (gt, node->message.arguments->block.statements);
    // include size of POP and JUMP statement
    size += sizes[POP_STACK_TOP] + sizes[JUMP];

    emit (gt, (size >> 8) & 0xFF);
    emit (gt, size & 0xFF);

    generate_statements (gt, node->message.arguments->block.statements);
	
    size += size_statements (gt, node->message.receiver->block.statements);
    size = - size;

    emit (gt, POP_STACK_TOP);

    emit (gt, JUMP);
    emit (gt, (size >> 8) & 0xFF);
    emit (gt, size & 0xFF);

    if (!node->message.is_statement)
	emit (gt, PUSH_NIL);

}

static int
size_func_4 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    st_node *block;
    int size = 0;

    block = node->message.receiver;
    if (block->type != ST_BLOCK_NODE || block->block.arguments != NULL)
	generation_error (gt, st_strdup_printf ("receiver of %s message must be a zero argument block",
					       subpattern_index == 0 ? "#whileTrue:" : "#whileFalse:"), block);

    block = node->message.arguments;
    if (block->type != ST_BLOCK_NODE || block->block.arguments != NULL)
	generation_error (gt, st_strdup_printf ("argument of %s message must be a zero argument block",
					       subpattern_index == 0 ? "#whileTrue:" : "#whileFalse:"), block);

    size += size_statements (gt, node->message.receiver->block.statements);
    size += sizes[JUMP_TRUE];
    size += size_statements (gt, node->message.arguments->block.statements);
    size += sizes[POP_STACK_TOP] + sizes[JUMP];
    
    if (!node->message.is_statement)
	size += sizes[PUSH_NIL];
    
    return size;
}

static void
generation_func_5 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    st_node *block;
    int size;

    block = node->message.arguments;
    if (block->type != ST_BLOCK_NODE || block->block.arguments != NULL)
	generation_error (gt, st_strdup_printf ("argument of %s message must be a zero argument block",
					subpattern_index == 0 ? "#and:" : "#or:"), block);

    generate_expression (gt, node->message.receiver);

    size = size_statements (gt, block->block.statements);
    // size of JUMP instr after block statements
    size += 3;

    if (subpattern_index == 0)
	emit (gt, JUMP_FALSE);
    else
	emit (gt, JUMP_TRUE);
    emit (gt, (size >> 8) & 0xFF);
    emit (gt, size & 0xFF);

    generate_statements (gt, block->block.statements);

    emit (gt, JUMP);
    emit (gt, 0);
    emit (gt, 1);

    if (subpattern_index == 0)
	emit (gt, PUSH_FALSE);
    else
	emit (gt, PUSH_TRUE);

    if (node->message.is_statement)
	emit (gt, POP_STACK_TOP);

}

static int
size_func_5 (Generator *gt, st_node *node, st_uint subpattern_index)
{
    int size = 0;

    size += size_expression (gt, node->message.receiver);
    size += sizes[JUMP_TRUE];   
    size += size_statements (gt, node->message.arguments->block.statements);
    size += sizes[JUMP];	
    size += sizes[PUSH_TRUE];
    
    if (node->message.is_statement)
	size += sizes[POP_STACK_TOP];
    
    return size;
}

typedef void (* CodeGenerationFunc) (Generator *gt, st_node *message, st_uint subpattern_index);
typedef int  (* CodeSizeFunc)       (Generator *gt, st_node *message, st_uint subpattern_index);

static const struct generators {
    
    const char *const pattern[3];
    CodeGenerationFunc generation_func;
    CodeSizeFunc       size_func;

} generators[] =
{
    { { "ifTrue:", "ifFalse", NULL },
      generation_func_1, size_func_1 },
    { { "ifTrue:ifFalse:", "ifFalse:ifTrue:", NULL },
      generation_func_2, size_func_2 },
    { { "whileTrue", "whileFalse", NULL },
      generation_func_3, size_func_3 },
    { { "whileTrue:", "whileFalse:", NULL },
      generation_func_4, size_func_4 },
    { { "and:", "or:", NULL },
      generation_func_5, size_func_5 },
};

static int
size_message_send (Generator *gt, st_node *node)
{
    st_node *args;
    int size = 0;

    args = node->message.arguments;
    for (; args; args = args->next)
	size += size_expression (gt, args);

    /* check if message is a special */
    for (int i = 0; i < ST_N_ELEMENTS (st_specials); i++) {
	if (!node->message.super_send && node->message.selector == st_specials[i]) {
	    size += 1;
	    goto out;
	}
    }

    size += sizes[SEND];

out:

    if (node->message.is_statement)
	size += sizes[POP_STACK_TOP];
    
    return size;
}

static void
generate_message_send (Generator *gt, st_node *node)
{
    st_node *args;
    st_uint   argcount = 0;

    /* generate arguments */
    args = node->message.arguments;
    for (; args; args = args->next) {
	generate_expression (gt, args);
	argcount++;
    }

    /* check if message is a special */
    for (int i = 0; i < ST_N_ELEMENTS (st_specials); i++) {
	if (!node->message.super_send && node->message.selector == st_specials[i]) {
	    emit (gt, SEND_PLUS + i);
	    goto out;
	}
    }

    if (node->message.super_send)
	emit (gt, SEND_SUPER);
    else
	emit (gt, SEND);

    emit (gt, (st_uchar) argcount);
	
    int index = find_literal_const (gt, node->message.selector);
    emit (gt, (st_uchar) index);

out:

    if (node->message.is_statement)
	emit (gt, POP_STACK_TOP);

}

static int
size_cascade (Generator *gt, st_node *node)
{
    st_uint i, j;
    st_uint size = 0;

    size += size_expression (gt, node->cascade.receiver);
    size += sizes[DUPLICATE_STACK_TOP];

    for (st_list *l = node->cascade.messages; l; l = l->next) {

	size += size_message_send (gt, (st_node *) l->data); 

	if (l->next || node->cascade.is_statement)
	    size += sizes[POP_STACK_TOP];

	if (l->next && l->next->next)
	    size += sizes[DUPLICATE_STACK_TOP];
    }

    return size;
}

static void
generate_cascade (Generator *gt, st_node *node)
{
    st_uint i, j;

    generate_expression (gt, node->cascade.receiver);
    emit (gt, DUPLICATE_STACK_TOP);

    for (st_list *l = node->cascade.messages; l; l = l->next) {

	generate_message_send (gt, (st_node*) l->data); 

	if (l->next || node->cascade.is_statement)
	    emit (gt, POP_STACK_TOP);

	if (l->next && l->next->next)
	    emit (gt, DUPLICATE_STACK_TOP);
    }
}

static int
size_message (Generator *gt, st_node *node)
{
    const char *selector;
    st_uint i, j;
    st_uint size = 0;

    selector = CSTRING (node->message.selector);

    for (i = 0; i < ST_N_ELEMENTS (generators); i++) {
	for (j = 0; j < ST_N_ELEMENTS (generators[i].pattern); j++) {
	    if (generators[i].pattern[j] && strcmp (generators[i].pattern[j], selector) == 0) {
		size += generators[i].size_func (gt, node, j);
		return size;
	    }
	}
    }

    size += size_expression (gt, node->message.receiver);
    size += size_message_send (gt, node);

    return size;
}

static void
generate_message (Generator *gt, st_node *node)
{
    const char *selector;
    st_uint i, j;

    selector = CSTRING (node->message.selector);

    for (i = 0; i < ST_N_ELEMENTS (generators); i++) {
	for (j = 0; j < ST_N_ELEMENTS (generators[i].pattern); j++) {
	    if (generators[i].pattern[j] && strcmp (generators[i].pattern[j], selector) == 0) {
		generators[i].generation_func (gt, node, j);
		return;
	    }
	}
    }

    generate_expression (gt, node->message.receiver);
    generate_message_send (gt, node);
}

static int
size_expression (Generator *gt, st_node *node)
{
    int index;
    int size = 0;

    switch (node->type) {
    case ST_VARIABLE_NODE:
    {
	const char *name = node->variable.name;

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
	
	index = find_temporary (gt, node->variable.name);
	if (index >= 0) {
	    size += 2;
	    break;
	}
	index = find_instvar (gt, node->variable.name);
	if (index >= 0) {
	    size += 2;
	    break;
	}
	index = find_literal_var (gt, node->variable.name);
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

	size += size_message (gt, node);
	break;

    case ST_CASCADE_NODE:

	size += size_cascade (gt, node);
	break;

    default:
	st_assert_not_reached ();
    }

    return size;
}

static void
generate_expression (Generator *gt, st_node *node)
{   
    int index;

    switch (node->type) {
    case ST_VARIABLE_NODE:
    {
	const char *name = node->variable.name;
	
	if (streq (name, "self")) {
	    push_special (gt, PUSH_SELF);
	    break;
	} else if (streq (name, "super")) {
	    gt->references_super = true;
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

	index = find_temporary (gt, node->variable.name);
	if (index >= 0) {
	    push (gt, PUSH_TEMP, index);
	    break;
	}
	index = find_instvar (gt, node->variable.name);
	if (index >= 0) {
	    push (gt, PUSH_INSTVAR, index);
	    break;
	}
	index = find_literal_var (gt, node->variable.name);
	if (index >= 0) {
	    push (gt, PUSH_LITERAL_VAR, index);
	    break;
	}
	generation_error (gt, "unknown variable", node);

    }
    case ST_LITERAL_NODE:
	index = find_literal_const (gt, node->literal.value);
	push (gt, PUSH_LITERAL_CONST, index);
	break;

    case ST_ASSIGN_NODE:
	generate_assign (gt, node, false);
	break;

    case ST_BLOCK_NODE:	
	generate_block (gt, node);
	break;

    case ST_MESSAGE_NODE:
	generate_message (gt, node);
	break;

    case ST_CASCADE_NODE:
	generate_cascade (gt, node);
	break;

    default:
	st_assert_not_reached ();
    }
}

static int
size_statements (Generator *gt, st_node *statements)
{
    int size = 0;

    if (statements == NULL)
	size += sizes[PUSH_NIL];

    for (st_node *node = statements; node; node = node->next) {

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

	    st_assert (node->next == NULL);	    
	    size += size_return (gt, node);
	    return size;
	    
	case ST_MESSAGE_NODE:

	    size += size_message (gt, node);
	    break;

	case ST_CASCADE_NODE:

	    size += size_cascade (gt, node);
	    break;
	    
	default:
	    st_assert_not_reached ();
	}
    }

    return size;
}

static void
generate_statements (Generator *gt, st_node *statements)
{
    if (statements == NULL) {
	emit (gt, PUSH_NIL);
    }

    for (st_node *node = statements; node; node = node->next) {

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
	    
	    st_assert (node->next == NULL);
	    generate_return (gt, node);
	    return;
	    
	case ST_MESSAGE_NODE:

	    generate_message (gt, node);
	    break;
	    
	case ST_CASCADE_NODE:

	    generate_cascade (gt, node);
	    break;

	default:
	    st_assert_not_reached ();
	}
    }
}


static void
generate_method_statements (Generator *gt, st_node *statements)
{
   for (st_node *node = statements; node; node = node->next) {

       switch (node->type) {
	   
       case ST_VARIABLE_NODE:
       case ST_LITERAL_NODE:
       case ST_BLOCK_NODE:
	    break;
	    
       case ST_ASSIGN_NODE:
	   
	    generate_assign (gt, node, true);		
	    break;
	    
	case ST_RETURN_NODE:

	    st_assert (node->next == NULL);	    
	    generate_return (gt, node);
	    return;
	    
	case ST_MESSAGE_NODE:

	    generate_message (gt, node);
	    break;

	case ST_CASCADE_NODE:

	    generate_cascade (gt, node);
	    break;
	    
       default:
	   st_assert_not_reached ();
       }
    }
   
   emit (gt, PUSH_SELF);
   emit (gt, RETURN_STACK_TOP);
}

static st_list *
collect_temporaries (Generator *gt, st_node *node)
{
    st_list *temps = NULL;

    if (node == NULL)
	return NULL;

    if (node->type == ST_BLOCK_NODE)
	temps = st_list_concat (get_block_temporaries (gt, node->block.arguments),
			       get_block_temporaries (gt, node->block.temporaries));

    switch (node->type) {
    case ST_BLOCK_NODE:
	temps = st_list_concat (temps, collect_temporaries (gt, node->block.statements));
	break;
    case ST_ASSIGN_NODE:
	temps = st_list_concat (temps, collect_temporaries (gt, node->assign.expression));
	break;
    case ST_RETURN_NODE:
	temps = st_list_concat (temps, collect_temporaries (gt, node->retrn.expression));
	break;
    case ST_MESSAGE_NODE:
	temps = st_list_concat (temps, collect_temporaries (gt, node->message.receiver));
	temps = st_list_concat (temps, collect_temporaries (gt, node->message.arguments));
	break;
	break;
    case ST_CASCADE_NODE:
	temps = st_list_concat (temps, collect_temporaries (gt, node->cascade.receiver));
	for (st_list *l = node->cascade.messages; l; l = l->next)
	    temps = st_list_concat (temps, collect_temporaries (gt, (st_node *) l->data));

	break;

    case ST_METHOD_NODE:
    case ST_VARIABLE_NODE:
    case ST_LITERAL_NODE:
	break;
    }

    temps = st_list_concat (temps, collect_temporaries (gt, node->next));

    return temps;
}

st_oop
st_generate_method (st_oop class, st_node *node, st_compiler_error *error)
{
    Generator *gt;
    st_oop     method;

    st_assert (class != st_nil);
    st_assert (node != NULL && node->type == ST_METHOD_NODE);
    
    check_init ();

    gt = generator_new ();
    gt->error = error;

    if (setjmp (gt->jmploc))
	goto error;

    gt->class = class;
    gt->instvars = st_behavior_all_instance_variables (class);
    gt->temporaries = get_temporaries (gt, gt->instvars, node->method.arguments, node->method.temporaries);

    /* collect all block-level temporaries */
    gt->temporaries = st_list_concat (gt->temporaries, collect_temporaries (gt, node->method.statements));

    // generate bytecode
    generate_method_statements (gt, node->method.statements);

    method = st_object_new (st_compiled_method_class);

    ST_METHOD_HEADER (method) = st_smi_new (0);

    st_method_set_arg_count   (method, st_node_list_length (node->method.arguments));
    st_method_set_temp_count  (method, st_list_length (gt->temporaries) - st_node_list_length (node->method.arguments));
    st_method_set_stack_depth (method, gt->max_stack_depth);

    if (node->method.primitive >= 0) {
	st_method_set_flags (method, ST_METHOD_PRIMITIVE);	
    } else {
	st_method_set_flags (method, ST_METHOD_NORMAL);
    }

    st_method_set_primitive_index (method, node->method.primitive);

    ST_METHOD_LITERALS (method) = create_literals_array (gt);
    ST_METHOD_BYTECODE (method) = create_bytecode_array (gt); 
    ST_METHOD_SELECTOR (method) = node->method.selector;

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
print_bytecodes (st_oop literals, st_uchar *codes, int len) 
{
    static const char * const formats[] = {
	"<%02x>       ",
	"<%02x %02x>    ",
	"<%02x %02x %02x> ",
    };

    st_uchar *ip = codes;

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

	case DUPLICATE_STACK_TOP:
	    printf (FORMAT (ip), ip[0]);
	    printf ("dup");

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
	    printf ("sendSpecial: #%s", st_byte_array_bytes (st_specials[ip[0] - SEND_PLUS]));

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
	st_unichar_to_utf8 (st_character_value (lit), outbuf);
	printf ("$%s", outbuf);
    }
}


static void
print_literals (st_oop literals)
{
    if (literals == st_nil)
	return;

    printf ("literals: ");

    for (int i = 1; i <= st_smi_value (ST_ARRAYED_OBJECT (literals)->size); i++) {
	
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
    st_uchar *bytecodes;
    int     size;

    printf ("flags: %i; ", st_method_get_flags (method));
    printf ("arg-count: %i; ", st_method_get_arg_count (method));
    printf ("temp-count: %i; ", st_method_get_temp_count (method));
    printf ("stack-depth: %i; ", st_method_get_stack_depth (method));
    printf ("primitive: %i;\n", st_method_get_primitive_index (method));
    
    printf ("\n");

    literals  = ST_METHOD_LITERALS (method);
    bytecodes = st_method_bytecode_bytes (method);
    size      = ST_ARRAYED_OBJECT (ST_METHOD_BYTECODE (method))->size;

    print_bytecodes (literals, bytecodes, size);
    
    print_literals (literals);
}

