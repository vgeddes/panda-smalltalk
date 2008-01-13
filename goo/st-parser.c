/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
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

#include <glib.h>

#include <st-types.h>
#include <st-lexer.h>

#include <tommath.h>

typedef enum
{
    /* non-terminals */
    ST_NODE_METHOD,
    ST_NODE_BLOCK,
    ST_NODE_EXPRESSION,
    ST_NODE_ASSIGNMENT,
    ST_NODE_RETURN,
    ST_NODE_MESSAGE,
    
    /* terminals */
    ST_NODE_LITERAL,
    ST_NODE_IDENTIFIER,
    
} st_node_type_t;

typedef enum
{
    ST_NODE_VARIABLE_IDENTIFIER,
    ST_NODE_VARIABLE_LITERAL,
    ST_NODE_VARIABLE_BLOCK,
    
} st_node_variable_type_t;

typedef enum
{
    ST_NODE_LITERAL_FLOATING,
    ST_NODE_LITERAL_INTEGER,
    ST_NODE_LITERAL_LARGE_INTEGER,
    ST_NODE_LITERAL_STRING,
    ST_NODE_LITERAL_SYMBOL,
    ST_NODE_LITERAL_CHARACTER,
    ST_NODE_LITERAL_TUPLE,
    
} st_node_literal_type_t;

typedef enum
{
    ST_NODE_MESSAGE_UNARY,
    ST_NODE_MESSAGE_BINARY,
    ST_NODE_MESSAGE_KEYWORD,
    
} st_node_message_type_t;



typedef struct st_node_t                  st_node_t;
typedef struct st_node_variable_t         st_node_variable_t;
typedef struct st_node_identifier_t       st_node_identifier_t;
typedef struct st_node_literal_t          st_node_literal_t;
typedef struct st_node_block_t            st_node_block_t;
typedef struct st_node_expression_t       st_node_expression_t;
typedef struct st_node_message_t          st_node_message_t;
typedef struct st_node_unary_message_t    st_node_unary_message_t;
typedef struct st_node_binary_message_t   st_node_binary_message_t;
typedef struct st_node_keyword_message_t  st_node_keyword_message_t;
typedef struct st_node_keyword_and_arg_t  st_node_keyword_and_arg_t;

/* 
 * st_node_t:
 * Base of AST Node hierarchy.
 */
struct st_node_t
{
    st_node_type_t type;
};

/* 
 * st_node_method_t:
 * Toplevel AST Node for representing a method.
 */
struct st_node_method_t
{
    st_node_t parent;

    /* message pattern type */
    st_node_message_type_t message_type;
    
    /* message selector */
    char *selector;  

    /* list of st_node_identifier_t nodes */
    GList *arg;
    GList *temps;

    /* expression statements, optionally followed by a return statement */
    GList *statements;

};

/* 
 * st_node_variable_t:
 * AST Node for variables (instvars, literals, temps)
 */
struct st_node_variable_t
{
    st_node_t parent;

    st_node_variable_type_t  variable_type;
};

/* 
 * st_node_block_t:
 * AST Node for blocks.
 */
struct st_node_block_t
{
    st_node_variable_t parent;
    
    /* lists of st_node_identifier_t nodes */
    GList *args;
    GList *temps;

    /* expression statements, optionally followed by a return statement */
    GList *statements;

};

/* 
 * st_node_literal_t:
 * AST Node for literal constants
 */
struct st_node_literal_t
{
    st_node_variable_t parent;
    
    st_node_literal_type_t literal_type;
    
    union {
	double     floating;
	st_smi_t   integer;
	mp_int     large_integer;
	char      *string;
	gunichar   character;
	
	/* variable* | tuple*
	 */
	GList     *tuple_elements;
    };
};

/* 
 * st_node_identifier_t:
 * AST Node for identifiers
 */
struct st_node_identifier_t
{
    st_node_variable_t parent;
   
