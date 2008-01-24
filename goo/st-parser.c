/* st-parser.c
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

#include "st-parser.h"
#include "st-types.h"
#include "st-lexer.h"
#include "st-utils.h"
#include "st-ast.h"

#include <tommath.h>
#include <glib.h>
#include <errno.h>
#include <stdlib.h>   

static void
parse_error (const char  *message,
	     st_token_t  *token)
{
    printf ("error:%i: %s\n", st_token_line (token), message);
    abort ();
}

/* adaptor for st_lexer_next_token() so we can catch lexer errors and strip comments
 */
static st_token_t *
next (st_lexer_t *lexer)
{
    st_token_t     *token;
    st_token_type_t type;

    token = st_lexer_next_token (lexer);
    type  = st_token_type (token);

    if (type == ST_TOKEN_COMMENT)
	return next (lexer);
    else if (type == ST_TOKEN_INVALID)
	parse_error (st_lexer_error_message (lexer), token);	
	
    return token;
}

static st_token_t *
current (st_lexer_t *lexer)
{
    return st_lexer_current_token (lexer);
}

static GList      *parse_statements (st_lexer_t *lexer, bool in_block);
static GList      *parse_temporaries (st_lexer_t *lexer);
static st_node_t  *parse_subexpression (st_lexer_t *lexer);


static GList *
parse_block_arguments (st_lexer_t *lexer)
{
    st_token_t     *token;   
    GList          *args = NULL;

    token = current (lexer);

    while (st_token_type (token) == ST_TOKEN_COLON) {
	
	token = next (lexer);
	if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error ("expected identifier", token);
	
	args = g_list_prepend (args, g_strdup (st_token_text (token)));
	token = next (lexer);
    }
    
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR ||
	!streq (st_token_text (token), "|"))
	parse_error ("expected ':' or '|'", token);

    next (lexer);

    return g_list_reverse (args);
}


static st_node_t *
parse_block (st_lexer_t *lexer)
{
    st_token_t *token;
    GList *args = NULL, *temps = NULL;
    GList *statements = NULL; 
    
    // parse block arguments
    token = next (lexer);
    
    if (st_token_type (token) == ST_TOKEN_COLON)
	args = parse_block_arguments (lexer);
    
    token = current (lexer);
    if (st_token_type (token) == ST_TOKEN_BINARY_SELECTOR
	&& streq (st_token_text (token), "|"))
	temps = parse_temporaries (lexer);
    
    statements = parse_statements (lexer, true);

    token = current (lexer);
    if (st_token_type (token) != ST_TOKEN_BLOCK_END)
	parse_error ("expected ']'", token);
    next (lexer);

    g_debug ("foo\n");
    

    return st_node_block_new (args, temps, statements);
}


static st_node_t *
parse_number (st_lexer_t *lexer)
{    
    int radix;
    int exponent;   
    char *number, *p;
    st_node_t *node;
    st_number_token_t *token;

    token = ST_NUMBER_TOKEN (current (lexer));
    
    radix    = st_number_token_radix (token);
    exponent = st_number_token_exponent (token);
    
    p = number = st_token_text ((st_token_t *) (token));

    /* check if there is a decimal point */
    while (*p && *p != '.')
	++p;
	
    if (*p == '.' || exponent != 0) {

	if (radix != 10)
	    parse_error ("only base-10 floats are supported at the moment",(st_token_t *) token);
    
	char *format = g_strdup_printf ("%se%i", number, exponent);
	
	double floating = strtod (format, NULL);
	g_free (format);
	node = st_node_literal_float_new (floating);

    } else {

	/* right, we use strtoll() when smi's can contain 61-bit integers. This is the
	 * case for 64-bit systems. 
	 */
	 
#if ST_HOST32 == 1
	long int integer;
	integer = strtol (number, NULL, radix);
#else
	long long int integer;
	integer = strtol (number, NULL, radix);
#endif
	/* check for overflow */
	if ((errno == ERANGE) || (integer < ST_SMALL_INTEGER_MIN) || (integer > ST_SMALL_INTEGER_MAX)) {
	
	    mp_int value;
	    int result;
	    
	    result = mp_init (&value);
	    if (result != MP_OKAY)
		parse_error ("memory exhausted while trying parse LargeInteger", (st_token_t *) token);
	
	    result = mp_read_radix (&value, number, radix);
	    if (result != MP_OKAY)
		parse_error ("memory exhausted while trying parse LargeInteger", (st_token_t *) token);
	    
	    node = st_node_literal_large_integer_new (&value);
	    
	} else {
	    node = st_node_literal_integer_new (integer);
	}
    }

    next (lexer);

    return node;
}

static st_node_t *parse_expression (st_lexer_t *lexer);


