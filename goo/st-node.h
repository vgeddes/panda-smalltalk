/*
 * st-ast.c
 *
 * Copyright (c) 2008 Vincent Geddes
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

#ifndef __ST_NODE_H__
#define __ST_NODE_H__

#include <glib.h>
#include <st-types.h>

typedef enum
{
    ST_METHOD_NODE,
    ST_BLOCK_NODE,
    ST_VARIABLE_NODE,
    ST_ASSIGN_NODE,
    ST_RETURN_NODE,
    ST_MESSAGE_NODE,
    ST_LITERAL_NODE,
    
} STNodeType;

typedef enum
{
    ST_UNARY_PRECEDENCE,
    ST_BINARY_PRECEDENCE,
    ST_KEYWORD_PRECEDENCE,

} STMessagePrecedence;

typedef struct STNode STNode;

struct STNode
{
    STNodeType type;
        
    /* line number where the node construct starts */
    int line;

    /* next node in list */
    STNode *next;

    /* Common fields (to one or more node types) */
    struct {
	STMessagePrecedence precedence;
	st_oop  selector;
	STNode *temporaries;
	STNode *arguments;
	STNode *statements;
	STNode *expression;
    };

    union {
	/*  receiver for MessageNode */
	struct {
	    STNode *receiver;
	    bool    is_statement;
	};
	/* primitive for MethodNode */
	struct {
	    int primitive;
	};
	/* assignee for AssignNode */
	struct {
	    STNode *assignee;
	};
	/* value for LiteralNode */
	struct {
	    st_oop literal;
	};
	/* name for a VariableNode */
	struct {
	    st_oop name;
	};
    };
};

STNode *st_node_new          (STNodeType type);

STNode *st_node_list_append  (STNode *list, STNode *node);

guint   st_node_list_length  (STNode *list);

void    st_print_method_node (STNode *method);

void    st_node_destroy      (STNode *node);

#endif /* __ST_NODE_H__ */

