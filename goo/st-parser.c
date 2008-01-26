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
#include "st-byte-array.h"
#include "st-symbol.h"
#include "st-float.h"
#include "st-large-integer.h"
#include "st-ast.h"

#include <tommath.h>
#include <glib.h>
#include <errno.h>
#include <stdlib.h>   

static void
parse_error (const char  *message,
	     STToken  *token)
{
    printf ("error:%i: %s\n", st_token_line (token), message);
    abort ();
}

/* adaptor for st_lexer_next_token() so we can catch lexer errors and strip comments
 */
static STToken *
next (STLexer *lexer)
{
    STToken     *token;
    STTokenType type;

    token = st_lexer_next_token (lexer);
    type  = st_token_type (token);

    if (type == ST_TOKEN_COMMENT)
	return next (lexer);
    else if (type == ST_TOKEN_INVALID)
	parse_error (st_lexer_error_message (lexer), token);	
	
    return token;
}

static STToken *
current (STLexer *lexer)
{
    return st_lexer_current_token (lexer);
}

static STNode  *parse_statements (STLexer *lexer);
static STNode  *parse_temporaries (STLexer *lexer);
static STNode  *parse_subexpression (STLexer *lexer);


static STNode *
parse_block_arguments (STLexer *lexer)
{
    STToken *token;   
    STNode  *arguments = NULL, *arg;

    token = current (lexer);

    while (st_token_type (token) == ST_TOKEN_COLON) {
	
	token = next (lexer);
	if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error ("expected identifier", token);

	arg = st_node_new (ST_VARIABLE_NODE);
	arg->name = st_symbol_new (st_token_text (token));
	arguments = st_node_append (arguments, arg);

	token = next (lexer);
    }
    
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR ||
	!streq (st_token_text (token), "|"))
	parse_error ("expected ':' or '|'", token);

    next (lexer);

    return arguments;
}


static STNode *
parse_block (STLexer *lexer)
{
    STToken *token;
    STNode  *block;

    block = st_node_new (ST_BLOCK_NODE);
    
    // parse block arguments
    token = next (lexer);
    
    if (st_token_type (token) == ST_TOKEN_COLON)
	block->arguments = parse_block_arguments (lexer);
    
    token = current (lexer);
    if (st_token_type (token) == ST_TOKEN_BINARY_SELECTOR
	&& streq (st_token_text (token), "|"))
	block->temporaries = parse_temporaries (lexer);
    
    block->statements = parse_statements (lexer);

    token = current (lexer);
    if (st_token_type (token) != ST_TOKEN_BLOCK_END)
	parse_error ("expected ']'", token);
    next (lexer);

    return block;
}

static STNode *
parse_number (STLexer *lexer)
{    
    int radix;
    int exponent;   
    char *number, *p;
    STNode *node;
    STNumberToken *token;

    token = ST_NUMBER_TOKEN (current (lexer));
    
    radix    = st_number_token_radix (token);
    exponent = st_number_token_exponent (token);
    
    p = number = st_token_text ((STToken *) (token));

    node = st_node_new (ST_LITERAL_NODE);

    /* check if there is a decimal point */
    while (*p && *p != '.')
	++p;
	
    if (*p == '.' || exponent != 0) {
	
	char *format;
	
	if (radix != 10)
	    parse_error ("only base-10 floats are supported at the moment",(STToken *) token);
    
	format = g_strdup_printf ("%se%i", number, exponent);
	
	node->literal = st_float_new (strtod (format, NULL));
	
	g_free (format);

    } else {

	/* right, we use strtoll() when smi's can contain 61-bit integers. This is the
	 * case for 64-bit systems.
	 */
#if ST_HOST32 == 1
	long int integer = strtol (number, NULL, radix);
#else
	long long int integer = strtoll (number, NULL, radix);
#endif
	/* check for overflow */
	if (errno == ERANGE
	    || integer < ST_SMALL_INTEGER_MIN || integer > ST_SMALL_INTEGER_MAX) {
	
	    mp_int value;
	    int result;
	    
	    result = mp_init (&value);
	    if (result != MP_OKAY)
		parse_error ("memory exhausted while trying parse LargeInteger", (STToken *) token);
	
	    result = mp_read_radix (&value, number, radix);
	    if (result != MP_OKAY)
		parse_error ("memory exhausted while trying parse LargeInteger", (STToken *) token);
	
	    node->literal = st_large_integer_new (&value);
    
	} else {
	    node->literal = st_smi_new (integer);
	}
    }

    next (lexer);

    return node;
}

static STNode *parse_expression (STLexer *lexer);


