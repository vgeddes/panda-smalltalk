/*
 * st-parser.c
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


struct st_parser_t
{
    st_lexer_t *lexer;
   
};

static void
parse_error (st_parser_t *parser,
	     const char  *message,
	     st_token_t  *token)
{
    printf ("error:%i: %s\n", st_token_line (token), message);
    exit (1);
}

/* adaptor for st_lexer_next_token() so we can catch lexer errors and strip comments
 */
static st_token_t *
next_token (st_parser_t *parser)
{
    st_token_t *token;
    
    token = st_lexer_next_token (parser->lexer);
    
    if (st_token_type (token) == ST_TOKEN_INVALID)
	parse_error (parser , st_lexer_error_message (parser->lexer), token);
	
    else if (st_token_type (token) == ST_TOKEN_COMMENT)
	return next_token (parser);
	
    return token;
}


static st_node_t *
parse_number (st_parser_t *parser)
{    
    int radix;
    int exponent;   
    char *number, *p;
    st_node_t *node;
    st_number_token_t *token;
    
    token = ST_NUMBER_TOKEN (st_lexer_current_token (parser->lexer));
    
    radix    = st_number_token_radix (token);
    exponent = st_number_token_exponent (token);
    
    p = number = st_token_text ((st_token_t *) (token)); 
    
    /* check if there is a decimal point */
    while (*p && *p != '.')
	++p;
	
    if (*p == '.' || exponent != 0) {
    
	if (radix != 10)
	    parse_error (parser, "only base-10 floats are supported at the moment",(st_token_t *) token);
    
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
		parse_error (parser, "memory exhausted while trying parse LargeInteger", (st_token_t *) token);
	
	    result = mp_read_radix (&value, number, radix);
	    if (result != MP_OKAY)
		parse_error (parser, "memory exhausted while trying parse LargeInteger", (st_token_t *) token);
	    
	    node = st_node_literal_large_integer_new (&value);
	    
	} else {
	    node = st_node_literal_integer_new (integer);
	}
    }
    return node;
}

static st_node_t *
parse_tuple (st_parser_t *parser);

static st_node_t *
parse_expression (st_parser_t *parser);


/* identifiers, literals, tuples, blocks */
static st_node_t *
parse_primary (st_parser_t *parser)
{
    st_token_t *token;
    
    token = st_lexer_current_token (parser->lexer);
    
    switch (st_token_type (token)) {
    
    case ST_TOKEN_IDENTIFIER:
    
	return st_node_identifier_new (st_token_text (token));
    
    case ST_TOKEN_TUPLE_BEGIN:
 
	return parse_tuple (parser);
	
    case ST_TOKEN_NUMBER_CONST:
 
    	return parse_number (parser);
    
    case ST_TOKEN_STRING_CONST: 
    
	return st_node_literal_string_new (st_token_text (token));
	
    case ST_TOKEN_SYMBOL_CONST:
    
	return st_node_literal_symbol_new (st_token_text (token));
    
    case ST_TOKEN_CHARACTER_CONST:

	return st_node_literal_character_new (g_utf8_get_char (st_token_text (token)));

    case ST_TOKEN_BLOCK_BEGIN:
    
	//return parse_block (parser);
	return NULL;

    default:
	parse_error (parser, "expected variable", token);
	return NULL;
    }
}

static st_node_t *
parse_tuple (st_parser_t *parser)
{
    st_node_t  *node = NULL;
    st_token_t *token;
    GList *elements = NULL;

    token = next_token (parser);
    
    while (true) {

	switch (st_token_type (token)) {
    
	case ST_TOKEN_IDENTIFIER:
	case ST_TOKEN_NUMBER_CONST:
	case ST_TOKEN_STRING_CONST:
	case ST_TOKEN_SYMBOL_CONST:
	case ST_TOKEN_CHARACTER_CONST:
	    node = parse_primary (parser);
	    break;
    
	case ST_TOKEN_LPAREN:
	    node = parse_tuple (parser);
	    break;
	    
	case ST_TOKEN_RPAREN:
	    break;
    
	default:
	    parse_error (parser, "expected ')'", token);
	}
	
	if (st_token_type (token) == ST_TOKEN_RPAREN)
	    break;
	
	elements = g_list_append (elements, node);
	token = next_token (parser);
    }
    
    return st_node_tuple_new (elements);
}

static st_node_t *
parse_unary_message (st_parser_t *parser)
{
    st_token_t *token;

    token = st_lexer_current_token (parser->lexer);

    char *selector = st_token_text (token);

    next_token (parser);

    return st_node_unary_message_new (selector);
}