    /* index of instvar in receiver */ 
    char *name;  
    
};

/* 
 * st_node_assignment_t:
 * AST Node for representing assignments.
 */
struct st_node_assignment_t
{
    st_node_t parent;

    /* identifier
     */
    st_node_identifier_t *primary;
    
    /* variable | expression
     */
    st_node_t *message;
};

/* 
 * st_node_expression_t:
 * AST Node for representing expressions.
 */
struct st_node_expression_t
{
    st_node_t parent;

    /* variable | expression
     */
    st_node_t  *receiver;
    
    /* (unary-message | binary-message | keyword-message)+
     */
    GList *messages;
};

struct st_node_message_t
{
    st_node_t parent;

    st_node_message_type_t;
};


struct st_node_unary_message_t
{
    st_node_message_t parent;

    char *selector;
};

struct st_node_binary_message_t
{
    st_node_message_t parent;

    char *selector;
    
    /* variable | expression
     */
    st_node_t  *argument;
    
    /* unary-message*
     */
    GList *messages;
};

struct st_node_keyword_message_t
{
    st_node_message_t parent;

    char *selector;

    /* keyword-and-argument+
     */
    GList *keyargs;
};

struct st_node_keyword_and_arg_t
{
    st_node_t parent;

    char *keyword;
 
    /* variable | expression
     */
    st_node_t  *arg;
   
    /* unary-message* | binary-message*
     */
    GList *messages;
};







typedef struct
{

    st_lexer_t *lexer;
    st_oop_t    method;

    st_node_t  *ast;
    
    int       balance;

} st_parser_t;

static st_node_type_t
st_node_type (st_node_t *node)
{
    return node->type;
}


/* hide insane namespacing of lexer api */

#define token_t       st_lexer_token_t
#define token_type_t  st_lexer_token_type_t

#define lexer_next(parser)     (st_lexer_next_token ((parser)->lexer))
#define lexer_current(parser)  (st_lexer_current_token ((parser)->lexer))
#define token_type(token)      (st_lexer_token_type (token))
#define token_text(token)      (st_lexer_token_text (token))
#define token_text_take(token) (g_strdup (st_lexer_token_text (token)))


/*
 * we use strtol on 32bit hosts, and strtoll on 64bit hosts.
 *
 */
static st_node_t *
parse_number (st_node_t *node, char *number)
{

    char *p = number;
    
    int radix = 10;
    int len = strlen (number);
    
    long int integer;
    
    /* examples:
     *   1 23425 654.456 2r1010 16rDEADBEEF 23e-2 23.34e-3 67e 16r56FE.AEF
     */
	
    while (*p && isdigit (*p))
	++p;

    /* extract integer or radix */	
    if (*p == 0) {
	node->type = ST_NODE_LITERAL_INTEGER;
#if ST_HOST32 == 1
	node->integer = strtol (number, NULL, 10);
#else
	node->integer = strtoll (number, NULL, 10);
#endif 
	/* exit early */
	return node;
	
    } else if (*p == 'r') {
	*p = 0;
	radix = strtol (number, NULL, 10);
	*p = 'r';
	if (radix < 2 || radix > 36) {
	    parse_error (p, _("specified radix for number must be between 2 and 36"), token);
	}   
    
    }
    
    if (radix > 10) {
    
    while (*p && isdigit (*p))
	++p;
        
    
    }


}

/*
 * current token: literal
 */
static st_node_t *
parse_literal (st_parser_t *p)
{
    st_node_literal_t *node, *token;
    
    node = (st_node_literal_t *) node_new (ST_NODE_LITERAL);

    token = lexer_current (p);

    switch (token_type (token)) {
    
    case ST_TOKEN_CHAR_CONST:
	node->type = ST_NODE_LITERAL_CHARACTER;
	node->character = token_text_take (token_text (token));
	break;
    
    case ST_TOKEN_STRING_CONST:
	node->type = ST_NODE_LITERAL_STRING;
	node->string = token_text_take (token_text (token));    
	break;
	   
    case ST_TOKEN_NUMBER_CONST:
	parse_integer (node, token_text (token));
	break;
	    
    case ST_TOKEN_SYMBOL_CONST:
	node->type = ST_NODE_LITERAL_SYMBOL;
	node->string = token_text_take (token_text (token));    
	break;
	
    default:
	g_assert_not_reached ();
    
    }
    
    lexer_next (p);

    return node;
}


