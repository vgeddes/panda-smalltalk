/*
 * st-parser.c
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
#include "st-lexer.h"
#include "st-utils.h"
#include "st-primitives.h"
#include "st-array.h"
#include "st-symbol.h"
#include "st-float.h"
#include "st-large-integer.h"
#include "st-universe.h"
#include "st-object.h"
#include "st-array.h"
#include "st-character.h"
#include "st-unicode.h"
#include "st-behavior.h"

#include <tommath.h>
#include <errno.h>
#include <stdlib.h>
#include <setjmp.h> 


typedef struct
{
    st_lexer  *lexer;
    bool      in_block;

    st_compiler_error *error;  
    jmp_buf   jmploc;
} st_parser;

static void
parse_error (st_parser   *parser,
	     const char *message,
	     st_token    *token)
{
    if (parser->error) {
	strncpy (parser->error->message, message, 255);
	parser->error->line   = st_token_get_line (token);
	parser->error->column = st_token_get_column (token);
    }
    
    longjmp (parser->jmploc, 0);
}

/* adaptor for st_lexer_next_token(). Catches lexer errors and filters out comments
 */
static st_token *
next (st_parser *parser, st_lexer *lexer)
{
    st_token     *token;
    st_token_type type;

    token = st_lexer_next_token (lexer);
    type  = st_token_get_type (token);

    if (type == ST_TOKEN_COMMENT)
	return next (parser, lexer);
    else if (type == ST_TOKEN_INVALID)
	parse_error (parser, st_lexer_error_message (lexer), token);	
	
    return token;
}

static st_token *
current (st_lexer *lexer)
{
    return st_lexer_current_token (lexer);
}

static st_node *parse_statements     (st_parser *parser);
static st_node *parse_temporaries    (st_parser *parser);
static st_node *parse_subexpression  (st_parser *parser);
static st_node *parse_expression     (st_parser *parser);
static int     parse_primitive      (st_parser *parser);

