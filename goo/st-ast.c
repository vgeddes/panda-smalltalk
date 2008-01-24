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


st_node_type_t
st_node_type (st_node_t *node)
{
    return node->type;
}

bool
st_node_is_variable (st_node_t *node)
{
    return node->type >= ST_NODE_BLOCK
	|| node->type == ST_NODE_EXPRESSION
	|| node->type == ST_NODE_ASSIGN;
}	  

bool
st_node_is_message (st_node_t *node)
{
    return (node->type == ST_NODE_MESSAGE_UNARY   ||
	   node->type == ST_NODE_MESSAGE_BINARY  ||
	   node->type == ST_NODE_MESSAGE_KEYWORD);
}


#define DATA(l) ((st_node_t *) l->data)

st_node_t *
st_node_method_new (const char *selector,
		    GList *args,
		    GList *temps,
		    GList *statements)
{
    st_node_method_t *node;

    g_assert (selector != NULL);

    node = g_slice_new (st_node_method_t);

    ST_NODE_CAST (node)->type = ST_NODE_METHOD;

    node->selector   = g_strdup (selector);
    node->temps      = temps;
    node->args       = args;

    node->statements = statements;

    return ST_NODE_CAST (node);
}




st_node_t *
st_node_block_new (GList *args,
		   GList *temps,
		   GList *statements)
{
    st_node_block_t *node;

    node = g_slice_new (st_node_block_t);

    ST_NODE_CAST (node)->type = ST_NODE_BLOCK;

    node->args  = args;
    node->temps = temps;

    node->statements = statements;

    return ST_NODE_CAST (node);
}




st_node_t *
st_node_expression_new (st_node_t *receiver, st_node_t *message)
{
    st_node_expression_t *node;

    g_assert (receiver != NULL);
    g_assert (message  != NULL);

    node = g_slice_new (st_node_expression_t);

    ST_NODE_CAST (node)->type = ST_NODE_EXPRESSION;

    g_assert (st_node_is_variable (receiver));
    node->receiver = receiver;

    g_assert (st_node_is_message (message));
    node->message = message;

    return ST_NODE_CAST (node);
}



st_node_t *
st_node_return_new (st_node_t *variable)
{
    st_node_return_t *node;
    g_assert (variable != NULL);

    node = g_slice_new (st_node_return_t);
    ST_NODE_CAST (node)->type = ST_NODE_RETURN;

    g_assert (st_node_is_variable (variable));
    node->variable = variable;

    return ST_NODE_CAST (node);
}


st_node_t *
st_node_assign_new (st_node_t *assignee,
		    st_node_t *variable)
{
    st_node_assign_t *node;
    g_assert (variable != NULL);

    node = g_slice_new (st_node_assign_t);

    ST_NODE_CAST (node)->type = ST_NODE_ASSIGN;

    g_assert (st_node_type (assignee) == ST_NODE_IDENTIFIER);
    node->assignee = assignee;

    g_assert (st_node_is_variable (variable));
    node->variable = variable;

    return ST_NODE_CAST (node);
}


st_node_t *
st_node_literal_string_new (const char *string)
{
    st_node_literal_t *node;
    g_assert (string != NULL);

    node = g_slice_new (st_node_literal_t);

    ST_NODE_CAST (node)->type = ST_NODE_LITERAL_STRING;
    node->string = g_strdup (string);

    return ST_NODE_CAST (node);
}

st_node_t *
st_node_literal_symbol_new (const char *symbol)
{
    st_node_literal_t *node;
    g_assert (symbol != NULL);

    node = g_slice_new (st_node_literal_t);

    ST_NODE_CAST (node)->type = ST_NODE_LITERAL_SYMBOL;
    node->string = g_strdup (symbol);

    return ST_NODE_CAST (node);
}