/*
 * current token: identifier
 */
static st_node_t *
parse_indentifier (st_parser_t *p)
{
    st_node_t *node = node_new (ST_NODE_IDENTIFIER);

    node->name = token_text_take (lexer_current (p));
    
    lexer_next (p);

    return node;
}

/*
 * current token: '#('
 *
 * match: (tuple | identifier | literal | block)* ')'
 */
static st_node_t *
parse_tuple (st_parser_t *p)
{
    st_node_t *node, *item, *token;
    
    /* Tuples are literals */    
    node = node_new (ST_NODE_LITERAL);
    node->literal_type = ST_NODE_LITERAL_TUPLE;
    node->elements = NULL;
    
    token = lexer_next (p);
    
    // return with success if tuple is empty
    if (token_type (token) == ST_TOKEN_RPAREN)
	return node;
    
    while {
    
	switch (token_type (token)) {

	case ST_TOKEN_LPAREN:
	    item = parse_tuple (p);
	    break;
	    
	case ST_TOKEN_IDENTIFIER:
	    item = parse_identifier (p);
	    break;
    
	case ST_TOKEN_CHAR_CONST:
	case ST_TOKEN_STRING_CONST:
	case ST_TOKEN_NUMBER_CONST:
	case ST_TOKEN_SYMBOL_CONST:
	    item = parse_literal (p);    
	    break;
    
	case ST_TOKEN_BLOCK_BEGIN:
	    item = parse_block (p);
	    break;
	    
	default:
	    parse_error (p, "only literals and variables are allowed in tuples", token);
	}
	
	node->elements = g_list_append (node->elements, item);	
    }
    
    token = lexer_current (p);
    if (token_type (token) != ST_TOKEN_RPAREN)
	parser_error (p, "expected ')'", token);
    
    lexer_next (p);
    
    return node;
}

/*
 * current token: '(' or '.'
 */
static st_node_t *
parse_expression (st_parser_t *p)
{
    st_node_t *node, *receiver;
    token_t   *token;
    
    node = node_new (ST_NODE_EXPRESSION);
    
    /* parse receiver
     *
     * identifier | literal | block | '(' expression ')'
     */
    token = lexer_next (p);
    switch (token_type (token)) {
     
    case ST_TOKEN_IDENTIFIER:

	receiver = parse_identifier (p);    
	break;
	
    case ST_TOKEN_LPAREN:

	/* parse nested expression */
	receiver = parse_expression (p);
	if (lexer_current (p) != ST_TOKEN_RPAREN)
	    parse_error (p, "expected ')'", lexer_current (p));
	break;
	
    case ST_TOKEN_CHAR_CONST:
    case ST_TOKEN_STRING_CONST:
    case ST_TOKEN_NUMBER_CONST:
    case ST_TOKEN_SYMBOL_CONST:
	
	receiver = parse_literal (p);
	break;
    
    case ST_TOKEN_TUPLE_BEGIN:
    
	receiver = parse_tuple (p);    
	break;
	
    case ST_TOKEN_BLOCK_BEGIN:
    
	receiver = parse_block (p);    
	break;

    default:
	parse_error (p, "expected expression", token);
    }
    
    /*
     * Before parsing messages, check if expression is simply a variable.
     * This is the case if token is ')' or '.'
     */
    token = lexer_current (p);
    if (token_type (token) == ST_TOKEN_RPAREN ||
        token_type (token) == ST_TOKEN_PERIOD)
	return receiver;

    /* Now parse messages, checking for cascades */
   
    node->receiver = receiver;

    do {
	node->messages = g_list_append (node->messages, parse_message (p));
    
    } while ((token = lexer_next (p)) == ST_TOKEN_SEMICOLON);
    
    
    
    return node;
}