static st_node *
parse_block_arguments (st_parser *parser)
{
    st_token *token;   
    st_node  *arguments = NULL, *node;

    token = current (parser->lexer);

    while (st_token_get_type (token) == ST_TOKEN_COLON) {
	
	token = next (parser, parser->lexer);
	if (st_token_get_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error (parser,"expected identifier", token);

	node = st_node_new (ST_VARIABLE_NODE);
	node->line = st_token_get_line (token);
	node->variable.name = st_strdup (st_token_get_text (token));
	arguments = st_node_list_append (arguments, node);

	token = next (parser, parser->lexer);
    }
    
    if (st_token_get_type (token) != ST_TOKEN_BINARY_SELECTOR ||
	!streq (st_token_get_text (token), "|"))
	parse_error (parser,"expected ':' or '|'", token);

    next (parser, parser->lexer);

    return arguments;
}


static st_node *
parse_block (st_parser *parser)
{
    st_token *token;
    st_node  *node;
    bool     nested;

    node = st_node_new (ST_BLOCK_NODE);
    
    // parse block arguments
    token = next (parser, parser->lexer);

    node->line = st_token_get_line (token);
    
    if (st_token_get_type (token) == ST_TOKEN_COLON)
	node->block.arguments = parse_block_arguments (parser);
    
    token = current (parser->lexer);
    if (st_token_get_type (token) == ST_TOKEN_BINARY_SELECTOR
	&& streq (st_token_get_text (token), "|"))
	node->block.temporaries = parse_temporaries (parser);

    nested = parser->in_block;
    parser->in_block = true;

    node->block.statements = parse_statements (parser);

    if (nested == false)
	parser->in_block = false;

    token = current (parser->lexer);
    if (st_token_get_type (token) != ST_TOKEN_BLOCK_END)
	parse_error (parser,"expected ']'", token);
    next (parser, parser->lexer);

    return node;
}

static st_node *
parse_number (st_parser *parser)
{    
    int      radix;
    int      sign;
    int      exponent;   
    char    *number, *p;
    st_node  *node;
    st_token *token;

    token = current (parser->lexer);
    
    sign     = st_number_token_negative (token) ? -1 : 1;
    radix    = st_number_token_radix (token);
    exponent = st_number_token_exponent (token);
    
    p = number = st_number_token_number (token);

    node = st_node_new (ST_LITERAL_NODE);
    node->line = st_token_get_line (token);

    /* check if there is a decimal point */
    while (*p && *p != '.')
	++p;
	
    if (*p == '.' || exponent != 0) {
	
	char *format;
	
	if (radix != 10)
	    parse_error (parser,"only base-10 floats are supported at the moment", token);

	format = st_strdup_printf ("%se%i", number, exponent);
	
	node->literal.value = st_float_new (sign * strtod (format, NULL));
	
	st_free (format);

    } else {

	/* we use strtoll() when smi's are 64-bits wide.
	 */
	errno = 0;
#if ST_HOST32 == 1
	long int integer = sign * strtol (number, NULL, radix);
#else
	long long int integer = sign * strtoll (number, NULL, radix);
#endif
	/* check for overflow */
	if (errno == ERANGE
	    || integer < ST_SMALL_INTEGER_MIN || integer > ST_SMALL_INTEGER_MAX) {
	
	    mp_int value;
	    int result;
	    
	    result = mp_init (&value);
	    if (result != MP_OKAY)
		parse_error (parser,"memory exhausted while trying parse LargeInteger", token);
	
	    result = mp_read_radix (&value, number, radix);
	    if (result != MP_OKAY)
		parse_error (parser,"memory exhausted while trying parse LargeInteger", token);
	
	    if (sign == -1)
		mp_neg (&value, &value);

	    node->literal.value = st_large_integer_new (&value);

	} else {
	    node->literal.value = st_smi_new (integer);
	}
    }

    next (parser, parser->lexer);

    return node;
}

static st_node *parse_primary (st_parser *parser);

static st_node *
parse_tuple (st_parser *parser)
{
    st_token *token;
    st_node *node;
    st_list *items = NULL;

    token = next (parser, parser->lexer);
    while (true) {
	
	switch (st_token_get_type (token)) {
	case ST_TOKEN_NUMBER_CONST:
	case ST_TOKEN_STRING_CONST:
	case ST_TOKEN_SYMBOL_CONST:
	case ST_TOKEN_CHARACTER_CONST:
	    node = parse_primary (parser);
	    items = st_list_prepend (items, (st_pointer) node->literal.value);
	    st_node_destroy (node);
	    break;
	    
	case ST_TOKEN_LPAREN:
	    node = parse_tuple (parser);
	    items = st_list_prepend (items, (st_pointer) node->literal.value);
	    st_node_destroy (node);
	    break;
	
	default:
	    if (st_token_get_type (token) == ST_TOKEN_RPAREN) {
		goto out;
	    } else
		parse_error (parser,"expected ')'", token);
	}

	token = current (parser->lexer);
    }

 out:
    token = next (parser, parser->lexer);
    st_oop tuple;

    items = st_list_reverse (items);

    tuple = st_object_new_arrayed (ST_ARRAY_CLASS, st_list_length (items));

    int i = 1;
    for (st_list *l = items; l; l = l->next)
	st_array_at_put (tuple, i++, (st_oop) l->data);
    
    node = st_node_new (ST_LITERAL_NODE);
    node->literal.value = tuple;
    node->line = st_token_get_line (token);

    return node;
}


/* identifiers, literals, blocks */
static st_node *
parse_primary (st_parser *parser)
{
    st_node *node = NULL;
    st_token *token;
    
    token = current (parser->lexer);
    
    switch (st_token_get_type (token)) {
	
    case ST_TOKEN_IDENTIFIER:
	
	node = st_node_new (ST_VARIABLE_NODE);
	node->line = st_token_get_line (token);
	node->variable.name = st_strdup (st_token_get_text (token));

	next (parser, parser->lexer);
	break;

    case ST_TOKEN_NUMBER_CONST:
 
    	node = parse_number (parser);
	break;
	
    case ST_TOKEN_STRING_CONST: 
    
	node = st_node_new (ST_LITERAL_NODE);
	node->line = st_token_get_line (token);
	node->literal.value = st_string_new (st_token_get_text (token));

	next (parser, parser->lexer);
	break;

    case ST_TOKEN_SYMBOL_CONST:

	node = st_node_new (ST_LITERAL_NODE);
	node->line = st_token_get_line (token);
	node->literal.value = st_symbol_new (st_token_get_text (token));
    
	next (parser, parser->lexer);
	break;

    case ST_TOKEN_CHARACTER_CONST:

	node = st_node_new (ST_LITERAL_NODE);
	node->line = st_token_get_line (token);
	node->literal.value = st_character_new (st_utf8_get_unichar (st_token_get_text (token)));

	next (parser, parser->lexer);
	break;

    case ST_TOKEN_BLOCK_BEGIN:
    
	node = parse_block (parser);
	break;

    case ST_TOKEN_TUPLE_BEGIN:
	
	node = parse_tuple (parser);
	break;

    default:
	parse_error (parser,"expected expression", token);
    }

    return node;
}

static st_node *
parse_unary_message (st_parser *parser, st_node *receiver)
{
    st_token *token;
    st_node  *node;
    
    token = current (parser->lexer);

    node = st_node_new (ST_MESSAGE_NODE);
    node->line = st_token_get_line (token);
    node->message.precedence = ST_UNARY_PRECEDENCE;
    node->message.receiver = receiver;
    node->message.selector = st_symbol_new (st_token_get_text (token));
    node->message.arguments = NULL;

    next (parser, parser->lexer);

    return node;
}

static st_node *
parse_binary_argument (st_parser *parser, st_node *receiver)
{
    st_node  *node;
    st_token *token;

    token = current (parser->lexer);
    if (st_token_get_type (token) != ST_TOKEN_IDENTIFIER)
	return receiver;
   
    node = parse_unary_message (parser, receiver);

    return parse_binary_argument (parser, node);
}

static st_node *
parse_binary_message (st_parser *parser, st_node *receiver)
{
    st_node  *node, *argument;
    st_token *token;
    char     *selector;

    token = current (parser->lexer);
    
    selector = st_token_get_text (token);

    /* parse the primary */
    token = next (parser, parser->lexer);
    if (st_token_get_type (token) == ST_TOKEN_LPAREN)
        argument = parse_subexpression (parser);
    else
	argument = parse_primary (parser);
     
    argument = parse_binary_argument (parser, argument);
   
    node = st_node_new (ST_MESSAGE_NODE);
    
    node->message.precedence = ST_BINARY_PRECEDENCE;
    node->message.receiver   = receiver;
    node->message.selector   = st_symbol_new (selector);
    node->message.arguments  = argument;

    return node;
}


static st_node *
parse_keyword_argument (st_parser *parser, st_node *receiver)
{
    st_token *token;

    token = current (parser->lexer);
   
    if (receiver == NULL) {
	/* parse the primary */
	if (st_token_get_type (token) == ST_TOKEN_LPAREN)
	    receiver = parse_subexpression (parser);
	else
	    receiver = parse_primary (parser);

    } else if (st_token_get_type (token) == ST_TOKEN_IDENTIFIER) {
	receiver = parse_unary_message (parser, receiver);
	
    } else if (st_token_get_type (token) == ST_TOKEN_BINARY_SELECTOR && !streq (st_token_get_text (token), "!")) {
	receiver = parse_binary_message (parser, receiver);
	
    } else {
	return receiver;
    }

    return parse_keyword_argument (parser, receiver);
}

static st_node *
parse_keyword_message (st_parser *parser, st_node *receiver)
{
    st_token *token;
    st_node  *node, *arguments = NULL, *arg;
    char *temp, *string = st_strdup ("");
    
    token = current (parser->lexer);
    
    while (st_token_get_type (token) == ST_TOKEN_KEYWORD_SELECTOR) {
	
	temp = st_strconcat (string, st_token_get_text (token), NULL);
	st_free (string);
	string = temp;

	token = next (parser, parser->lexer);
	
	arg = parse_keyword_argument (parser, NULL);
	arguments = st_node_list_append (arguments, arg);

	token = current (parser->lexer);
    }

    node = st_node_new (ST_MESSAGE_NODE);

    node->message.precedence = ST_KEYWORD_PRECEDENCE;
    node->message.receiver = receiver;
    node->message.selector = st_symbol_new (string);
    node->message.arguments = arguments;

    return node;
}

/*
 * parses an expression from left to right, by recursively parsing subexpressions
 */
static st_node *
parse_message (st_parser *parser, st_node *receiver)
{
    st_node  *message = NULL;
    st_token *token;
    st_token_type type;

    /* Before parsing messages, check if expression is simply a variable.
     * This is the case if token is ')' or '.'
     */    
    token = current (parser->lexer);
    type = st_token_get_type (token);
    
    if (type == ST_TOKEN_PERIOD || type == ST_TOKEN_RPAREN || type == ST_TOKEN_SEMICOLON
	|| type == ST_TOKEN_EOF || type == ST_TOKEN_BLOCK_END
	|| (type == ST_TOKEN_BINARY_SELECTOR && streq (st_token_get_text (token), "!")))
	return receiver;

    if (type == ST_TOKEN_IDENTIFIER)
	message = parse_unary_message (parser, receiver);

    else if (type == ST_TOKEN_BINARY_SELECTOR)
	message = parse_binary_message (parser, receiver);

    else if (type == ST_TOKEN_KEYWORD_SELECTOR)
	message = parse_keyword_message (parser, receiver);

    else
	parse_error (parser,"nothing more expected", token);

    return parse_message (parser, message);
}
    
static st_node *
parse_assign (st_parser *parser, st_node *assignee)
{
    st_token *token;
    st_node *node, *expression;
    
    token = next (parser, parser->lexer);

    expression = parse_expression (parser);
	
    node = st_node_new (ST_ASSIGN_NODE);
    node->line = st_token_get_line (token);
    node->assign.assignee = assignee;
    node->assign.expression = expression;   

    return node;
}

static st_node *
parse_cascade (st_parser *parser, st_node *first_message)
{
    st_token *token;
    st_node *message, *node;
    bool super_send = first_message->message.super_send;

    token = current (parser->lexer);

    node = st_node_new (ST_CASCADE_NODE);
    node->line = st_token_get_line (token);

    node->cascade.receiver = first_message->message.receiver;
    node->cascade.messages = st_list_append (node->cascade.messages, first_message);

    first_message->message.receiver = NULL;

    while (st_token_get_type (token) == ST_TOKEN_SEMICOLON) {
    
	next (parser, parser->lexer);

	message = parse_message (parser, NULL);

	if (message == NULL)
	    parse_error (parser,"expected cascade", token);
	    
	message->message.super_send = super_send;	    

	node->cascade.messages = st_list_append (node->cascade.messages, message);
	token = current (parser->lexer);
    }

    return node;
}   
 
static st_node *
parse_expression (st_parser *parser)
{
    st_node   *receiver = NULL;
    st_node   *message, *cascade;
    st_token  *token;
    bool super_send =false;
    
    token = current (parser->lexer);

    switch (st_token_get_type (token)) {
    case ST_TOKEN_NUMBER_CONST:
    case ST_TOKEN_STRING_CONST:
    case ST_TOKEN_SYMBOL_CONST:
    case ST_TOKEN_CHARACTER_CONST:    
    case ST_TOKEN_BLOCK_BEGIN:
    case ST_TOKEN_TUPLE_BEGIN:

	receiver = parse_primary (parser);
	break;

    case ST_TOKEN_IDENTIFIER:

	receiver = parse_primary (parser);

	if (st_token_get_type (current (parser->lexer)) == ST_TOKEN_ASSIGN)
	    return parse_assign (parser, receiver);       

	break;

    case ST_TOKEN_LPAREN:

	receiver = parse_subexpression (parser);
	break;
	
    default:
	parse_error (parser,"expected expression", token);
    }

    /* check if receiver is pseudo-variable 'super' */
    if (receiver->type == ST_VARIABLE_NODE
	&& streq (receiver->variable.name, "super"))
	super_send = true;

    message = parse_message (parser, receiver);
    message->message.super_send = super_send;

    token = current (parser->lexer);
    if (st_token_get_type (token) == ST_TOKEN_SEMICOLON)
	return parse_cascade (parser, message);
    else
	return message;
}

static st_node *
parse_subexpression (st_parser *parser)
{
    st_token *token;
    st_node  *expression;
    
    next (parser, parser->lexer);
    
    expression = parse_expression (parser);
    
    token = current (parser->lexer);
    if (st_token_get_type (token) != ST_TOKEN_RPAREN)
	parse_error (parser,"expected ')' after expression", token);    
	
    next (parser, parser->lexer);

    return expression;
}

static st_node *
parse_return (st_parser *parser)
{
    st_node *node;
    st_token *token;
   
    token = next (parser, parser->lexer);
    
    node = st_node_new (ST_RETURN_NODE);
    node->line = st_token_get_line (token);
    node->retrn.expression = parse_expression (parser);
    
    return node;
}

static st_node *
parse_statement (st_parser *parser)
{
    st_token *token;
    
    token = current (parser->lexer);

    if (st_token_get_type (token) == ST_TOKEN_RETURN)
	return parse_return (parser);
    else
	return parse_expression (parser);
}

static st_node *
parse_statements (st_parser *parser)
{
    st_token *token;
    st_node  *expression = NULL, *statements = NULL;

    token = current (parser->lexer);

    while (st_token_get_type (token) != ST_TOKEN_EOF
	   && st_token_get_type (token) != ST_TOKEN_BLOCK_END) {

	/* check for unreachable statements */
	if (expression && expression->type == ST_RETURN_NODE) {
	    /* first check that unreachable statement is valid ! */
	    parse_statement (parser);
	    parse_error (parser,"statement is unreachable", token);
	}

	expression = parse_statement (parser);
	statements = st_node_list_append (statements, expression);

	/* Consume statement delimiter ('.') if there is one.
	 *
         * If the current token is a wrongly placed/mismatched
         * closing token (')' or ']'), then parse_expression() will handle
         * that.
         */
	token = current (parser->lexer);
	if (st_token_get_type (token) == ST_TOKEN_PERIOD) {   
	    token = next (parser, parser->lexer);
	}
	
    }

    for (st_node *node = statements; node; node = node->next) {
	if (parser->in_block && node->type == ST_MESSAGE_NODE && node->next != NULL)
	    node->message.is_statement = true;
	else if ((!parser->in_block) && node->type == ST_MESSAGE_NODE)
	    node->message.is_statement = true;

	if (parser->in_block && node->type == ST_CASCADE_NODE && node->next != NULL)
	    node->cascade.is_statement = true;
	else if ((!parser->in_block) && node->type == ST_CASCADE_NODE)
	    node->cascade.is_statement = true;
    }

    return statements;
}


static int
parse_primitive (st_parser *parser)
{
    st_token *token;
    int      index = -1;

    token = current (parser->lexer);
    if (st_token_get_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_get_text (token), "<"))
	return -1;

    token = next (parser, parser->lexer);
    if (st_token_get_type (token) == ST_TOKEN_KEYWORD_SELECTOR
	&& streq (st_token_get_text (token), "primitive:")) {
	
	token = next (parser, parser->lexer);
	if (st_token_get_type (token) != ST_TOKEN_STRING_CONST)
	    parse_error (parser,"expected string literal", token); 

	index = st_primitive_index_for_name (st_token_get_text (token));
	if (index < 0)
	    parse_error (parser,"unknown primitive", token); 
	
	token = next (parser, parser->lexer);
	if (st_token_get_type (token) != ST_TOKEN_BINARY_SELECTOR
	    || !streq (st_token_get_text (token), ">"))
	    parse_error (parser,"expected '>'", token);
	
	next (parser, parser->lexer);
	
    } else {
	parse_error (parser,"expected primitive declaration", token);
    }
    
    return index;
}

