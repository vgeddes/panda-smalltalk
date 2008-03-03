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

    PUSH_ACTIVE_CONTEXT,

    BLOCK_COPY,

    JUMP_TRUE,
    JUMP_FALSE,
    JUMP,

    SEND,        /* B, B (arg count), B (selector index) */ 
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
    
} Generator;

st_oop specials[NUM_SPECIALS] = { 0 };

typedef struct
{
    guint       size;
    const char *format;

} CodeDescriptor;

CodeDescriptor codes[255] = { { 0 }, }; 


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

    codes[PUSH_TEMP].size             = 2;
    codes[PUSH_INSTVAR].size          = 2;
    codes[PUSH_LITERAL_CONST].size    = 2;
    codes[PUSH_LITERAL_VAR].size      = 2;
    codes[PUSH_SELF].size             = 1;
    codes[PUSH_NIL].size              = 1;
    codes[PUSH_TRUE].size             = 1;
    codes[PUSH_FALSE].size            = 1;
    codes[STORE_LITERAL_VAR].size     = 2;
    codes[STORE_TEMP].size            = 2;
    codes[STORE_INSTVAR].size         = 2; 
    codes[STORE_POP_LITERAL_VAR].size = 2;
    codes[STORE_POP_TEMP].size        = 2;
    codes[STORE_POP_INSTVAR].size     = 2;
    codes[RETURN_STACK_TOP].size      = 1;
    codes[BLOCK_RETURN].size          = 1;
    codes[POP_STACK_TOP].size         = 1;
    codes[PUSH_ACTIVE_CONTEXT].size   = 1;
    codes[BLOCK_COPY].size       = 2;
    codes[JUMP_TRUE].size        = 3;
    codes[JUMP_FALSE].size       = 3;
    codes[JUMP].size             = 3;
    codes[SEND].size             = 3;    
    codes[SEND_SUPER].size       = 3;
    codes[SEND_PLUS].size        = 1;
    codes[SEND_MINUS].size       = 1;
    codes[SEND_LT].size          = 1;
    codes[SEND_GT].size          = 1;
    codes[SEND_LE].size          = 1;
    codes[SEND_GE].size          = 1;
    codes[SEND_EQ].size          = 1;
    codes[SEND_NE].size          = 1;
    codes[SEND_MUL].size         = 1;
    codes[SEND_DIV].size         = 1;
    codes[SEND_MOD].size         = 1;
    codes[SEND_BITSHIFT].size    = 1;
    codes[SEND_BITAND].size      = 1;
    codes[SEND_BITOR].size       = 1;
    codes[SEND_BITXOR].size      = 1;
    codes[SEND_AT].size          = 1; 
    codes[SEND_AT_PUT].size      = 1;
    codes[SEND_SIZE].size        = 1;
    codes[SEND_VALUE].size       = 1;
    codes[SEND_VALUE_ARG].size   = 1;
    codes[SEND_IDENTITY_EQ].size = 1;
    codes[SEND_CLASS].size       = 1;
    codes[SEND_NEW].size         = 1;
    codes[SEND_NEW_ARG].size     = 1;

}

static int size_message (Generator *gt, STNode *node);
static int size_expression (Generator *gt, STNode *node);
static int size_statements (Generator *gt, STNode *node, bool optimized_block);


static void generate_expression (Generator *gt, STNode *node);
static void generate_statements (Generator *gt, STNode *statements, bool optimized_block);

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
jump_offset (Generator *gt, int offset)
{
    g_assert (offset <= G_MAXINT16);
    
    push (gt, JUMP);

    /* push high byte */
    push (gt, (offset >> 8) & 0xFF);
    /* push low byte */
    push (gt, offset & 0xFF);

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
}

static void
push_instance_var (Generator *gt, int index)
{
    g_assert (index <= 255);

    push (gt, PUSH_INSTVAR);
    push (gt, (guchar) index);
}

static void
push_literal_var (Generator *gt, int index)
{
    g_assert (index <= 255);
    
    push (gt, PUSH_LITERAL_VAR);
    push (gt, (guchar) index);
}

