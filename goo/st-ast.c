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

#include <st-ast.h>
#include <st-byte-array.h>
#include <st-types.h>
#include <st-object.h>
#include <st-float.h>
#include <string.h>


static void print_expression (STNode *expression);


static void
print_variable (STNode *variable)
{
    g_assert (variable->type == ST_VARIABLE_NODE);

    char *name = (char *) st_byte_array_bytes (variable->name);

    printf (name);
}

static void
print_literal (STNode *literal)
{
    st_oop value;

    g_assert (literal->type == ST_LITERAL_NODE); 
   
    value = literal->literal;

    if (st_object_is_smi (value)) {
	printf ("%li", st_smi_value (value));
  
    } else if (st_object_is_symbol (value)) {
	
	printf ("#%s", st_byte_array_bytes (value));
    } else if (st_object_class (value) == st_string_class) {

	printf ("%s", st_byte_array_bytes (value));
    } else if (st_object_is_float (value)) {

	printf ("%f", st_float_value (value));
    } else if (st_object_class (value) == st_character_class) {

	char outbuf[6] = { 0 };
	g_unichar_to_utf8 (st_character_value (value), outbuf);
	printf ("$%s", outbuf);
    }
}

static void
print_return (STNode *return_node)
{
   g_assert (return_node->type == ST_RETURN_NODE);

    printf ("^ ");
    print_expression (return_node->expression);
    
}