/*
 * '|' identifier* '|'
 */
static st_node *
parse_temporaries (st_parser *parser)
{
    st_token *token;
    st_node *temporaries = NULL, *temp;

    token = current (parser->lexer);
    
    if (st_token_get_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_get_text (token), "|")) 
	return NULL;
   
    token = next (parser, parser->lexer);
    while (st_token_get_type (token) == ST_TOKEN_IDENTIFIER) {

	temp = st_node_new (ST_VARIABLE_NODE);
	temp->line = st_token_get_line (token);
	temp->variable.name = st_strdup (st_token_get_text (token));
	
	temporaries = st_node_list_append (temporaries, temp);
   
	token = next (parser, parser->lexer);
    }
    
    if (st_token_get_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_get_text (token), "|"))
	parse_error (parser,"expected '|'", token);
    
    token = next (parser, parser->lexer);
   
    return temporaries;
}

static void
parse_message_pattern (st_parser *parser, st_node *method)
{    
    st_token      *token;
    st_token_type  type;
    st_node *arguments = NULL;

    token = next (parser, parser->lexer);    
    type  = st_token_get_type (token);

    if (type == ST_TOKEN_IDENTIFIER) {
	
        method->method.selector = st_symbol_new (st_token_get_text (token));
	method->method.precedence = ST_UNARY_PRECEDENCE;

	next (parser, parser->lexer);
    
    } else if (type == ST_TOKEN_BINARY_SELECTOR) {

	method->method.selector = st_symbol_new (st_token_get_text (token));
	
	token = next (parser, parser->lexer);
	if (st_token_get_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error (parser,"argument name expected after binary selector", token);

	arguments = st_node_new (ST_VARIABLE_NODE);
	arguments->line = st_token_get_line (token);
	arguments->variable.name = st_strdup (st_token_get_text (token));

	method->method.precedence = ST_BINARY_PRECEDENCE;

	next (parser, parser->lexer);
    
    } else if (type == ST_TOKEN_KEYWORD_SELECTOR) {
    
	char *temp, *string = st_strdup ("");
      	st_node  *arg;

	while (st_token_get_type (token) == ST_TOKEN_KEYWORD_SELECTOR) {	
	    
	    temp = st_strconcat (string, st_token_get_text (token), NULL);
	    st_free (string);
	    string = temp;

	    token = next (parser, parser->lexer);
	    if (st_token_get_type (token) != ST_TOKEN_IDENTIFIER)
		parse_error (parser,"argument name expected after keyword", token);	
	
	    arg = st_node_new (ST_VARIABLE_NODE);
	    arg->line = st_token_get_line (token);
	    arg->variable.name = st_strdup (st_token_get_text (token));
	    arguments = st_node_list_append (arguments, arg);

	    token = next (parser, parser->lexer);
	} 
	
	method->method.selector = st_symbol_new (string);
	method->method.precedence = ST_KEYWORD_PRECEDENCE;

    } else {
	parse_error (parser,"invalid message pattern", token);
    }

    method->method.arguments = arguments;
}

static st_node *
parse_method (st_parser *parser)
{   
    st_node *node;

    parser->in_block = false;

    node = st_node_new (ST_METHOD_NODE);

    node->method.primitive = -1;
   
    parse_message_pattern (parser, node);
    
    node->method.temporaries = parse_temporaries (parser);
    node->method.primitive   = parse_primitive (parser);
    node->method.statements  = parse_statements (parser);

    st_assert (node->type == ST_METHOD_NODE);

    return node;
}

st_node *
st_parser_parse (st_lexer *lexer,
		 st_compiler_error *error)
{
    st_parser *parser;
    st_node   *method;

    st_assert (lexer != NULL);

    parser = st_new0 (st_parser);
    
    parser->lexer = lexer;
    parser->error = error;
    parser->in_block = false;

    if (!setjmp (parser->jmploc)) {
	method = parse_method (parser); 
    } else {
	method = NULL;
    }

    st_free (parser);

    return method;
}