st_node_t *
st_node_literal_integer_new (st_smi_t integer)
{
    st_node_literal_t *node;

    node = g_slice_new (st_node_literal_t);

    ST_NODE_CAST (node)->type = ST_NODE_LITERAL_INTEGER;
    node->integer = integer;

    return ST_NODE_CAST (node);
}

st_node_t *
st_node_literal_large_integer_new (mp_int *large_integer)
{
    st_node_literal_t *node;
    g_assert (large_integer != NULL);

    node = g_slice_new (st_node_literal_t);

    ST_NODE_CAST (node)->type = ST_NODE_LITERAL_LARGE_INTEGER;
    node->large_integer = *large_integer;

    return ST_NODE_CAST (node);
}

st_node_t *
st_node_literal_float_new (double floating)
{
    st_node_literal_t *node;

    node = g_slice_new (st_node_literal_t);

    ST_NODE_CAST (node)->type = ST_NODE_LITERAL_FLOATING;
    node->floating = floating;

    return ST_NODE_CAST (node);
}

st_node_t *
st_node_literal_character_new (gunichar character)
{
    st_node_literal_t *node;
    g_assert (g_unichar_validate (character));

    node = g_slice_new (st_node_literal_t);

    ST_NODE_CAST (node)->type = ST_NODE_LITERAL_CHARACTER;
    node->character = character;

    return ST_NODE_CAST (node);
}


st_node_t *
st_node_identifier_new (const char *name)
{
    st_node_identifier_t *node;

    node = g_slice_new (st_node_identifier_t);

    ST_NODE_CAST (node)->type = ST_NODE_IDENTIFIER;
    node->name = g_strdup (name);

    return ST_NODE_CAST (node);
}



st_node_t *
st_node_unary_message_new (const char *selector)
{
    st_node_unary_message_t *node;
    g_assert (selector != NULL);

    node = g_slice_new (st_node_unary_message_t);

    ST_NODE_CAST (node)->type = ST_NODE_MESSAGE_UNARY;
    node->selector = g_strdup (selector);

    return ST_NODE_CAST (node);
}


st_node_t *
st_node_binary_message_new (const char *selector,
			    st_node_t  *argument)
{
    st_node_binary_message_t *node;
    g_assert (selector != NULL);
    g_assert (argument != NULL);

    node = g_slice_new (st_node_binary_message_t);

    ST_NODE_CAST (node)->type = ST_NODE_MESSAGE_BINARY;

    node->selector = g_strdup (selector);

    g_assert (st_node_is_variable (argument));
    node->argument = argument;

    return ST_NODE_CAST (node);
}

st_node_t *
st_node_keyword_message_new (const char *selector,
			     GList      *arguments)
{
    st_node_keyword_message_t *node;
    g_assert (selector != NULL);
    g_assert (arguments != NULL);

    node = g_slice_new (st_node_keyword_message_t);

    ST_NODE_CAST (node)->type = ST_NODE_MESSAGE_KEYWORD;

    node->selector = g_strdup (selector);

    for (GList *l = arguments; l; l=l->next)
	g_assert (st_node_is_variable (DATA (l)));
    node->arguments = arguments;

    return ST_NODE_CAST (node);
}

