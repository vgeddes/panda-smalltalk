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

typedef enum
{
    ST_METHOD_NODE,
    ST_BLOCK_NODE,
    ST_PRIMITIVE_NODE,
    ST_VARIABLE_NODE,
    ST_ASSIGN_NODE,
    ST_RETURN_NODE,
    ST_MESSAGE_NODE,
    ST_SELECTOR_NODE,
    ST_LITERAL_NODE,
    
} STNodeType;

typedef enum
{
    ST_UNARY_PRECEDENCE,
    ST_BINARY_PRECEDENCE,
    ST_KEYWORD_PRECEDENCE,

} STMessagePrecedence;

typedef enum
{
    
    POBNIK,

} STNodeFlags;

typedef struct STNode STNode;
 
struct STNode
{
    STNodeType type;
    
    STNodeFlags flags;
    
    /* line number where the node construct starts */
    int line;

    /* next node in list */
    STNode *next;

    /* Common fields */
    struct {
	STMessagePrecedence precedence;
	STNode *selector;
	STNode *temporaries;
	STNode *arguments;
	STNode *statements;
	STNode *expression;
    };

    union {
	/* Message */
	struct {
	    STNode *receiver;
	};
	/* Primitive */
	struct {
	    int primitive;
	};
	/* Assign */
	struct {
	    STNode *assignee;
	};
	/* Literal (SmallInteger, String, Symbol, Float, Character ) */
	struct {
	    st_oop literal;
	};
	/* Variable | Selector */
	struct {
	    st_oop name;
	};
    };
};


STNode *st_node_new (STNodeType type);

STNode *st_node_append (STNode *list, STNode *node);

void    st_print_method (STNode *method);

#endif /* __ST_AST_H__ */

