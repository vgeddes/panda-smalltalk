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
#include "st-byte-array.h"
#include "st-symbol.h"
#include "st-float.h"
#include "st-large-integer.h"
#include "st-universe.h"
#include "st-object.h"
#include "st-array.h"

#include <tommath.h>
#include <glib.h>
#include <errno.h>
#include <stdlib.h>
#include <setjmp.h> 


typedef struct
{
    STLexer  *lexer;
    bool      in_block;

    GError  **error;  
    jmp_buf   jmploc;
} STParser;
    

static void
parse_error (STParser   *parser,
	     const char *message,
	     STToken    *token)
{
    g_set_error (parser->error,
		 ST_COMPILATION_ERROR,
		 ST_COMPILATION_ERROR_FAILED,
		 "%i: %s", st_token_line (token), message);
    
   /* go back to start */
    longjmp (parser->jmploc, 0);
}

/* adaptor for st_lexer_next_token(). Catches lexer errors and filters out comments
 */
static STToken *
next (STParser *parser, STLexer *lexer)
{
    STToken     *token;
    STTokenType type;

    token = st_lexer_next_token (lexer);
    type  = st_token_type (token);

    if (type == ST_TOKEN_COMMENT)
	return next (parser, lexer);
    else if (type == ST_TOKEN_INVALID)
	parse_error (parser, st_lexer_error_message (lexer), token);	
	
    return token;
}

static STToken *
current (STLexer *lexer)
{
    return st_lexer_current_token (lexer);
}

static STNode *parse_statements     (STParser *parser);
static STNode *parse_temporaries    (STParser *parser);
static STNode *parse_subexpression  (STParser *parser);
static STNode *parse_expression     (STParser *parser);
static int     parse_primitive      (STParser *parser);

static STNode *
parse_block_arguments (STParser *parser)
{
    STToken *token;   
    STNode  *arguments = NULL, *arg;

    token = current (parser->lexer);

    while (st_token_type (token) == ST_TOKEN_COLON) {
	
	token = next (parser, parser->lexer);
	if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error (parser,"expected identifier", token);

	arg = st_node_new (ST_VARIABLE_NODE);
	arg->line = st_token_line (token);
	arg->name = st_symbol_new (st_token_text (token));
	arguments = st_node_list_append (arguments, arg);

	token = next (parser, parser->lexer);
    }
    
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR ||
	!streq (st_token_text (token), "|"))
	parse_error (parser,"expected ':' or '|'", token);

    next (parser, parser->lexer);

    return arguments;
}


static STNode *
parse_block (STParser *parser)
{
    STToken *token;
    STNode  *block;
    bool     nested;

    block = st_node_new (ST_BLOCK_NODE);
    
    // parse block arguments
    token = next (parser, parser->lexer);

    block->line = st_token_line (token);
    
    if (st_token_type (token) == ST_TOKEN_COLON)
	block->arguments = parse_block_arguments (parser);
    
    token = current (parser->lexer);
    if (st_token_type (token) == ST_TOKEN_BINARY_SELECTOR
	&& streq (st_token_text (token), "|"))
	block->temporaries = parse_temporaries (parser);
    
    nested = parser->in_block;
    parser->in_block = true;

    block->statements = parse_statements (parser);

    if (nested == false)
	parser->in_block = false;

    token = current (parser->lexer);
    if (st_token_type (token) != ST_TOKEN_BLOCK_END)
	parse_error (parser,"expected ']'", token);
    next (parser, parser->lexer);

    return block;
}