/*
 * (expression '.')* return-statement? 
 */
static GList *
parse_statements (st_parser_t *p)
{
    token_t   *token;
    st_node_t *node;
    
    GList *statements = NULL;
    
    do {

        node = parse_expression (p, node);
	g_list_append (statements, node);
 
    } while (token_type != ST_TOKEN_EOF || token_type != 
    
    if (token_type (token) == ST_TOKEN_RETURN) {
	
	parse_exp
    
    
    }
    
}

/*
 * '|' identifier* '|'
 */
static void
parse_temporaries (st_parser_t *p, st_node_method_t *node)
{
    token_t *token;

    token = lexer_next (p);
    
    // return if token text != '|'
    if (token_type (token) != ST_TOKEN_BINARY_SELECTOR)
	return;
    else {
	if (!streq (token_text (token), "|"))
	    return;    
    }
     
    while ((token = lexer_next (p)) == ST_TOKEN_IDENTIFIER)
	node->temps = g_list_append (node->temps, token_text_take (token));
     
    // return if token text != '|'
    if (token_type (token) != ST_TOKEN_BINARY_SELECTOR)
	parse_error (p, token);
    else {
	if (!streq (token_text (token), "|"))
	    parse_error (p, token);    
    }
}


/*
 * (unary-selector | binary-selector argument | (keyword argument)* )+
 */
static void
parse_message_pattern (st_parser_t *p, st_node_method_t *node)
{    
    token_t *token = lexer_next (p); 
    token_type_t type = token_type (token);
    
    if (type == ST_TOKEN_IDENTIFIER) {
    
	node->message_type = ST_NODE_MESSAGE_UNARY;    
	node->selector = token_text_take (token);
    
    } else if (type == ST_TOKEN_BINARY_SELECTOR) {
    
	node->message_type = ST_NODE_MESSAGE_BINARY;    
	node->selector = token_text_take (token);

	token = lexer_next (p);
	if (token_type (p) != ST_TOKEN_IDENTIFIER)
	    parse_error (p, token);
	node->args = g_list_append (node->args, token_text_take (token));
    
    } else if (type == ST_TOKEN_KEYWORD_SELECTOR) {
    
	node->message_type = ST_NODE_MESSAGE_KEYWORD;
	GString *selector = g_string_new (token_text (token));
	
	token = lexer_next (p);
	if (token_type (p) != ST_TOKEN_IDENTIFIER)
	    parse_error (p, token);	
	node->args = g_list_append (node->args, token_text_take (token));	
	
	while ((token = lexer_next (p)) == ST_TOKEN_KEYWORD_SELECTOR) {
	    g_string_append (selector, token_text (token));
	    
	    token = lexer_next (p);
	    if (token_type (p) != ST_TOKEN_IDENTIFIER)
		parse_error (p, token);
	    node->args = g_list_append (node->args, token_text_take (p));	    
	}
	
	node->selector = g_string_free (selector, FALSE);
    
    } else {
	parse_error (p, token);
    }

}

static void
parse_method (st_parser_t *parser)
{
    st_node_method_t *node;
    
    node = node_new (ST_NODE_METHOD);

    parse_message_pattern (parser, node);
    parse_temporaries (parser, node);
    parse_statements (parser, node);
    
    p->ast = node;

} 

static const test_case[] =
"foo: bar"
"#(var1 var2 var3 [:x | x * 2])";

void
st_parser_parse (char *src)
{
    st_parser_t *p = g_slice_new (st_parser_t);
    p->lexer = st_lexer_new (src);
    p->method = compiled_method;
    
    parse_method (p);
}

