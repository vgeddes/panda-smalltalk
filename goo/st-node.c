/*
 * st-bootstrap.c
 *
 * Copyright (c) 2008 Vincent Geddes
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

#include <st-node.h>
#include <st-byte-array.h>
#include <st-array.h>
#include <st-types.h>
#include <st-object.h>
#include <st-float.h>
#include <st-symbol.h>
#include <st-character.h>
#include <string.h>


static void print_expression (STNode *expression);

static void
print_variable (STNode *node)
{
    g_assert (node->type == ST_VARIABLE_NODE);

    printf (node->variable.name);
}

static void print_tuple (st_oop tuple);

static void
print_object (st_oop object)
{
    if (st_object_is_smi (object)) {
	printf ("%li", st_smi_value (object));
  
    } else if (st_object_is_symbol (object)) {
	
	printf ("#%s", st_byte_array_bytes (object));
    } else if (st_object_class (object) == st_string_class) {

	printf ("%s", st_byte_array_bytes (object));
    } else if (st_object_is_float (object)) {

	printf ("%f", st_float_value (object));
    } else if (st_object_class (object) == st_character_class) {

	char outbuf[6] = { 0 };
	g_unichar_to_utf8 (st_character_value (object), outbuf);
	printf ("$%s", outbuf);
    } else if (st_object_class (object) == st_array_class) {

	print_tuple (object);

    }
}

static void
print_tuple (st_oop tuple)
{
    int size;

    size = st_smi_value (st_array_size (tuple));

    printf ("#(");
    for (int i = 1; i <= size; i++) {
	print_object (st_array_at (tuple, i));	
	if (i < size)
	    printf (" ");
    }
    printf (")");
}

static void
print_literal (STNode *node)
{
    g_assert (node->type == ST_LITERAL_NODE); 
   
    print_object (node->literal.value);
}

static void
print_return (STNode *node)
{
   g_assert (node->type == ST_RETURN_NODE);

    printf ("^ ");
    print_expression (node->retrn.expression);
    
}

static void
print_assign (STNode *node)
{
    g_assert (node->type == ST_ASSIGN_NODE);
   
    print_variable (node->assign.assignee);
    printf (" := ");
    print_expression (node->assign.expression);
}

static char **
extract_keywords (char *selector)
{
    int len = strlen (selector);

    /* hide trailing ':' */
    selector[len - 1] = 0; 
    char **keywords = g_strsplit (selector, ":", 0);
    selector[len - 1] = ':'; 

    return keywords;
}

static void
print_method (STNode *node)
{
    g_assert (node->type == ST_METHOD_NODE);

    if (node->method.precedence == ST_KEYWORD_PRECEDENCE) {

	char *selector = (char *) st_byte_array_bytes (node->method.selector);
	
	char **keywords = extract_keywords (selector);
	STNode *arguments = node->method.arguments;

	for (char **keyword = keywords; *keyword; keyword++) {
	    
	    printf ("%s: ", *keyword);
	    print_variable (arguments);
	    printf (" ");
	    
	    arguments = arguments->next;
	}
	g_strfreev (keywords);

    } else if (node->method.precedence == ST_BINARY_PRECEDENCE) {
	
	printf ((char *) st_byte_array_bytes (node->method.selector));    
	printf (" ");
	print_variable (node->method.arguments);
    } else {
	
	printf ((char *) st_byte_array_bytes (node->method.selector));	
    }

    printf ("\n");

    if (node->method.temporaries != NULL) {

	printf ("| ");
	STNode *temp = node->method.temporaries;
	for (; temp; temp = temp->next) {
	    print_variable (temp);
	    printf (" ");
	}
	printf ("|\n");
    }

    if (node->method.primitive >= 0) {
	
	printf ("<primitive: %i>\n", node->method.primitive);
	
    }
    

    STNode *stm = node->method.statements;
    for (; stm; stm = stm->next) {

	if (stm->type == ST_RETURN_NODE)
	    print_return (stm);
	else
	    print_expression (stm);

	printf (".\n");
    }
}

static void
print_block (STNode *node)
{   
    g_assert (node->type == ST_BLOCK_NODE);

    printf ("[ ");

    if (node->block.arguments != NULL) {

	STNode *arg = node->block.arguments;
	for (; arg; arg = arg->next) {
	    printf (":");
	    print_variable (arg);
	    printf (" ");
	}
	printf ("|");
        printf (" ");
    }

    if (node->block.temporaries != NULL) {

	printf ("| ");
	STNode *temp = node->block.temporaries;
	for (; temp; temp = temp->next) {
	    print_variable (temp);
	    printf (" ");
	}
	printf ("|");
	printf (" ");
    }

    STNode *stm = node->block.statements;
    for (; stm; stm = stm->next) {
	if (stm->type == ST_RETURN_NODE)
	    print_return (stm);
	else
	    print_expression (stm);
	
	if (stm->next != NULL)
	    printf (". ");
    }
    
    printf (" ]");
}


