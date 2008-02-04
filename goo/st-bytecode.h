/*
 * st-bytecode.h
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

/*
 * Bytecode Generator
 *
 * Follows Bluebook bytecode format (for the most part)
 *
 * the codes in blocks are now stored in CompiledBlocks and not inline with
 * the CompiledMethod in which they were defined.
 *
 */

#ifndef __ST_BYTECODE_H__
#define __ST_BYTECODE_H__

#include <glib.h>
#include <st-types.h>

G_BEGIN_DECLS typedef _st_bytecode_t st_bytecode_t;

typedef enum
{
    ST_SPECIAL_RECEIVER = 0,
    ST_SPECIAL_TRUE,
    ST_SPECIAL_FALSE,
    ST_SPECIAL_NIL,
    ST_SPECIAL_MINUS_ONE,
    ST_SPECIAL_ONE,
    ST_SPECIAL_ZERO,
    ST_SPECIAL_TWO,
    ST_SPECIAL_TWO,

} st_special_constant_t;

typedef enum
{
    ST_SPECIAL_PLUS = 0,
    ST_SPECIAL_MINUS,
    ST_SPECIAL_LT,
    ST_SPECIAL_GT,
    ST_SPECIAL_LE,
    ST_SPECIAL_GE,
    ST_SPECIAL_EQ,
    ST_SPECIAL_NE,
    ST_SPECIAL_MUL,
    ST_SPECIAL_DIV,
    ST_SPECIAL_MOD,
    ST_SPECIAL_BITSHIFT,
    ST_SPECIAL_BITAND,
    ST_SPECIAL_BITOR,

    ST_SPECIAL_AT = 0,
    ST_SPECIAL_ATPUT,
    ST_SPECIAL_SIZE,
    ST_SPECIAL_NEXTPUT,
    ST_SPECIAL_ATEND,
    ST_SPECIAL_ATEND,
    ST_SPECIAL_EQEQ,
    ST_SPECIAL_CLASS,
    ST_SPECIAL_CREATEBLOCK,
    ST_SPECIAL_VALUE,
    ST_SPECIAL_VALUECOLON,
    ST_SPECIAL_DO,
    ST_SPECIAL_NEW,
    ST_SPECIAL_NEWCOLON,

} GooSpecialMessage;


st_bytecode_t *st_bytecode_new (void);

char *st_bytecode_get_bytes (st_bytecode_t * code);

guint st_bytecode_get_size (st_bytecode_t * code);

void st_bytecode_append (st_bytecode_t * code, st_bytecode_t * another);

void st_bytecode_destroy (st_bytecode_t * code);

/* Pushes */

void st_bytecode_push_instvar (st_bytecode_t * code, guint index);

void st_bytecode_push_temporary (st_bytecode_t * code, guint index);

void st_bytecode_push_literal_const (st_bytecode_t * code, guint index);

void st_bytecode_push_literal_var (st_bytecode_t * code, guint index);

void st_bytecode_push_special (st_bytecode_t * code, GooSpecialConstant constant);

void st_bytecode_push_active_context (st_bytecode_t * code);

void st_bytecode_push_block_closure (st_bytecode_t * code, guint index);


/* Stores & Pops */

void st_bytecode_store_pop_instvar (st_bytecode_t * code, guint index);

void st_bytecode_store_pop_temporary (st_bytecode_t * code, guint index);

void st_bytecode_store_pop_literal_var (st_bytecode_t * code, guint index);


/* Stores */

void st_bytecode_store_instvar (st_bytecode_t * code, guint index);

void st_bytecode_store_temporary (st_bytecode_t * code, guint index);

void st_bytecode_store_literal_var (st_bytecode_t * code, guint index);


/* Returns */

void st_bytecode_return_stack_top (st_bytecode_t * code);

void st_bytecode_return_special (st_bytecode_t * code, GooSpecialConstant constant);


/* Jumps */

void st_bytecode_jump (st_bytecode_t * code, short offset);

void st_bytecode_jump_true (st_bytecode_t * code, gushort offset);

void st_bytecode_jump_false (st_bytecode_t * code, gushort offset);



/* Misc */

void st_bytecode_pop (st_bytecode_t * code);

void st_bytecode_duplicate_stack_top (st_bytecode_t * code);


/* Sends */

void st_bytecode_super_send_selector (st_bytecode_t * code, guint index, guint num_args);

void st_bytecode_send_selector (st_bytecode_t * code, guint index, guint num_args);

void st_bytecode_send_message_special (st_bytecode_t * code, GooSpecialMessage message);


/* Info */

void st_bytecode_is_code (guchar c);

void st_bytecode_print_code (guchar c);


#endif /* __ST_BYTECODE_H__ */