/* identifiers, literals, blocks */
static st_node_t *
parse_primary (st_lexer_t *lexer)
{
    st_node_t *node;
    st_token_t *token;
    
    token = current (lexer);
    
    switch (st_token_type (token)) {
    
    case ST_TOKEN_IDENTIFIER:
    
	node = st_node_identifier_new (st_token_text (token));
	next (lexer);
	break;

    case ST_TOKEN_NUMBER_CONST:
 
    	node = parse_number (lexer);
	break;
	
    case ST_TOKEN_STRING_CONST: 
    
	node = st_node_literal_string_new (st_token_text (token));
	next (lexer);
	break;

    case ST_TOKEN_SYMBOL_CONST:
    
	node = st_node_literal_symbol_new (st_token_text (token));
	next (lexer);
	break;

    case ST_TOKEN_CHARACTER_CONST:

	node = st_node_literal_character_new (g_utf8_get_char (st_token_text (token)));
	next (lexer);
	break;

    case ST_TOKEN_BLOCK_BEGIN:
    
	node = parse_block (lexer);
	break;

    default:
	g_assert_not_reached ();
    }

    return node;
}

static st_node_t *
parse_unary_message (st_lexer_t *lexer)
{
    st_token_t *token;
    char *selector;
    
    token = current (lexer);

    selector = st_token_text (token);

    next (lexer);

    return st_node_unary_message_new (selector);
}

static st_node_t *
parse_binary_argument (st_lexer_t *lexer, st_node_t *argument)
{
    st_node_t  *expression;
    st_token_t *token;

    token = current (lexer);
    if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	return argument;
    
    expression = st_node_expression_new (argument, parse_unary_message (lexer));

    return parse_binary_argument (lexer, expression);
}

static st_node_t *
parse_binary_message (st_lexer_t *lexer)
{
    st_node_t  *argument;
    st_token_t *token;
    char       *selector;

    token = current (lexer);
    
    selector = st_token_text (token);

    /* parse the primary */
    token = next (lexer);
    if (st_token_type (token) == ST_TOKEN_LPAREN)
        argument = parse_subexpression (lexer);
    else
	argument = parse_primary (lexer);
     
    argument = parse_binary_argument (lexer, argument);
    
    return st_node_binary_message_new (selector, argument);
}


static st_node_t *
parse_keyword_argument (st_lexer_t *lexer, st_node_t *receiver)
{
    st_node_t  *msg;
    st_token_t *token;

    token = current (lexer);
   
    if (receiver == NULL) {
	/* parse the primary */
	if (st_token_type (token) == ST_TOKEN_LPAREN)
	    receiver = parse_subexpression (lexer);
	else
	    receiver = parse_primary (lexer);

    } else if (st_token_type (token) == ST_TOKEN_IDENTIFIER) {
	msg = parse_unary_message (lexer);
	receiver = st_node_expression_new (receiver, msg);    	
	
    } else if (st_token_type (token) == ST_TOKEN_BINARY_SELECTOR) {
	msg = parse_binary_message (lexer);
	receiver = st_node_expression_new (receiver, msg);
	
    } else {
	return receiver;
    }
    
    return parse_keyword_argument (lexer, receiver);
}

static st_node_t *
parse_keyword_message (st_lexer_t *lexer)
{
    st_token_t *token;
    st_node_t  *argument;
    GList      *args = NULL;
    GString    *selector;
    
    selector = g_string_new (NULL);

    token = current (lexer);
    
    while (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR) {
	
	g_string_append (selector, st_token_text (token));
	token = next (lexer);
	
	argument = parse_keyword_argument (lexer, NULL);
	args = g_list_append (args, argument);	

	token = current (lexer);
    }
    
    return st_node_keyword_message_new (g_string_free (selector, false), args);
}

/*
 * parses an expression from left to right, by recursively parsing subexpressions
 */
static st_node_t *
parse_message (st_lexer_t *lexer, st_node_t *receiver)
{
    st_node_t  *msg = NULL;
    st_token_t *token;
    st_token_type_t type;

    /* Before parsing messages, check if expression is simply a variable.
     * This is the case if token is ')' or '.'
     */    
    token = current (lexer);
    type = st_token_type (token);
    
    if (type == ST_TOKEN_PERIOD || type == ST_TOKEN_RPAREN
	|| type == ST_TOKEN_EOF || type == ST_TOKEN_BLOCK_END)
	return receiver;		      

    if (type == ST_TOKEN_IDENTIFIER)
	msg = parse_unary_message (lexer);

    else if (type == ST_TOKEN_BINARY_SELECTOR)
	msg = parse_binary_message (lexer);

    else if (type == ST_TOKEN_KEYWORD_SELECTOR)
	msg = parse_keyword_message (lexer);

    else
	parse_error ("token not expected", token);

    return parse_message (lexer, st_node_expression_new (receiver, msg));
}
    