static void
print_message (STNode *node)
{

    if (node->message.precedence == ST_UNARY_PRECEDENCE) {

	if (node->message.receiver->type == ST_MESSAGE_NODE &&
	    (node->message.receiver->message.precedence == ST_BINARY_PRECEDENCE ||
	     node->message.receiver->message.precedence == ST_KEYWORD_PRECEDENCE))
	    printf ("(");

	print_expression (node->message.receiver);

	if (node->message.receiver->type == ST_MESSAGE_NODE &&
	    (node->message.receiver->message.precedence == ST_BINARY_PRECEDENCE ||
	     node->message.receiver->message.precedence == ST_KEYWORD_PRECEDENCE))
	    printf ("(");

	printf (" ");
	printf ((char *) st_byte_array_bytes (node->message.selector));    


    } else if (node->message.precedence == ST_BINARY_PRECEDENCE) {

	if (node->message.receiver->type == ST_MESSAGE_NODE &&
	    node->message.receiver->message.precedence == ST_KEYWORD_PRECEDENCE)
	    printf ("(");

	print_expression (node->message.receiver);

	if (node->message.receiver->type == ST_MESSAGE_NODE &&
	    node->message.receiver->message.precedence == ST_KEYWORD_PRECEDENCE)
	    printf (")");

	printf (" ");
	printf ((char *) st_byte_array_bytes (node->message.selector));
	printf (" ");

	if (node->message.arguments->type == ST_MESSAGE_NODE &&
	    (node->message.arguments->message.precedence == ST_BINARY_PRECEDENCE ||
	     node->message.arguments->message.precedence == ST_KEYWORD_PRECEDENCE))
	    printf ("(");

	print_expression (node->message.arguments);

	if (node->message.arguments->type == ST_MESSAGE_NODE &&
	    (node->message.arguments->message.precedence == ST_BINARY_PRECEDENCE ||
	     node->message.arguments->message.precedence == ST_KEYWORD_PRECEDENCE))
	    printf (")");
	
    } else if (node->message.precedence == ST_KEYWORD_PRECEDENCE) {

	char   *selector = (char *) st_byte_array_bytes (node->message.selector);

	char  **keywords = extract_keywords (selector);
	STNode *arguments = node->message.arguments;

	if (node->message.receiver->type == ST_MESSAGE_NODE &&
	    node->message.receiver->message.precedence == ST_KEYWORD_PRECEDENCE)
	    printf ("(");
	print_expression (node->message.receiver);
	if (node->message.receiver->type == ST_MESSAGE_NODE &&
	     node->message.receiver->message.precedence == ST_KEYWORD_PRECEDENCE)
	    printf (")");

	printf (" ");

	for (char **keyword = keywords; *keyword; keyword++) {
	    
	    printf ("%s: ", *keyword);

	if (node->message.arguments->type == ST_MESSAGE_NODE &&
	    node->message.arguments->message.precedence == ST_KEYWORD_PRECEDENCE)
	    printf ("(");
	    print_expression (arguments);
	if (node->message.arguments->type == ST_MESSAGE_NODE &&
	    node->message.arguments->message.precedence == ST_KEYWORD_PRECEDENCE)
	    printf (")");

	    printf (" ");
	    
	    arguments = arguments->next;
	}
	g_strfreev (keywords);

    }
    
    if (node->next) {
	printf (". ");
	print_message (node->next);
    }
}

static void
print_expression (STNode *node)
{
    switch (node->type) {
	
    case ST_LITERAL_NODE:
	print_literal (node);
	break;
	
    case ST_VARIABLE_NODE:
	print_variable (node);
	break;
	      
    case ST_ASSIGN_NODE:
	print_assign (node);
	break;

    case ST_BLOCK_NODE:
	print_block (node);
	break;

    case ST_MESSAGE_NODE:
	print_message (node);
	break;

    }
}

void
st_print_method_node (STNode *node)
{
    g_assert (node && node->type == ST_METHOD_NODE);
    
    print_method (node);
}

STNode *
st_node_new (STNodeType type)
{
    STNode *node = g_slice_new0 (STNode);
    node->type = type;

    if (node->type == ST_MESSAGE_NODE)
	node->message.is_statement = false;

    if (node->type == ST_CASCADE_NODE)
	node->cascade.is_statement = false;


    return node;
}

STNode *
st_node_list_append (STNode *list, STNode *node)
{
    STNode *l = list;
    if (list == NULL)
	return node;
    while (l->next)
	l = l->next;
    l->next = node;
    return list;
}


guint
st_node_list_length (STNode *list)
{
    STNode *l   = list;
    int     len = 0;
    for (; l; l = l->next)
	++len;
    return len;
}

void
st_node_destroy (STNode *node)
{
    if (node == NULL)
	return;

    switch (node->type) {
    case ST_METHOD_NODE:
	st_node_destroy (node->method.arguments);
	st_node_destroy (node->method.temporaries);
	st_node_destroy (node->method.statements);
	break;

    case ST_BLOCK_NODE:
	st_node_destroy (node->block.arguments);
	st_node_destroy (node->block.temporaries);
	st_node_destroy (node->block.statements);
	break;

    case ST_ASSIGN_NODE:
	st_node_destroy (node->assign.assignee);
	st_node_destroy (node->assign.expression);
	break;

    case ST_RETURN_NODE:
	st_node_destroy (node->retrn.expression);
	break;

    case ST_MESSAGE_NODE:
	st_node_destroy (node->message.receiver);
	st_node_destroy (node->message.arguments);
	break;

    case ST_CASCADE_NODE:
	st_node_destroy (node->cascade.receiver);

	g_list_foreach (node->cascade.messages, (GFunc) st_node_destroy, NULL);
	g_list_free (node->cascade.messages);
	break;

    case ST_VARIABLE_NODE:
	
	g_free (node->variable.name);
	break;

    default:
	break;
    }
	
    st_node_destroy (node->next);
    g_slice_free (STNode, node);
}