void
st_print_node (st_node_t *node)
{
    switch (st_node_type (node)) {

    case ST_NODE_METHOD:
    {
	st_node_method_t *method_node = ST_NODE_METHOD_CAST (node);
	
	printf ("method:\n");
	printf ("  selector: %s\n", method_node->selector);
	    
	printf ("  args: ");
	for (GList *l = method_node->args; l; l = l->next)
	    printf ("%s ", (char *) l->data);
	
	printf ("\n");
	
	printf ("  temps: ");
	for (GList *l = method_node->temps; l; l = l->next)
	    printf ("%s ", (char *) l->data);
	
	printf ("\n");
	
	for (GList *l = method_node->statements; l; l = l->next) {
	    st_print_node (ST_NODE_CAST (l->data));
	    printf (".\n");
	}
	
	printf ("\n");
	
	break;
    }
    case ST_NODE_BLOCK:
    {
	st_node_block_t *block = ST_NODE_BLOCK_CAST (node);
	
	printf ("[");

	printf ("args: ");
	for (GList *l = block->args; l; l = l->next)
	    printf ("%s ", (char *) l->data);
	    
	printf ("temps: ");
	for (GList *l = block->temps; l; l = l->next)
	    printf ("%s ", (char *) l->data);
	
	printf ("| ");	

	for (GList *l = block->statements; l; l = l->next) {
	    st_print_node (ST_NODE_CAST (l->data));
	    printf (". ");
	}	

	printf ("]");
	

	break;       
    }
    case ST_NODE_EXPRESSION:
    {
	st_node_expression_t *expression = ST_NODE_EXPRESSION_CAST (node);
	
	printf ("(");
	
	st_print_node (expression->receiver);
	
	printf (" ");
	
	st_print_node (expression->message);
	
	printf (")");
	
	break;
    }
    case ST_NODE_RETURN:
	printf ("^ ");
	st_print_node (ST_NODE_RETURN_CAST (node)->variable);
	break;
    case ST_NODE_ASSIGN:
	printf ("(");
	st_print_node (ST_NODE_ASSIGN_CAST (node)->assignee);
	printf (" := ");
	st_print_node (ST_NODE_ASSIGN_CAST (node)->variable);
	printf (")");
	break;
    case ST_NODE_IDENTIFIER:
	printf (ST_NODE_IDENTIFIER_CAST (node)->name);
	break;
    case ST_NODE_MESSAGE_UNARY:
    {
	printf ("#%s", ST_NODE_UNARY_MESSAGE_CAST (node)->selector);
	break;
    }
    case ST_NODE_MESSAGE_BINARY:
    {
	printf ("#%s ", ST_NODE_BINARY_MESSAGE_CAST (node)->selector);
	st_print_node (ST_NODE_BINARY_MESSAGE_CAST (node)->argument);
	break;
    }
    case ST_NODE_MESSAGE_KEYWORD:
    {
	
	printf ("#%s ", ST_NODE_KEYWORD_MESSAGE_CAST (node)->selector);
	
	GList *arguments = ST_NODE_KEYWORD_MESSAGE_CAST (node)->arguments;
	for (GList *l = arguments; l; l = l->next) {
	    st_print_node (ST_NODE_CAST (l->data));
	    printf (" ");
	}
	break;
    }
    case ST_NODE_LITERAL_INTEGER:
	printf ("%li", ST_NODE_LITERAL_CAST (node)->integer);
	    break;
    case ST_NODE_LITERAL_LARGE_INTEGER:
    {
	char *string;
	int result, size;
	mp_int value = ST_NODE_LITERAL_CAST (node)->large_integer;
	
	result = mp_radix_size (&value, 10, &size);
	g_assert (result == MP_OKAY);
	
	g_debug ("%i", size);
	
	string = g_malloc (size);
	
	result = mp_toradix (&value, string, 10);
	g_assert (result == MP_OKAY);
	
	printf ("%s", string);
	break;
    }
    case ST_NODE_LITERAL_FLOATING:
	printf ("%f", ST_NODE_LITERAL_CAST (node)->floating);
	break;
    case ST_NODE_LITERAL_STRING:
	printf ("'%s'", ST_NODE_LITERAL_CAST (node)->string);
	break;
    case ST_NODE_LITERAL_SYMBOL:
	printf ("#%s", ST_NODE_LITERAL_CAST (node)->string);
	break;
    case ST_NODE_LITERAL_CHARACTER:
    {
	char out[6] = {0};
	g_unichar_to_utf8 (ST_NODE_LITERAL_CAST (node)->character, out);
	printf ("$%s", out);
	break;
    }
    default:
	// not handled yet
	break;
    }
}