static st_node_t *
parse_assign (st_lexer_t *lexer, st_node_t *assignee)
{
    st_node_t  *variable;
    
    next (lexer);
    
    variable = parse_expression (lexer);
	
    return st_node_assign_new (assignee, variable);
}
    
 
static st_node_t *
parse_expression (st_lexer_t *lexer)
{
    st_node_t   *receiver = NULL;
    st_token_t  *token;
    
    token = current (lexer);

    switch (st_token_type (token)) {
    case ST_TOKEN_NUMBER_CONST:
    case ST_TOKEN_STRING_CONST:
    case ST_TOKEN_SYMBOL_CONST:
    case ST_TOKEN_CHARACTER_CONST:    
    case ST_TOKEN_BLOCK_BEGIN:

	receiver = parse_primary (lexer);
	break;

    case ST_TOKEN_IDENTIFIER:

	receiver = parse_primary (lexer);

	if (st_token_type (current (lexer)) == ST_TOKEN_ASSIGN)
	    return parse_assign (lexer, receiver);       

	break;

    case ST_TOKEN_LPAREN:

	receiver = parse_subexpression (lexer);
	break;
	
    default:
	g_debug ("%i", st_token_type (token));
	parse_error ("expected expression", token);
    }

    return parse_message (lexer, receiver);
}

static st_node_t *
parse_subexpression (st_lexer_t *lexer)
{
    st_token_t *token;
    st_node_t *expression;
    
    next (lexer);
    
    expression = parse_expression (lexer);
    
    token = current (lexer);
    if (st_token_type (token) != ST_TOKEN_RPAREN)
	parse_error ("expected ')' after expression", token);    
	
    next (lexer);

    return expression;
}

static st_node_t *
parse_return (st_lexer_t *lexer)
{   
    next (lexer);

    return st_node_return_new (parse_expression (lexer));
}

static st_node_t *
parse_statement (st_lexer_t *lexer)
{
    st_token_t *token;
    
    token = current (lexer);

    if (st_token_type (token) != ST_TOKEN_RETURN)
	return parse_expression (lexer);
    else
	return parse_return (lexer);
}

static GList *
parse_statements (st_lexer_t *lexer, bool in_block)
{
    st_token_t *token;
    GList      *statements = NULL;
    st_node_t  *node;

    token = current (lexer);

    if (in_block)
	while (st_token_type (token) != ST_TOKEN_EOF
	       && st_token_type (token) != ST_TOKEN_BLOCK_END) {

	    node = parse_statement (lexer);
	    statements = g_list_append (statements, node);
	    
	    if (st_token_type (current (lexer)) == ST_TOKEN_BLOCK_END)
		break;
	    token = next (lexer);
	}
    else
	while (st_token_type (token) != ST_TOKEN_EOF) {
	    
	    node = parse_statement (lexer);
	
	    statements = g_list_append (statements, node);
	    token = next (lexer);
	}
  	
    return statements;
}

/*
 * '|' identifier* '|'
 */
static GList *
parse_temporaries (st_lexer_t *lexer)
{
    st_token_t *token;
    GList *temps = NULL;

    token = current (lexer);
    
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_text (token), "|")) 
	return NULL;
   
    token = next (lexer);
     
    while (st_token_type (token) == ST_TOKEN_IDENTIFIER) {
	temps = g_list_prepend (temps, g_strdup (st_token_text (token)));	   
	token = next (lexer);	       
    }
    
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_text (token), "|"))
	parse_error ("temporaries declaration not terminated by '|'", token);
    
    next (lexer);
   
    return g_list_reverse (temps);
}


typedef struct
{
    char  *selector;
    GList *args;
} message_pattern_t;

static message_pattern_t
parse_message_pattern (st_lexer_t *lexer)
{    
    st_token_t     *token;
    st_token_type_t type;
    char           *selector = NULL;
    GList          *args = NULL;
    
    token = next (lexer);    
    type = st_token_type (token);

    if (type == ST_TOKEN_IDENTIFIER) {
	
	selector = g_strdup (st_token_text (token));
	args = NULL;
	
	next (lexer);
    
    } else if (type == ST_TOKEN_BINARY_SELECTOR) {
	
	selector = g_strdup (st_token_text (token));

	token = next (lexer);

	if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error ("argument name expected after binary selector", token);

	args = g_list_append (args, g_strdup (st_token_text (token)));

	next (lexer);	
    
    } else if (type == ST_TOKEN_KEYWORD_SELECTOR) {
    
	GString *gstring = g_string_new (NULL);
	
	while (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR) {	
	    g_string_append (gstring, st_token_text (token));
	    token = next (lexer);

	    if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
		parse_error ("argument name expected after keyword", token);	
	    args = g_list_append (args, g_strdup (st_token_text (token)));
	    token = next (lexer);
	} 
	
	selector = g_string_free (gstring, FALSE);
    
    } else {
	parse_error ("invalid message pattern", token);
    }

    // yeah, look at this C99 goodness
    return (message_pattern_t) { .selector = selector, .args = args };
}

static st_node_t *
parse_method (st_lexer_t *lexer)
{
    GList *temps = NULL;
    GList *statements = NULL;
    message_pattern_t pattern;

    pattern     = parse_message_pattern (lexer);
    temps       = parse_temporaries (lexer);
    statements  = parse_statements (lexer, false);
    
    return st_node_method_new (pattern.selector, pattern.args, temps, statements);
} 

st_node_method_t *
st_parser_parse (st_lexer_t *lexer)
{   
    g_assert (lexer != NULL);

    return (st_node_method_t *) parse_method (lexer);
}

