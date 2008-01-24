/* 
 * st-ast.h
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

#ifndef __ST_AST_H__
#define __ST_AST_H__

#include <glib.h>
#include <st-types.h>
#include <tommath.h>

typedef enum
{
    ST_NODE_METHOD,

    ST_NODE_EXPRESSION,
    ST_NODE_ASSIGN,
    ST_NODE_RETURN,
    
    ST_NODE_MESSAGE_UNARY,
    ST_NODE_MESSAGE_BINARY,
    ST_NODE_MESSAGE_KEYWORD,
    
    ST_NODE_BLOCK,
    ST_NODE_IDENTIFIER,
    ST_NODE_LITERAL_INTEGER,
    ST_NODE_LITERAL_LARGE_INTEGER,
    ST_NODE_LITERAL_FLOATING,
    ST_NODE_LITERAL_STRING,
    ST_NODE_LITERAL_SYMBOL,
    ST_NODE_LITERAL_CHARACTER,
    
} st_node_type_t;




typedef struct st_node_t                  st_node_t;
typedef struct st_node_method_t           st_node_method_t;
typedef struct st_node_identifier_t       st_node_identifier_t;
typedef struct st_node_literal_t          st_node_literal_t;
typedef struct st_node_block_t            st_node_block_t;
typedef struct st_node_assign_t           st_node_assign_t;
typedef struct st_node_return_t           st_node_return_t;
typedef struct st_node_expression_t       st_node_expression_t;
typedef struct st_node_unary_message_t    st_node_unary_message_t;
typedef struct st_node_binary_message_t   st_node_binary_message_t;
typedef struct st_node_keyword_message_t  st_node_keyword_message_t;


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
 *
 * Toplevel AST Node for representing a method.
 */
struct st_node_method_t
{
    st_node_t parent;

    /* message selector */
    char *selector;

    GList *args;
    GList *temps;

    /* statements (type: variable/assign; last element may be a return) */
    GList *statements;

};


/*
 * st_node_block_t:
 * AST Node for blocks.
 */
struct st_node_block_t
{
    st_node_t parent;

    /* lists of st_node_identifier_t nodes */
    GList *args;
    GList *temps;

    /* statements (type: statement) */
    GList *statements;

};



/*
 * st_node_expression_t:
 *
 * AST Node for representing expressions.
 */
struct st_node_expression_t
{
    st_node_t parent;

    /* variable
     */
    st_node_t  *receiver;

    /* messages (normally 1 message, but more for cascading messages)
     */
    st_node_t *message;
};


/*
 * st_node_return_t:
 *
 * AST Node for representing assignments.
 */
struct st_node_return_t
{
    st_node_t parent;

    /* variable
     */
    st_node_t *variable;
};


/*
 * st_node_assignment_t:
 *
 * AST Node for representing assignments.
 */
struct st_node_assign_t
{
    st_node_t parent;

    /* identifier
     */
    st_node_t *assignee;

    /* variable
     */
    st_node_t *variable;
};



/*
 * st_node_literal_t:
 * AST Node for literal constants
 */
struct st_node_literal_t
{
    st_node_t parent;

    union {
	double     floating;
	st_smi_t   integer;
	mp_int     large_integer;
	char      *string;
	gunichar   character;
    };
};

struct st_node_identifier_t
{
    st_node_t parent;

    /* name of identifier */
    char *name;

};

struct st_node_unary_message_t
{
    st_node_t parent;

    char *selector;
};

struct st_node_binary_message_t
{
    st_node_t parent;

    char *selector;

    /* argument (type: variable)
     */
    st_node_t  *argument;
};

struct st_node_keyword_message_t
{
    st_node_t parent;

    char *selector;

    /* arguments (type: variable)
     */
    GList *arguments;
};

#define ST_NODE_CAST(node)                 ((st_node_t *) node)
#define ST_NODE_METHOD_CAST(node)          ((st_node_method_t *) node)
#define ST_NODE_IDENTIFIER_CAST(node)      ((st_node_identifier_t *) node)
#define ST_NODE_LITERAL_CAST(node)         ((st_node_literal_t *) node)
#define ST_NODE_BLOCK_CAST(node)           ((st_node_block_t *) node)
#define ST_NODE_ASSIGN_CAST(node)          ((st_node_assign_t *) node)
#define ST_NODE_RETURN_CAST(node)          ((st_node_return_t *) node)
#define ST_NODE_EXPRESSION_CAST(node)      ((st_node_expression_t *) node)
#define ST_NODE_UNARY_MESSAGE_CAST(node)   ((st_node_unary_message_t *) node)
#define ST_NODE_BINARY_MESSAGE_CAST(node)  ((st_node_binary_message_t *) node)
#define ST_NODE_KEYWORD_MESSAGE_CAST(node) ((st_node_keyword_message_t *) node)


st_node_type_t  st_node_type (st_node_t *node);

/* node classifications
 *
 * variables are literals, identifiers, blocks and expressions
 *
 * messages can be unary, binary, or keywords
 *
 * literals can be integers, large-integers, floats, strings, symbols, chars
 *   blocks, tuples
 */

bool  st_node_is_variable (st_node_t *node);
bool  st_node_is_message  (st_node_t *node);

/* node constructors */

st_node_t *st_node_method_new        (const char *selector,
				      GList      *args,
				      GList      *temps,
				      GList      *statements);
				      
st_node_t *st_node_block_new	     (GList      *args,
				      GList      *temps,
				      GList      *statements);


st_node_t *st_node_expression_new    (st_node_t  *receiver, st_node_t *message);
st_node_t *st_node_assign_new        (st_node_t  *assignee,
				      st_node_t  *variable);
st_node_t *st_node_return_new        (st_node_t  *variable);


st_node_t *st_node_unary_message_new   (const char *selector);
st_node_t *st_node_binary_message_new  (const char *selector,
				        st_node_t  *argument);
st_node_t *st_node_keyword_message_new (const char *selector,
				        GList      *arguments);

st_node_t *st_node_identifier_new    (const char *name);

st_node_t  *st_node_literal_integer_new       (st_smi_t    integer);
st_node_t  *st_node_literal_large_integer_new (mp_int     *large_integer);
st_node_t  *st_node_literal_float_new         (double      floating);
st_node_t  *st_node_literal_string_new        (const char *string);
st_node_t  *st_node_literal_symbol_new        (const char *symbol);
st_node_t  *st_node_literal_character_new     (gunichar    character);

void st_print_node (st_node_t *node);

#endif /* __ST_AST_H__ */

