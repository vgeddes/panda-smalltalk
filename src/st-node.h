/*
 * st-node.h
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

#include <st-types.h>
#include <st-utils.h>

typedef enum
{
    ST_METHOD_NODE,
    ST_BLOCK_NODE,
    ST_VARIABLE_NODE,
    ST_ASSIGN_NODE,
    ST_RETURN_NODE,
    ST_MESSAGE_NODE,
    ST_CASCADE_NODE,
    ST_LITERAL_NODE,
    
} st_node_type;

typedef enum
{
    ST_UNARY_PRECEDENCE,
    ST_BINARY_PRECEDENCE,
    ST_KEYWORD_PRECEDENCE,

} STMessagePrecedence;

typedef struct st_node st_node;

struct st_node
{
    st_node_type type;
    int        line;
    st_node    *next;

    union {

	struct {
	    STMessagePrecedence precedence;
	    int     primitive;
	    st_oop  selector;
	    st_node *statements;
	    st_node *temporaries;
	    st_node *arguments;

	} method;

	struct {

	    STMessagePrecedence precedence;
	    bool    is_statement;
	    st_oop  selector;
	    st_node *receiver;
	    st_node *arguments;

	    bool super_send;

	} message;

	struct {

	    char *name;

	} variable;

	struct {

	    st_oop value;

	} literal;

	struct {

	    st_node *assignee;
	    st_node *expression;

	} assign;

	struct {

	    st_node *expression;

	} retrn;

	struct {

	    st_node *statements;
	    st_node *temporaries;
	    st_node *arguments;

	} block;

	struct {

	    st_node *receiver;
	    st_list *messages;
	    bool is_statement;
	    
	} cascade;

    };

};

st_node *st_node_new          (st_node_type type);

st_node *st_node_list_append  (st_node *list, st_node *node);

st_uint  st_node_list_length  (st_node *list);

void     st_print_method_node (st_node *method);

void     st_node_destroy      (st_node *node);

#endif /* __ST_NODE_H__ */