/* identifiers, literals, blocks */
static STNode *
parse_primary (STLexer *lexer)
{
    STNode *node;
    STToken *token;
    
    token = current (lexer);
    
    switch (st_token_type (token)) {
	
    case ST_TOKEN_IDENTIFIER:
	
	node = st_node_new (ST_VARIABLE_NODE);
	node->name = st_symbol_new (st_token_text (token));

	next (lexer);
	break;

    case ST_TOKEN_NUMBER_CONST:
 
    	node = parse_number (lexer);
	break;
	
    case ST_TOKEN_STRING_CONST: 
    
	node = st_node_new (ST_LITERAL_NODE);
	node->literal = st_string_new (st_token_text (token));

	next (lexer);
	break;

    case ST_TOKEN_SYMBOL_CONST:

	node = st_node_new (ST_LITERAL_NODE);
	node->literal = st_symbol_new (st_token_text (token));
    
	next (lexer);
	break;

    case ST_TOKEN_CHARACTER_CONST:

	node = st_node_new (ST_LITERAL_NODE);
	node->literal = st_character_new (g_utf8_get_char (st_token_text (token)));

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

static STNode *
parse_unary_message (STLexer *lexer, STNode *receiver)
{
    STToken *token;
    STNode  *message;
    
    token = current (lexer);

    message = st_node_new (ST_MESSAGE_NODE);
    message->precedence = ST_UNARY_PRECEDENCE;
    message->receiver = receiver;
    message->selector = st_node_new (ST_SELECTOR_NODE);
    message->selector->name = st_symbol_new (st_token_text (token));
    message->arguments = NULL;

    next (lexer);

    return message;
}

static STNode *
parse_binary_argument (STLexer *lexer, STNode *receiver)
{
    STNode  *message;
    STToken *token;

    token = current (lexer);
    if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	return receiver;
   
    message = parse_unary_message (lexer, receiver);

    return parse_binary_argument (lexer, message);
}

static STNode *
parse_binary_message (STLexer *lexer, STNode *receiver)
{
    STNode  *message, *argument;
    STToken *token;
    char     *selector;

    token = current (lexer);
    
    selector = st_token_text (token);

    /* parse the primary */
    token = next (lexer);
    if (st_token_type (token) == ST_TOKEN_LPAREN)
        argument = parse_subexpression (lexer);
    else
	argument = parse_primary (lexer);
     
    argument = parse_binary_argument (lexer, argument);
   
    message = st_node_new (ST_MESSAGE_NODE);

    message->precedence = ST_BINARY_PRECEDENCE;
    message->receiver = receiver;
    message->selector = st_node_new (ST_SELECTOR_NODE);
    message->selector->name = st_symbol_new (selector);
    message->arguments = argument;

    return message;
}


static STNode *
parse_keyword_argument (STLexer *lexer, STNode *receiver)
{
    STToken *token;

    token = current (lexer);
   
    if (receiver == NULL) {
	/* parse the primary */
	if (st_token_type (token) == ST_TOKEN_LPAREN)
	    receiver = parse_subexpression (lexer);
	else
	    receiver = parse_primary (lexer);

    } else if (st_token_type (token) == ST_TOKEN_IDENTIFIER) {
	receiver = parse_unary_message (lexer, receiver);
	
    } else if (st_token_type (token) == ST_TOKEN_BINARY_SELECTOR) {
	receiver = parse_binary_message (lexer, receiver);
	
    } else {
	return receiver;
    }

    return parse_keyword_argument (lexer, receiver);
}

static STNode *
parse_keyword_message (STLexer *lexer, STNode *receiver)
{
    STToken *token;
    STNode  *message, *arguments = NULL, *arg;
    GString  *selector;
    
    selector = g_string_new (NULL);
    token = current (lexer);
    
    while (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR) {
	
	g_string_append (selector, st_token_text (token));
	token = next (lexer);
	
	arg = parse_keyword_argument (lexer, NULL);
	arguments = st_node_append (arguments, arg);

	token = current (lexer);
    }
    
    message = st_node_new (ST_MESSAGE_NODE);

    message->precedence = ST_KEYWORD_PRECEDENCE;
    message->receiver = receiver;
    message->selector = st_node_new (ST_SELECTOR_NODE);
    message->selector->name = st_symbol_new (g_string_free (selector, false));
    message->arguments = arguments;

    return message;
}

/*
 * parses an expression from left to right, by recursively parsing subexpressions
 */
static STNode *
parse_message (STLexer *lexer, STNode *receiver)
{
    STNode  *message = NULL;
    STToken *token;
    STTokenType type;

    /* Before parsing messages, check if expression is simply a variable.
     * This is the case if token is ')' or '.'
     */    
    token = current (lexer);
    type = st_token_type (token);
    
    if (type == ST_TOKEN_PERIOD || type == ST_TOKEN_RPAREN
	|| type == ST_TOKEN_EOF || type == ST_TOKEN_BLOCK_END)
	return receiver;		      

    if (type == ST_TOKEN_IDENTIFIER)
	message = parse_unary_message (lexer, receiver);

    else if (type == ST_TOKEN_BINARY_SELECTOR)
	message = parse_binary_message (lexer, receiver);

    else if (type == ST_TOKEN_KEYWORD_SELECTOR)
	message = parse_keyword_message (lexer, receiver);

    else
	parse_error ("nothing more expected", token);

    return parse_message (lexer, message);
}
    
static STNode *
parse_assign (STLexer *lexer, STNode *assignee)
{
    STNode *assign, *expression;
    
    next (lexer);

    expression = parse_expression (lexer);
	
    assign = st_node_new (ST_ASSIGN_NODE);
    assign->assignee = assignee;
    assign->expression = expression;   

    return assign;
}
    
 
static STNode *
parse_expression (STLexer *lexer)
{
    STNode   *receiver = NULL;
    STToken  *token;
    
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
	parse_error ("expected expression", token);
    }

    return parse_message (lexer, receiver);
}

static STNode *
parse_subexpression (STLexer *lexer)
{
    STToken *token;
    STNode *expression;
    
    next (lexer);
    
    expression = parse_expression (lexer);
    
    token = current (lexer);
    if (st_token_type (token) != ST_TOKEN_RPAREN)
	parse_error ("expected ')' after expression", token);    
	
    next (lexer);

    return expression;
}

static STNode *
parse_return (STLexer *lexer)
{   
    next (lexer);

    STNode *node;

    node = st_node_new (ST_RETURN_NODE);
    node->expression = parse_expression (lexer);

    return node;
}

static STNode *
parse_statement (STLexer *lexer)
{
    STToken *token;
    
    token = current (lexer);

    if (st_token_type (token) != ST_TOKEN_RETURN)
	return parse_expression (lexer);
    else
	return parse_return (lexer);
}

static STNode *
parse_statements (STLexer *lexer)
{
    STToken *token;
    STNode  *expression, *statements = NULL;

    token = current (lexer);

    while (st_token_type (token) != ST_TOKEN_EOF
	   && st_token_type (token) != ST_TOKEN_BLOCK_END) {
	
	expression = parse_statement (lexer);
	
	statements = st_node_append (statements, expression);

	if (st_token_type (current (lexer)) == ST_TOKEN_BLOCK_END)
	    break;
	token = next (lexer);
    }
      
    return statements;
}

/*
 * '|' identifier* '|'
 */
static STNode *
parse_temporaries (STLexer *lexer)
{
    STToken *token;
    STNode *temporaries = NULL, *temp;

    token = current (lexer);
    
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_text (token), "|")) 
	return NULL;
   
    token = next (lexer);
    while (st_token_type (token) == ST_TOKEN_IDENTIFIER) {

	temp = st_node_new (ST_VARIABLE_NODE);
	temp->name = st_symbol_new (st_token_text (token));
	
	temporaries = st_node_append (temporaries, temp);
   
	token = next (lexer);
    }
    
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_text (token), "|"))
	parse_error ("expected '|'", token);
    
    token = next (lexer);
   
    return temporaries;
}