static st_node_t *
parse_binary_argument (st_parser_t *parser, st_node_t *argument)
{
    st_node_t  *msg, *expression;
    st_token_t *token;

    token = st_lexer_current_token (parser->lexer);
    if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	return argument;

    msg        = parse_unary_message (parser);
    expression = st_node_expression_new (argument, msg);
    
    return parse_binary_argument (parser, expression);
}

static st_node_t *
parse_binary_message (st_parser_t *parser)
{
    st_node_t  *argument;
    st_token_t *token;
    char       *selector;

    token = st_lexer_current_token (parser->lexer);
    
    selector = st_token_text (token);

    /* parse the primary */
    token = next_token (parser);
    if (st_token_type (token) == ST_TOKEN_LPAREN) {

	next_token (parser);
    
        argument = parse_expression (parser);
    
        token = st_lexer_current_token (parser->lexer);
        if (st_token_type (token) != ST_TOKEN_RPAREN)
	    parse_error (parser, "expected ')' after expression", token);
	    
    } else {
       argument = parse_primary (parser);
    }
    
    /* parse any unary messages to the argument 
     */
    next_token (parser);
     
    argument = parse_binary_argument (parser, argument);
    
    return st_node_binary_message_new (selector, argument);
}


static st_node_t *
parse_keyword_argument (st_parser_t *parser, st_node_t *receiver)
{
    st_node_t  *msg;
    st_token_t *token;

    token = st_lexer_current_token (parser->lexer);
   
    if (receiver == NULL) {
	/* parse the primary */
	if (st_token_type (token) == ST_TOKEN_LPAREN) {

	    next_token (parser);
    
	    receiver = parse_expression (parser);

	    token = st_lexer_current_token (parser->lexer);
	    if (st_token_type (token) != ST_TOKEN_RPAREN)
		parse_error (parser, "expected ')' after expression", token);
	    
	} else {
	    receiver = parse_primary (parser);
	}
	
	next_token (parser);
	
    } else if (st_token_type (token) == ST_TOKEN_IDENTIFIER) {
	msg = parse_unary_message (parser);
	receiver = st_node_expression_new (receiver, msg);    	
	
    } else if (st_token_type (token) == ST_TOKEN_BINARY_SELECTOR) {
	msg = parse_binary_message (parser);
	receiver = st_node_expression_new (receiver, msg);
	
    } else {
	return receiver;
    }
    
    return parse_keyword_argument (parser, receiver);
}

static st_node_t *
parse_keywords (st_parser_t *parser, GString *selector, GList *args)
{
    st_token_t *token;
    st_node_t  *argument;

    token = st_lexer_current_token (parser->lexer);
    if (st_token_type (token) != ST_TOKEN_KEYWORD_SELECTOR)
	return st_node_keyword_message_new (selector->str, args);

    g_string_append (selector, st_token_text (token));

    next_token (parser);
    
    argument = parse_keyword_argument (parser, NULL);
    
    args = g_list_append (args, argument);

    return parse_keywords (parser, selector, args);
}

static st_node_t *
parse_keyword_message (st_parser_t *parser)
{
    st_node_t *message;
    GString   *selector; 
    
    selector = g_string_new (NULL);

    message = parse_keywords (parser, selector, NULL);
    
    g_string_free (selector, FALSE);

    return message;
}

static st_node_t *
parse_message_continuation (st_parser_t *parser, st_node_t *receiver)
{
    st_node_t *expression, *msg;
    st_token_t *token;

    /* Before parsing messages, check if expression is simply a variable.
     * This is the case if token is ')' or '.'
     */    
    token = st_lexer_current_token (parser->lexer);
    if (st_token_type (token) == ST_TOKEN_PERIOD || st_token_type (token) == ST_TOKEN_RPAREN)
	return receiver;

    if (st_token_type (token) == ST_TOKEN_IDENTIFIER)
	msg = parse_unary_message (parser);
	
    else if (st_token_type (token) == ST_TOKEN_BINARY_SELECTOR)
	msg = parse_binary_message (parser);
	
    else if (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR)
	msg = parse_keyword_message (parser);
    else
	g_assert_not_reached ();

    expression = st_node_expression_new (receiver, msg);

    return parse_message_continuation (parser, expression);
}