static void
print_assign (STNode *assign)
{
    g_assert (assign->type == ST_ASSIGN_NODE);
   
    print_variable (assign->assignee);
    printf (" := ");
    print_expression (assign->expression);
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
print_method (STNode *method)
{
    g_assert (method->type == ST_METHOD_NODE);

    if (method->precedence == ST_KEYWORD_PRECEDENCE) {

	char *selector = (char *) st_byte_array_bytes (method->selector);
	
	char **keywords = extract_keywords (selector);
	STNode *arguments = method->arguments;

	for (char **keyword = keywords; *keyword; keyword++) {
	    
	    printf ("%s: ", *keyword);
	    print_variable (arguments);
	    printf (" ");
	    
	    arguments = arguments->next;
	}
	g_strfreev (keywords);

    } else if (method->precedence == ST_BINARY_PRECEDENCE) {
	
	printf ((char *) st_byte_array_bytes (method->selector));    
	printf (" ");
	print_variable (method->arguments);
    } else {
	
	printf ((char *) st_byte_array_bytes (method->selector));	
    }

    printf ("\n");

    if (method->temporaries != NULL) {

	printf ("| ");
	STNode *temp = method->temporaries;
	for (; temp; temp = temp->next) {
	    print_variable (temp);
	    printf (" ");
	}
	printf ("|\n");
    }

    if (method->primitive >= 0) {
	
	printf ("<primitive: %i>\n", method->primitive);
	
    }
    

    STNode *stm = method->statements;
    for (; stm; stm = stm->next) {

	if (stm->type == ST_RETURN_NODE)
	    print_return (stm);
	else
	    print_expression (stm);

	printf (".\n");
    }
}


static void
print_block (STNode *block)
{   
    g_assert (block->type == ST_BLOCK_NODE);

    printf ("[ ");

    if (block->arguments != NULL) {

	STNode *arg = block->arguments;
	for (; arg; arg = arg->next) {
	    printf (":");
	    print_variable (arg);
	    printf (" ");
	}
	printf ("|");
        printf (" ");
    }

    if (block->temporaries != NULL) {

	printf ("| ");
	STNode *temp = block->temporaries;
	for (; temp; temp = temp->next) {
	    print_variable (temp);
	    printf (" ");
	}
	printf ("|");
	printf (" ");
    }

    STNode *stm = block->statements;
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
print_message (STNode *msg)
{

    if (msg->precedence == ST_UNARY_PRECEDENCE) {

	if (msg->receiver->type == ST_MESSAGE_NODE &&
	    (msg->receiver->precedence == ST_BINARY_PRECEDENCE ||
	     msg->receiver->precedence == ST_KEYWORD_PRECEDENCE))
	    printf ("(");

	print_expression (msg->receiver);

	if (msg->receiver->type == ST_MESSAGE_NODE &&
	    (msg->receiver->precedence == ST_BINARY_PRECEDENCE ||
	     msg->receiver->precedence == ST_KEYWORD_PRECEDENCE))
	    printf (")");

	printf (" ");
	printf ((char *) st_byte_array_bytes (msg->selector));    


    } else if (msg->precedence == ST_BINARY_PRECEDENCE) {

	if (msg->receiver->type == ST_MESSAGE_NODE &&
	    msg->receiver->precedence == ST_KEYWORD_PRECEDENCE)
	    printf ("(");

	print_expression (msg->receiver);

	if (msg->receiver->type == ST_MESSAGE_NODE &&
	    msg->receiver->precedence == ST_KEYWORD_PRECEDENCE)
	    printf (")");

	printf (" ");
	printf ((char *) st_byte_array_bytes (msg->selector));
	printf (" ");

	if (msg->arguments->type == ST_MESSAGE_NODE &&
	    (msg->arguments->precedence == ST_BINARY_PRECEDENCE ||
	     msg->arguments->precedence == ST_KEYWORD_PRECEDENCE))
	    printf ("(");

	print_expression (msg->arguments);

	if (msg->arguments->type == ST_MESSAGE_NODE &&
	    (msg->arguments->precedence == ST_BINARY_PRECEDENCE ||
	     msg->arguments->precedence == ST_KEYWORD_PRECEDENCE))
	    printf (")");
	
    } else if (msg->precedence == ST_KEYWORD_PRECEDENCE) {

	char   *selector = (char *) st_byte_array_bytes (msg->selector);

	char  **keywords = extract_keywords (selector);
	STNode *arguments = msg->arguments;

	if (msg->receiver->type == ST_MESSAGE_NODE &&
	    msg->receiver->precedence == ST_KEYWORD_PRECEDENCE)
	    printf ("(");
	print_expression (msg->receiver);
	if (msg->receiver->type == ST_MESSAGE_NODE &&
	     msg->receiver->precedence == ST_KEYWORD_PRECEDENCE)
	    printf (")");

	printf (" ");

	for (char **keyword = keywords; *keyword; keyword++) {
	    
	    printf ("%s: ", *keyword);

	if (msg->arguments->type == ST_MESSAGE_NODE &&
	    msg->arguments->precedence == ST_KEYWORD_PRECEDENCE)
	    printf ("(");
	    print_expression (arguments);
	if (msg->arguments->type == ST_MESSAGE_NODE &&
	    msg->arguments->precedence == ST_KEYWORD_PRECEDENCE)
	    printf (")");

	    printf (" ");
	    
	    arguments = arguments->next;
	}
	g_strfreev (keywords);

    }
}

static void
print_expression (STNode *expr)
{
    switch (expr->type) {
	
    case ST_LITERAL_NODE:
	print_literal (expr);
	break;
	
    case ST_VARIABLE_NODE:
	print_variable (expr);
	break;
	      
    case ST_ASSIGN_NODE:
	print_assign (expr);
	break;

    case ST_BLOCK_NODE:
	print_block (expr);
	break;

    case ST_MESSAGE_NODE:
	print_message (expr);
	break;

    }
}

void
st_print_method (STNode *method)
{
    g_assert (method->type == ST_METHOD_NODE);
    
    print_method (method);
}




STNode *
st_node_new (STNodeType type)
{
    STNode *node = g_slice_new0 (STNode);
    node->type = type;
    node->next = NULL;
    return node;
}


STNode *
st_node_append (STNode *list, STNode *node)
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
st_node_length (STNode *list)
{
    STNode *l   = list;
    int     len = 0;
    for (; l; l = l->next)
	++len;
    return len;
}