static void
parse_message_pattern (STLexer *lexer, STNode *method)
{    
    STToken      *token;
    STTokenType  type;
    STNode *arguments = NULL, *selector;
   
    selector = st_node_new (ST_SELECTOR_NODE);

    token = next (lexer);    
    type  = st_token_type (token);

    if (type == ST_TOKEN_IDENTIFIER) {
	
        selector->name = st_symbol_new (st_token_text (token));
	method->precedence = ST_UNARY_PRECEDENCE;

	next (lexer);
    
    } else if (type == ST_TOKEN_BINARY_SELECTOR) {

	selector->name = st_symbol_new (st_token_text (token));
	
	token = next (lexer);
	if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error ("argument name expected after binary selector", token);

	arguments = st_node_new (ST_VARIABLE_NODE);
	arguments->name = st_symbol_new (st_token_text (token));

	method->precedence = ST_BINARY_PRECEDENCE;

	next (lexer);
    
    } else if (type == ST_TOKEN_KEYWORD_SELECTOR) {
    
	GString *gstring = g_string_new (NULL);
      	STNode  *arg;

	while (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR) {	
	    g_string_append (gstring, st_token_text (token));

	    token = next (lexer);
	    if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
		parse_error ("argument name expected after keyword", token);	
	
	    arg = st_node_new (ST_VARIABLE_NODE);
	    arg->name = st_symbol_new (st_token_text (token));
	    arguments = st_node_append (arguments, arg);

	    token = next (lexer);
	} 
	
	selector->name = st_symbol_new (g_string_free (gstring, FALSE));
    
	method->precedence = ST_KEYWORD_PRECEDENCE;

    } else {
	parse_error ("invalid message pattern", token);
    }

    method->selector  = selector;
    method->arguments = arguments;
}

static STNode *
parse_method (STLexer *lexer)
{   
    STNode *method;

    method = st_node_new (ST_METHOD_NODE);
    
    parse_message_pattern (lexer, method);
    method->temporaries = parse_temporaries (lexer);
    method->statements  = parse_statements (lexer);

    return method;
} 

STNode *
st_parser_parse (STLexer *lexer)
{   
    g_assert (lexer != NULL);

    return parse_method (lexer);
}