static st_node_t *
parse_expression (st_parser_t *parser)
{
    st_node_t   *node = NULL;
    st_token_t  *token;
    
    token = st_lexer_current_token (parser->lexer);

    switch (st_token_type (token)) {
     
    case ST_TOKEN_IDENTIFIER:
    case ST_TOKEN_TUPLE_BEGIN:
    case ST_TOKEN_NUMBER_CONST:
    case ST_TOKEN_STRING_CONST:
    case ST_TOKEN_SYMBOL_CONST:
    case ST_TOKEN_CHARACTER_CONST:    
    case ST_TOKEN_BLOCK_BEGIN:
    
	node = parse_primary (parser);
	next_token (parser);
	break;

    case ST_TOKEN_LPAREN:
    
	next_token (parser);
	node = parse_expression (parser);
	
	token = st_lexer_current_token (parser->lexer);
	if (st_token_type (token) != ST_TOKEN_RPAREN)
	    parse_error (parser, "expected ')' after expression", token);
	next_token (parser);
	break;
	
    default:
	printf ("%i\n", st_token_type (token));
	parse_error (parser, "expected expression", token);
    }

    return parse_message_continuation (parser, node);
}

static st_node_t *
parse_return (st_parser_t *parser)
{
    st_token_t  *token;
    
    token = st_lexer_current_token (parser->lexer);
    
    if (st_token_type (token) != ST_TOKEN_RETURN)
	parse_error (parser, "expected return statement", token);
    
    next_token (parser);
    
    st_node_t *variable = parse_expression (parser);

    return st_node_return_new (variable);
}

static st_node_t *
parse_statement (st_parser_t *parser)
{
    st_token_t *token;
    
    token = st_lexer_current_token (parser->lexer);

    if (st_token_type (token) != ST_TOKEN_RETURN) {
	return parse_expression (parser);
    } else {
	return parse_return (parser);
    } 
}

static GList *
parse_statements (st_parser_t *p)
{
    st_token_t     *token;
    GList          *statements = NULL;
 
    token = st_lexer_current_token (p->lexer);

    while (st_token_type (token) != ST_TOKEN_EOF) {
    
	st_node_t *node;
    
	node = parse_statement (p);
	
	statements = g_list_append (statements, node);

	token = next_token (p);
    }
  	
    return statements;
}


/*
 * '|' identifier* '|'
 */
static GList *
parse_temporaries (st_parser_t *p)
{
    st_token_t *token;
    GList *temps = NULL;

    token = st_lexer_current_token (p->lexer);
    
    if (st_token_type (token) == ST_TOKEN_BINARY_SELECTOR && streq (st_token_text (token), "|")) {
     
	token = next_token (p);
     
	while (st_token_type (token) == ST_TOKEN_IDENTIFIER) {
	    temps = g_list_append (temps, g_strdup (st_token_text (token)));	   
	    token = next_token (p);	       
        }
        
        if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR || !streq (st_token_text (token), "|"))
	    parse_error (p, "temporaries declaration not terminated by '|'",token);

	next_token (p);
    }
   
    return temps;
}


/*
 * (unary-selector | binary-selector argument | (keyword argument)* )+
 */
static void
parse_message_pattern (st_parser_t *p, char **selector, GList **args)
{    
    st_token_t     *token;
    st_token_type_t type;
    
    token = next_token (p);
    
    type = st_token_type (token);
    
    if (type == ST_TOKEN_IDENTIFIER) {
	
	*selector = g_strdup (st_token_text (token));
	*args = NULL;
	
	next_token (p);
    
    } else if (type == ST_TOKEN_BINARY_SELECTOR) {
	
	*selector = g_strdup (st_token_text (token));

	token = next_token (p);

	if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error (p, "identifier expected after binary selector", token);

	*args = g_list_append (*args, g_strdup (st_token_text (token)));

	next_token (p);	
    
    } else if (type == ST_TOKEN_KEYWORD_SELECTOR) {
    
	GString *gstring = g_string_new (NULL);
	
	while (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR) {	
	
	    g_string_append (gstring, st_token_text (token));
	    
	    if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
		parse_error (p, "identifier expected after keyword", token);	
	    *args = g_list_append (*args, g_strdup (st_token_text (token)));	
	
	    token = next_token (p);
	} 
	
	*selector = g_string_free (gstring, FALSE);
    
    } else {
	parse_error (p, "invalid message pattern", token);
    }
}

static st_node_t *
parse_method (st_parser_t *parser)
{
    char *selector    = NULL;
    GList *args       = NULL;
    GList *temps      = NULL;
    GList *statements = NULL;

    parse_message_pattern (parser, &selector, &args);
    temps       = parse_temporaries (parser);
    statements  = parse_statements (parser);
    
    return st_node_method_new (selector, args, temps, statements);
} 

st_node_method_t *
st_parser_parse (const char *source)
{
    st_parser_t *parser = g_slice_new (st_parser_t);
    
    parser->lexer = st_lexer_new (source);
    
    return (st_node_method_t *) parse_method (parser);
}