static STNode *
parse_number (STParser *parser)
{    
    int radix;
    int exponent;   
    char *number, *p;
    STNode *node;
    STToken *token;

    token = current (parser->lexer);
    
    radix    = st_number_token_radix (token);
    exponent = st_number_token_exponent (token);
    
    p = number = st_number_token_number (token);

    node = st_node_new (ST_LITERAL_NODE);
    node->line = st_token_line (token);

    /* check if there is a decimal point */
    while (*p && *p != '.')
	++p;
	
    if (*p == '.' || exponent != 0) {
	
	char *format;
	
	if (radix != 10)
	    parse_error (parser,"only base-10 floats are supported at the moment", token);

	format = g_strdup_printf ("%se%i", number, exponent);
	
	node->literal = st_float_new (strtod (format, NULL));
	
	g_free (format);

    } else {

	/* we use strtoll() when smi's are 64-bits wide.
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
		parse_error (parser,"memory exhausted while trying parse LargeInteger", token);
	
	    result = mp_read_radix (&value, number, radix);
	    if (result != MP_OKAY)
		parse_error (parser,"memory exhausted while trying parse LargeInteger", token);
	
	    node->literal = st_large_integer_new (&value);

	} else {
	    node->literal = st_smi_new (integer);
	}
    }

    next (parser, parser->lexer);

    return node;
}

static STNode *parse_primary (STParser *parser);

static STNode *
parse_tuple (STParser *parser)
{
    STToken *token;
    STNode *node;
    GList *items = NULL;

    token = next (parser, parser->lexer);
    while (true) {
	
	switch (st_token_type (token)) {
	case ST_TOKEN_NUMBER_CONST:
	case ST_TOKEN_STRING_CONST:
	case ST_TOKEN_SYMBOL_CONST:
	case ST_TOKEN_CHARACTER_CONST:
	    node = parse_primary (parser);
	    items = g_list_prepend (items, (gpointer) node->literal);
	    st_node_destroy (node);
	    break;
	    
	case ST_TOKEN_LPAREN:
	    node = parse_tuple (parser);
	    items = g_list_prepend (items, (gpointer) node->literal);
	    st_node_destroy (node);
	    break;
	
	default:
	    if (st_token_type (token) == ST_TOKEN_RPAREN) {
		goto out;
	    } else
		parse_error (parser,"expected ')'", token);
	}

	token = current (parser->lexer);
    }

 out:
    token = next (parser, parser->lexer);
    st_oop tuple;

    items = g_list_reverse (items);

    tuple = st_object_new_arrayed (st_array_class, g_list_length (items));

    int i = 1;
    for (GList *l = items; l; l = l->next)
	st_array_at_put (tuple, i++, (st_oop) l->data);
    
    node = st_node_new (ST_LITERAL_NODE);
    node->literal = tuple;
    node->line = st_token_line (token);

    g_debug ("%i\n", g_list_length (items)); 

    return node;
}


/* identifiers, literals, blocks */
static STNode *
parse_primary (STParser *parser)
{
    STNode *node;
    STToken *token;
    
    token = current (parser->lexer);
    
    switch (st_token_type (token)) {
	
    case ST_TOKEN_IDENTIFIER:
	
	node = st_node_new (ST_VARIABLE_NODE);
	node->line = st_token_line (token);
	node->name = st_string_new (st_token_text (token));

	next (parser, parser->lexer);
	break;

    case ST_TOKEN_NUMBER_CONST:
 
    	node = parse_number (parser);
	break;
	
    case ST_TOKEN_STRING_CONST: 
    
	node = st_node_new (ST_LITERAL_NODE);
	node->line = st_token_line (token);
	node->literal = st_string_new (st_token_text (token));

	next (parser, parser->lexer);
	break;

    case ST_TOKEN_SYMBOL_CONST:

	node = st_node_new (ST_LITERAL_NODE);
	node->line = st_token_line (token);
	node->literal = st_symbol_new (st_token_text (token));
    
	next (parser, parser->lexer);
	break;

    case ST_TOKEN_CHARACTER_CONST:

	node = st_node_new (ST_LITERAL_NODE);
	node->line = st_token_line (token);
	node->literal = st_character_new (g_utf8_get_char (st_token_text (token)));

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

static STNode *
parse_unary_message (STParser *parser, STNode *receiver)
{
    STToken *token;
    STNode  *message;
    
    token = current (parser->lexer);

    message = st_node_new (ST_MESSAGE_NODE);
    message->precedence = ST_UNARY_PRECEDENCE;
    message->receiver = receiver;
    message->selector = st_symbol_new (st_token_text (token));
    message->arguments = NULL;

    next (parser, parser->lexer);

    return message;
}

static STNode *
parse_binary_argument (STParser *parser, STNode *receiver)
{
    STNode  *message;
    STToken *token;

    token = current (parser->lexer);
    if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	return receiver;
   
    message = parse_unary_message (parser, receiver);

    return parse_binary_argument (parser, message);
}

static STNode *
parse_binary_message (STParser *parser, STNode *receiver)
{
    STNode  *message, *argument;
    STToken *token;
    char     *selector;

    token = current (parser->lexer);
    
    selector = st_token_text (token);

    /* parse the primary */
    token = next (parser, parser->lexer);
    if (st_token_type (token) == ST_TOKEN_LPAREN)
        argument = parse_subexpression (parser);
    else
	argument = parse_primary (parser);
     
    argument = parse_binary_argument (parser, argument);
   
    message = st_node_new (ST_MESSAGE_NODE);

    message->precedence = ST_BINARY_PRECEDENCE;
    message->receiver   = receiver;
    message->selector   = st_symbol_new (selector);
    message->arguments  = argument;

    return message;
}


static STNode *
parse_keyword_argument (STParser *parser, STNode *receiver)
{
    STToken *token;

    token = current (parser->lexer);
   
    if (receiver == NULL) {
	/* parse the primary */
	if (st_token_type (token) == ST_TOKEN_LPAREN)
	    receiver = parse_subexpression (parser);
	else
	    receiver = parse_primary (parser);

    } else if (st_token_type (token) == ST_TOKEN_IDENTIFIER) {
	receiver = parse_unary_message (parser, receiver);
	
    } else if (st_token_type (token) == ST_TOKEN_BINARY_SELECTOR && !streq (st_token_text (token), "!")) {
	receiver = parse_binary_message (parser, receiver);
	
    } else {
	return receiver;
    }

    return parse_keyword_argument (parser, receiver);
}

static STNode *
parse_keyword_message (STParser *parser, STNode *receiver)
{
    STToken *token;
    STNode  *message, *arguments = NULL, *arg;
    GString  *selector;
    
    selector = g_string_new (NULL);
    token = current (parser->lexer);
    
    while (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR) {
	
	g_string_append (selector, st_token_text (token));
	token = next (parser, parser->lexer);
	
	arg = parse_keyword_argument (parser, NULL);
	arguments = st_node_list_append (arguments, arg);

	token = current (parser->lexer);
    }
    
    message = st_node_new (ST_MESSAGE_NODE);

    message->precedence = ST_KEYWORD_PRECEDENCE;
    message->receiver = receiver;
    message->selector = st_symbol_new (g_string_free (selector, false));
    message->arguments = arguments;

    return message;
}

/*
 * parses an expression from left to right, by recursively parsing subexpressions
 */
static STNode *
parse_message (STParser *parser, STNode *receiver)
{
    STNode  *message = NULL;
    STToken *token;
    STTokenType type;

    /* Before parsing messages, check if expression is simply a variable.
     * This is the case if token is ')' or '.'
     */    
    token = current (parser->lexer);
    type = st_token_type (token);
    
    if (type == ST_TOKEN_PERIOD || type == ST_TOKEN_RPAREN
	|| type == ST_TOKEN_EOF || type == ST_TOKEN_BLOCK_END
	|| (type == ST_TOKEN_BINARY_SELECTOR && streq (st_token_text (token), "!")))
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
    
static STNode *
parse_assign (STParser *parser, STNode *assignee)
{
    STToken *token;
    STNode *assign, *expression;
    
    token = next (parser, parser->lexer);

    expression = parse_expression (parser);
	
    assign = st_node_new (ST_ASSIGN_NODE);
    assign->line = st_token_line (token);
    assign->assignee = assignee;
    assign->expression = expression;   

    return assign;
}
    
 
static STNode *
parse_expression (STParser *parser)
{
    STNode   *receiver = NULL;
    STToken  *token;
    
    token = current (parser->lexer);

    switch (st_token_type (token)) {
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

	if (st_token_type (current (parser->lexer)) == ST_TOKEN_ASSIGN)
	    return parse_assign (parser, receiver);       

	break;

    case ST_TOKEN_LPAREN:

	receiver = parse_subexpression (parser);
	break;
	
    default:
	parse_error (parser,"expected expression", token);
    }

    return parse_message (parser, receiver);
}

static STNode *
parse_subexpression (STParser *parser)
{
    STToken *token;
    STNode  *expression;
    
    next (parser, parser->lexer);
    
    expression = parse_expression (parser);
    
    token = current (parser->lexer);
    if (st_token_type (token) != ST_TOKEN_RPAREN)
	parse_error (parser,"expected ')' after expression", token);    
	
    next (parser, parser->lexer);

    return expression;
}

static STNode *
parse_return (STParser *parser)
{
    STToken *token;
   
    token = next (parser, parser->lexer);
    
    STNode *node;
    
    node = st_node_new (ST_RETURN_NODE);
    node->line = st_token_line (token);
    node->expression = parse_expression (parser);
    
    return node;
}

static STNode *
parse_statement (STParser *parser)
{
    STToken *token;
    
    token = current (parser->lexer);

    if (st_token_type (token) == ST_TOKEN_RETURN)
	return parse_return (parser);
    else
	return parse_expression (parser);
}

static STNode *
parse_statements (STParser *parser)
{
    STToken *token;
    STNode  *expression = NULL, *statements = NULL;

    token = current (parser->lexer);

    while (st_token_type (token) != ST_TOKEN_EOF
	   && (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR || !streq (st_token_text (token), "!"))) {
	       
	if (parser->in_block && st_token_type (token) == ST_TOKEN_BLOCK_END)
	    break;

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
	if (st_token_type (token) == ST_TOKEN_PERIOD) {   
	    token = next (parser, parser->lexer);
	}
	
    }

    return statements;
}


static int
parse_primitive (STParser *parser)
{
    STToken *token;
    int      index = -1;

    token = current (parser->lexer);
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_text (token), "<"))
	return -1;

    token = next (parser, parser->lexer);
    if (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR
	&& streq (st_token_text (token), "primitive:")) {
	
	token = next (parser, parser->lexer);
	if (st_token_type (token) != ST_TOKEN_STRING_CONST)
	    parse_error (parser,"expected string literal", token); 

	index = st_primitive_index_for_name (st_token_text (token));
	if (index < 0)
	    parse_error (parser,"unknown primitive", token); 
	
	token = next (parser, parser->lexer);
	if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR
	    || !streq (st_token_text (token), ">"))
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
static STNode *
parse_temporaries (STParser *parser)
{
    STToken *token;
    STNode *temporaries = NULL, *temp;

    token = current (parser->lexer);
    
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_text (token), "|")) 
	return NULL;
   
    token = next (parser, parser->lexer);
    while (st_token_type (token) == ST_TOKEN_IDENTIFIER) {

	temp = st_node_new (ST_VARIABLE_NODE);
	temp->line = st_token_line (token);
	temp->name = st_string_new (st_token_text (token));
	
	temporaries = st_node_list_append (temporaries, temp);
   
	token = next (parser, parser->lexer);
    }
    
    if (st_token_type (token) != ST_TOKEN_BINARY_SELECTOR
	|| !streq (st_token_text (token), "|"))
	parse_error (parser,"expected '|'", token);
    
    token = next (parser, parser->lexer);
   
    return temporaries;
}

static void
parse_message_pattern (STParser *parser, STNode *method)
{    
    STToken      *token;
    STTokenType  type;
    STNode *arguments = NULL;

    token = next (parser, parser->lexer);    
    type  = st_token_type (token);

    if (type == ST_TOKEN_IDENTIFIER) {
	
        method->selector = st_symbol_new (st_token_text (token));
	method->precedence = ST_UNARY_PRECEDENCE;

	next (parser, parser->lexer);
    
    } else if (type == ST_TOKEN_BINARY_SELECTOR) {

	method->selector = st_symbol_new (st_token_text (token));
	
	token = next (parser, parser->lexer);
	if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
	    parse_error (parser,"argument name expected after binary selector", token);

	arguments = st_node_new (ST_VARIABLE_NODE);
	arguments->line = st_token_line (token);
	arguments->name = st_string_new (st_token_text (token));

	method->precedence = ST_BINARY_PRECEDENCE;

	next (parser, parser->lexer);
    
    } else if (type == ST_TOKEN_KEYWORD_SELECTOR) {
    
	GString *gstring = g_string_new (NULL);
      	STNode  *arg;

	while (st_token_type (token) == ST_TOKEN_KEYWORD_SELECTOR) {	
	    g_string_append (gstring, st_token_text (token));
	    
	    token = next (parser, parser->lexer);
	    if (st_token_type (token) != ST_TOKEN_IDENTIFIER)
		parse_error (parser,"argument name expected after keyword", token);	
	
	    arg = st_node_new (ST_VARIABLE_NODE);
	    arg->line = st_token_line (token);
	    arg->name = st_string_new (st_token_text (token));
	    arguments = st_node_list_append (arguments, arg);

	    token = next (parser, parser->lexer);
	} 
	
	method->selector = st_symbol_new (g_string_free (gstring, FALSE));
    
	method->precedence = ST_KEYWORD_PRECEDENCE;

    } else {
	parse_error (parser,"invalid message pattern", token);
    }

    method->arguments = arguments;
}

static STNode *
parse_method (STParser *parser)
{   
    STNode *method;

    parser->in_block = false;

    method = st_node_new (ST_METHOD_NODE);
    
    parse_message_pattern (parser, method);
    
    method->temporaries = parse_temporaries (parser);
    method->primitive   = parse_primitive (parser);
    method->statements  = parse_statements (parser);
    
    return method;
} 


STNode *
st_parse_expression (STLexer *lexer, GError **error)
{
    STNode   *node;
    STParser *parser;

    g_assert (lexer != NULL);

    parser = g_slice_new0 (STParser);
    
    parser->lexer = lexer;
    parser->error = error;
    parser->in_block = false;

    if (!setjmp (parser->jmploc)) {
	next (parser, parser->lexer);
	node = parse_expression (parser);
    } else
	node = NULL;

    g_slice_free (STParser, parser);

    return node;

}

STNode *
st_parser_parse (STLexer *lexer, GError **error)
{
    STParser *parser;
    STNode   *method;

    g_assert (lexer != NULL);

    parser = g_slice_new0 (STParser);
    
    parser->lexer = lexer;
    parser->error = error;
    parser->in_block = false;

    if (!setjmp (parser->jmploc))
	method = parse_method (parser);
    else
	method = NULL;

    g_slice_free (STParser, parser);

    return method;
     
}

