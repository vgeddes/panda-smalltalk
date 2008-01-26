/*
 * st-bootstrap.c
 * 
 * Copyright (C) 2008 Vincent Geddes <vgeddes@gnome.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
print_selector (STNode *selector)
{
    g_assert (selector->type == ST_SELECTOR_NODE);

    char *symbol = (char *) st_byte_array_bytes (selector->name);
    printf (symbol);
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

	char *selector = (char *) st_byte_array_bytes (method->selector->name);
	
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

	print_selector (method->selector);
	printf (" ");
	print_variable (method->arguments);
    } else {
	
	print_selector (method->selector);	
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
    }
   
    printf (" ");

    if (block->temporaries != NULL) {

	printf ("| ");
	STNode *temp = block->temporaries;
	for (; temp; temp = temp->next) {
	    print_variable (temp);
	    printf (" ");
	}
	printf ("|");
    }
    
    printf (" ");

    STNode *stm = block->statements;
    for (; stm; stm = stm->next) {

	if (stm->type == ST_RETURN_NODE)
	    print_return (stm);
	else
	    print_expression (stm);

	printf (". ");
    }

    printf (" ]");
}


static void
print_message (STNode *msg)
{

    if (msg->precedence == ST_UNARY_PRECEDENCE) {

	print_expression (msg->receiver);
	printf (" ");
	print_selector (msg->selector);

    } else if (msg->precedence == ST_BINARY_PRECEDENCE) {

	print_expression (msg->receiver);
	printf (" ");
	print_selector (msg->selector);
	printf (" ");
	print_expression (msg->arguments);
	
    } else if (msg->precedence == ST_KEYWORD_PRECEDENCE) {

	char   *selector = (char *) st_byte_array_bytes (msg->selector->name);

	char  **keywords = extract_keywords (selector);
	STNode *arguments = msg->arguments;

	print_expression (msg->receiver);
	printf (" ");

	for (char **keyword = keywords; *keyword; keyword++) {
	    
	    printf ("%s: ", *keyword);
	    print_expression (arguments);
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