static void
push_literal_const (Generator *gt, int index)
{
    g_assert (index <= 255);
  
    push (gt, PUSH_LITERAL_CONST);
    push (gt, (guchar) index);
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
    
    generation_error ("unknown variable", node);
}

static int
size_return (Generator *gt, STNode *node)
{
    int size = 0;

    size += size_expression (gt, node->expression);

    size += 1;

    return size;
}

static void
generate_return (Generator *gt, STNode *node)
{
    generate_expression (gt, node->expression);

    push (gt, RETURN_STACK_TOP);
}

static GList *
get_block_temporaries (Generator *gt, STNode *temporaries)
{
    GList *temps = NULL;

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
size_block (Generator *gt, STNode *node)
{
    int size = 0;

    /* BLOCKCOPY instruction */
    size += 2;

    /* JUMP instruction */
    size += 3;
    
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
    gt->temporaries = g_list_append (gt->temporaries,
				     get_block_temporaries (gt, node->arguments));
    gt->temporaries = g_list_append (gt->temporaries,
				     get_block_temporaries (gt, node->temporaries));

    push (gt, BLOCK_COPY);
    push (gt, st_node_list_length (node->arguments));

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
    st_oop sel = msg->selector;

    if (st_object_equal (sel, st_symbol_new ("ifTrue:")) ||
	st_object_equal (sel, st_symbol_new ("ifFalse:"))) {

	if (msg->arguments->type == ST_BLOCK_NODE)
	    return true;
    }

    if (st_object_equal (sel, st_symbol_new ("ifTrue:ifFalse:"))
	|| st_object_equal (sel, st_symbol_new ("ifFalse:ifTrue:"))) {

	if (msg->arguments->type == ST_BLOCK_NODE
	    && msg->arguments->next->type == ST_BLOCK_NODE)
	    return true;
    }

    if (st_object_equal (sel, st_symbol_new ("whileTrue"))
	|| st_object_equal (sel, st_symbol_new ("whileFalse"))) {

	if (msg->receiver->type == ST_BLOCK_NODE)
	    return true;
    }

    if (st_object_equal (sel, st_symbol_new ("whileTrue:"))	
	|| st_object_equal (sel, st_symbol_new ("whileFalse:"))) {

	if (msg->receiver->type == ST_BLOCK_NODE
	    && msg->arguments->type == ST_BLOCK_NODE)
	    return true;
    }

    if (st_object_equal (sel, st_symbol_new ("and:"))
	|| st_object_equal (sel, st_symbol_new ("or:"))) {
	
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

   if (st_object_equal (msg->selector, st_string_new ("ifTrue:"))
       || st_object_equal (msg->selector, st_string_new ("ifFalse:"))) {   

       size += size_expression (gt, msg->receiver);
      
      // JUMP_{TRUE,FALSE} instr
       size += 3;

       size += size_statements (gt, msg->arguments->statements, true);

       if (is_expr) {
	   // JUMP
	   size += 3;
	   // PUSH_NIL
	   size += 1;
       } else {
	   size += 1;
       }
       
   }  else if (st_object_equal (msg->selector, st_string_new ("ifTrue:ifFalse:"))) {

       size += size_expression (gt, msg->receiver);
       
       // JUMP_TRUE/FALSE instr
       size += 3;
       
       // true block
       size += size_statements (gt, msg->arguments->statements, true);
       
       // JUMP instr
       size += 3;

       size += size_statements (gt, msg->arguments->next->statements, true);

       if (!is_expr)
	   // POP
	   size += 1;

   } else if (st_object_equal (msg->selector, st_string_new ("whileTrue"))
	      || st_object_equal (msg->selector, st_string_new ("whileFalse"))) {

       size += size_statements (gt, msg->receiver->statements, true);

       // JUMP_FALSE/TRUE instr
       size += 3;

       // JUMP instr
       size += 3;

       if (is_expr)
	   size += 1;

   } else if (st_object_equal (msg->selector, st_string_new ("whileTrue:"))
	      || st_object_equal (msg->selector, st_string_new ("whileFalse:"))) {

       size += size_statements (gt, msg->receiver->statements, true);

       // JUMP_FALSE/TRUE instr
       size += 3;

       size += size_statements (gt, msg->arguments->statements, true);

       // JUMP instr
       size += 3;

       if (is_expr)
	   size += 1;

   } else if (st_object_equal (msg->selector, st_string_new ("and:"))
	      || st_object_equal (msg->selector, st_string_new ("or:"))) {
   
      size += size_expression (gt, msg->receiver);

      // JUMP_FALSE/TRUE instr
      size += 3;

      size += size_statements (gt, msg->arguments->statements, true);

      // JUMP instr
      size += 3;

      // PUSH_{TRUE,FALSE} instr
      size += 1;

      if (!is_expr)
	  size += 1;

   }
  
   return size;
}


static void
generate_optimized_message (Generator *gt, STNode *msg, bool is_expr)
{
    st_oop selector = msg->selector;
    STNode *block;
    int size;

    if (st_object_equal (selector, st_symbol_new ("ifTrue:")) ||
	st_object_equal (selector, st_symbol_new ("ifFalse:"))) {
	
	block = msg->arguments;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL)
	    generation_error ("argument of ifTrue: message must be a 0-argument block", block);  
       
	generate_expression (gt, msg->receiver);
	
	size = size_statements (gt, block->statements, true);

	if (is_expr)
	    size += 3;
	else
	    size += 1;

	if (st_object_equal (selector, st_symbol_new ("ifTrue:")))
	    push (gt, JUMP_FALSE);
	else
	    push (gt, JUMP_TRUE);
	push (gt, (size >> 8) & 0xFF);
	push (gt, size & 0xFF);
	
	generate_statements (gt, block->statements, true);
	
	if (is_expr) {
	    push (gt, JUMP);
	    push (gt, 0);
	    push (gt, 1);
	    push (gt, PUSH_NIL);
	} else {
	    push (gt, POP_STACK_TOP);
	}

	return;
    }

    if (st_object_equal (selector, st_symbol_new ("ifTrue:ifFalse:")) ||
	st_object_equal (selector, st_symbol_new ("ifFalse:ifTrue:"))) {
	
	STNode *true_block, *false_block;
	
	true_block = msg->arguments;
	if (true_block->type != ST_BLOCK_NODE || true_block->arguments != NULL)
	    generation_error ("first argument of ifTrue:ifFalse message must be a 0-argument block", true_block);
	false_block = msg->arguments->next;
	if (false_block->type != ST_BLOCK_NODE || false_block->arguments != NULL)
	    generation_error ("second argument of ifTrue:ifFalse message must be a 0-argument block", false_block);

	generate_expression (gt, msg->receiver);

	// add 3 for size of jump instr at end of true block
	size = size_statements (gt, true_block->statements, true);
	size += 3; 
	
	if (st_object_equal (selector, st_symbol_new ("ifTrue:ifFalse:")))
	    push (gt, JUMP_FALSE);
	else
	    push (gt, JUMP_TRUE);
	push (gt, (size >> 8) & 0xFF);
	push (gt, size & 0xFF);

	generate_statements (gt, true_block->statements, true);

	size = size_statements (gt, false_block->statements, true);
      	push (gt, JUMP);
	push (gt, (size >> 8) & 0xFF);
	push (gt, size & 0xFF);

	generate_statements (gt, false_block->statements, true);

	/* if this is a statement we pop the last value on the stack */
	if (!is_expr)
	    push (gt, POP_STACK_TOP);

	return;
    }

    if (st_object_equal (selector, st_symbol_new ("whileTrue")) ||
	st_object_equal (selector, st_symbol_new ("whileFalse"))) {

	block = msg->receiver;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL) {
	    if (st_object_equal (selector, st_symbol_new ("whileTrue")))
		generation_error ("receiver of whileTrue message must be a 0-argument block", block);
	    else
		generation_error ("receiver of whileFalse message must be a 0-argument block", block);
	}

	generate_statements (gt, block->statements, true);
	
	// jump around jump statement
	if (st_object_equal (selector, st_symbol_new ("whileTrue")))
	    push (gt, JUMP_FALSE);
	else
	    push (gt, JUMP_TRUE);
	push (gt, 0);
	push (gt, 3);

	size = size_statements (gt, block->statements, true);
	// size of JUMP_FALSE instr
	size += 3;
	// we jump backwards
	size = - size;
	
	push (gt, JUMP);
	push (gt, (size >> 8) & 0xFF);
	push (gt, size & 0xFF);
	
	if (is_expr)
	    push (gt, PUSH_NIL);

	return;
    }
    
    if (st_object_equal (selector, st_symbol_new ("whileTrue:")) ||
	st_object_equal (selector, st_symbol_new ("whileFalse:"))) {

	block = msg->receiver;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL) {
	    if (st_object_equal (selector, st_symbol_new ("whileTrue:")))
		generation_error ("receiver of whileTrue: message must be a 0-argument block", block);
	    else
		generation_error ("receiver of whileFalse: message must be a 0-argument block", block);
	}

	block = msg->arguments;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL) {
	    if (st_object_equal (selector, st_symbol_new ("whileTrue:")))
		generation_error ("argument of whileTrue: message must be a 0-argument block", block);
	    else
		generation_error ("argument of whileFalse: message must be a 0-argument block", block);

	}
	
	generate_statements (gt, msg->receiver->statements, true);
	
	if (st_object_equal (msg->selector, st_string_new ("whileTrue:")))
	    push (gt, JUMP_FALSE);
	else
	    push (gt, JUMP_TRUE);

	size = size_statements (gt, msg->arguments->statements, true);
	// include size of POP and JUMP statement
	size += 1 + 3;

	push (gt, (size >> 8) & 0xFF);
	push (gt, size & 0xFF);

	generate_statements (gt, msg->arguments->statements, true);
	
	size += size_statements (gt, msg->receiver->statements, true);
	size = - size;

	push (gt, POP_STACK_TOP);

	push (gt, JUMP);
	push (gt, (size >> 8) & 0xFF);
	push (gt, size & 0xFF);

	if (is_expr)
	    push (gt, PUSH_NIL);

	return;
    }

    if (st_object_equal (selector, st_symbol_new ("and:")) ||
	st_object_equal (selector, st_symbol_new ("or:"))) {

	block = msg->arguments;
	if (block->type != ST_BLOCK_NODE || block->arguments != NULL) {
	    if (st_object_equal (selector, st_symbol_new ("and:")))
		generation_error ("argument of and: message must be a 0-argument block", block);
	    else
		generation_error ("argument of or: message must be a 0-argument block", block);
	}

	generate_expression (gt, msg->receiver);

	size = size_statements (gt, block->statements, true);
	// size of JUMP instr after block statements
	size += 3;

	if (st_object_equal (selector, st_symbol_new ("and:")))
	    push (gt, JUMP_FALSE);
	else
	    push (gt, JUMP_TRUE);
	push (gt, (size >> 8) & 0xFF);
	push (gt, size & 0xFF);
	
	generate_statements (gt, block->statements, true);

	push (gt, JUMP);
	push (gt, 0);
	push (gt, 1);

	if (st_object_equal (selector, st_symbol_new ("and:")))
	    push (gt, PUSH_FALSE);
	else
	    push (gt, PUSH_TRUE);

	if (!is_expr)
	    push (gt, POP_STACK_TOP);

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

    size += 3;
    
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


static int
size_expression (Generator *gt, STNode *node)
{
    int size = 0;

    switch (node->type) {
    case ST_VARIABLE_NODE:
    {
	if (st_object_equal (node->name, st_string_new ("self"))
	    || st_object_equal (node->name, st_string_new ("super"))) {
	    size += 1;
	    break;
	} else if (st_object_equal (node->name, st_string_new ("true"))) {
	    size += 1;
	    break;
	} else if (st_object_equal (node->name, st_string_new ("false"))) {
	    size += 1;
	    break;
	} else if (st_object_equal (node->name, st_string_new ("nil"))) {
	    size += 1;
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
	generation_error ("unknown variable", node);
    }
    case ST_LITERAL_NODE:
	size += 2;
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
		
	if (st_object_equal (node->name, st_string_new ("self"))
	    || st_object_equal (node->name, st_string_new ("super"))) {
	    push (gt, PUSH_SELF);
	    break;
	} else if (st_object_equal (node->name, st_string_new ("true"))) {
	    push (gt, PUSH_TRUE);
	    break;
	} else if (st_object_equal (node->name, st_string_new ("false"))) {
	    push (gt, PUSH_FALSE);
	    break;
	} else if (st_object_equal (node->name, st_string_new ("nil"))) {
	    push (gt, PUSH_NIL);
	    break;
	}

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

    case ST_LITERAL_NODE:
	index = find_literal_const (gt, node->literal);
	push_literal_const (gt, index);
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
	size += 1;
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
		    size += 1;
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
	size += 1;

    return size;
}

static void
generate_statements (Generator *gt, STNode *statements, bool optimized_block)
{
    if (statements == NULL) {
	push (gt, PUSH_NIL);
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
		    push (gt, POP_STACK_TOP);
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
	push (gt, BLOCK_RETURN);
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
		push (gt, POP_STACK_TOP);
	    }
	    break;
	    
       default:
	   g_assert_not_reached ();
       }
    }
   
   push (gt, PUSH_SELF);
   push (gt, RETURN_STACK_TOP);
}

static guint
compute_stack_depth (Generator *gt)
{
    int stackp = 0, stackp_max = 0;
    
    for (int ip = 0; ip < gt->size;) {

	switch ((Code) gt->code[ip]) {
	case PUSH_TEMP:
	case PUSH_INSTVAR:
	case PUSH_LITERAL_CONST:
	case PUSH_LITERAL_VAR:
	case PUSH_SELF:
	case PUSH_NIL:
	case PUSH_TRUE:
	case PUSH_FALSE:
	case BLOCK_COPY:

	    stackp++;
	    if (stackp > stackp_max)
		stackp_max = stackp;

	    ip += codes[gt->code[ip]].size;
	    break;

	case STORE_POP_LITERAL_VAR:
	case STORE_POP_TEMP:
	case STORE_POP_INSTVAR:
	case POP_STACK_TOP:
	    stackp--;
	    ip += codes[gt->code[ip]].size;
	    break;

	case SEND:
	case SEND_SUPER:
	    stackp -= (int) gt->code[ip + 1];
	    ip += codes[gt->code[ip]].size;
	    break;
	    	   
	case SEND_PLUS:   case SEND_MINUS:
	case SEND_LT:     case SEND_GT:
	case SEND_LE:     case SEND_GE:
	case SEND_EQ:     case SEND_NE:
	case SEND_MUL:    case SEND_DIV:
	case SEND_MOD:    case SEND_BITSHIFT:
	case SEND_BITAND: case SEND_BITOR:
	case SEND_BITXOR: case SEND_NEW_ARG:
	case SEND_AT: case SEND_IDENTITY_EQ:
	case SEND_VALUE_ARG:
	    stackp -= 1;
	    ip += codes[gt->code[ip]].size;
	    break;

	case SEND_AT_PUT:	    
	    stackp -= 2;
	    ip += codes[gt->code[ip]].size;
	    break;

	default:
	    // no stack activity
	    ip += codes[gt->code[ip]].size;
	}
    }

    return stackp_max;
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
    generate_method_statements (gt, node->statements);

    method = st_object_new (st_compiled_method_class);

    st_compiled_code_set_arg_count   (method, st_node_list_length (node->arguments));
    st_compiled_code_set_temp_count  (method, g_list_length (gt->temporaries) - st_node_list_length (node->arguments));
    st_compiled_code_set_stack_depth (method, compute_stack_depth (gt));

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
print_value (int size, guchar b1, guchar b2, guchar b3)
{
    static const char * const formats[] = {
	"<%02x>       ",
	"<%02x %02x>    ",
	"<%02x %02x %02x> ",
    };
    
    printf (formats[size], b1, b2, b3);

}

static void
print_bytecodes (st_oop literals, guchar *codes, int len) 
{
    guchar *ip = codes;

    while (*ip) {

	printf ("%3li ", ip - codes);

	switch ((Code) *ip) {
	    
	case PUSH_TEMP:
	    printf ("<%02x %02x>    ", ip[0], ip[1]); 
	    printf ("pushTemp: %i", ip[1]);
	    
	    ip += 2;
	    break;
	    
	case PUSH_INSTVAR:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("pushInstvar: %i", ip[1]);

	    ip += 2;
	    break;
	    
	case PUSH_LITERAL_CONST:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("pushConst: %i", ip[1]);

	    ip += 2;
	    break;

	case PUSH_LITERAL_VAR:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("pushLit: %i", ip[1]);

	    ip += 2;
	    break;

	case PUSH_SELF:
	    printf ("<%02x>       ", ip[0]);
	    printf ("push: self");

	    ip += 1;
	    break;

	case PUSH_TRUE:
	    printf ("<%02x>       ", ip[0]);
	    printf ("pushConst: true");

	    ip += 1;
	    break;
	    
	case PUSH_FALSE:
	    printf ("<%02x>       ", ip[0]);
	    printf ("pushConst: false");

	    ip += 1;
	    break;
	   
	case PUSH_NIL:
	    printf ("<%02x>       ", ip[0]);
	    printf ("pushConst: nil");

	    ip += 1;
	    break; 

	case STORE_TEMP:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("storeTemp: %i", ip[1]);

	    ip += 2;
	    break;

	case STORE_INSTVAR:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("storeInstvar: %i", ip[1]);

	    ip += 2;
	    break;

	case STORE_LITERAL_VAR:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("storeLiteral: %i", ip[1]);

	    ip += 2;
	    break;

	case STORE_POP_TEMP:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("popIntoTemp: %i", ip[1]);

	    ip += 2;
	    break;

	case STORE_POP_INSTVAR:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("popIntoInstvar: %i", ip[1]);

	    ip += 2;
	    break;

	case STORE_POP_LITERAL_VAR:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("popIntoLiteral: %i", ip[1]);

	    ip += 2;
	    break;

	case BLOCK_COPY:
	    printf ("<%02x %02x>    ", ip[0], ip[1]);
	    printf ("blockCopy: %i", ip[1]);

	    ip += 2;
	    break;

	case JUMP:
	    printf ("<%02x %02x %02x> ", ip[0], ip[1], ip[2]);

	    short offset = ((ip[1] << 8) | ip[2]);
	    printf ("jump: %i", (offset >= 0 ? 3 : 0) + (ip - codes) + offset);
	    
	    ip += 3;
	    break;

	case JUMP_TRUE:
	    printf ("<%02x %02x %02x> ", ip[0], ip[1], ip[2]);
	    printf ("jumpTrue: %i", 3 + (ip - codes) + ((ip[1] << 8) | ip[2]));

	    ip += 3;
	    break;

	case JUMP_FALSE:
	    printf ("<%02x %02x %02x> ", ip[0], ip[1], ip[2]);
	    printf ("jumpFalse: %i", 3 + (ip - codes) + ((ip[1] << 8) | ip[2]));

	    ip += 3;
	    break;

	case POP_STACK_TOP:
	    printf ("<%02x>       ", ip[0]);
	    printf ("pop");

	    ip += 1;
	    break;

	case RETURN_STACK_TOP:
	    printf ("<%02x>       ", ip[0]);
	    printf ("returnTop");

	    ip += 1;
	    break;

	case BLOCK_RETURN:
	    printf ("<%02x>       ", ip[0]);
	    printf ("blockReturn");

	    ip += 1;
	    break;

	case SEND:
	{
	    st_oop selector;

	    selector = st_array_at (literals, ip[2] + 1);

	    printf ("<%02x %02x %02x> ", ip[0], ip[1], ip[2]);

	    printf ("send: #%s", (char *) st_byte_array_bytes (selector));

	    ip += 3;
	    break;
	}
	case SEND_SUPER:
	{
	    st_oop selector;

	    selector = st_array_at (literals, ip[2] + 1);

	    printf ("<%02x %02x %02x> ", ip[0], ip[1], ip[2]);

	    printf ("sendSuper: #%s", (char *) st_byte_array_bytes (selector));

	    ip += 3;
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

	    printf ("<%02x>       ", ip[0]);
	    printf ("sendSpecial: #%s", st_byte_array_bytes (specials[ip[0] - SEND_PLUS]));

	    ip += 1;
	    break;



	    
	}
	printf ("\n");
    }
    
}

void st_generator_print_method (st_oop method);
static void print_block (st_oop block);

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
