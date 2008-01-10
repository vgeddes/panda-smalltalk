/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-bytecode.h
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

G_BEGIN_DECLS typedef _GooBytecode GooBytecode;

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

} GooSpecialConstant;

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


GooBytecode *st_bytecode_new (void);

char *st_bytecode_get_bytes (GooBytecode * code);

guint st_bytecode_get_size (GooBytecode * code);

void st_bytecode_append (GooBytecode * code, GooBytecode * another);

void st_bytecode_destroy (GooBytecode * code);

/* Pushes */

void st_bytecode_push_instvar (GooBytecode * code, guint index);

void st_bytecode_push_temporary (GooBytecode * code, guint index);

void st_bytecode_push_literal_const (GooBytecode * code, guint index);

void st_bytecode_push_literal_var (GooBytecode * code, guint index);

void st_bytecode_push_special (GooBytecode * code, GooSpecialConstant constant);

void st_bytecode_push_active_context (GooBytecode * code);

void st_bytecode_push_block_closure (GooBytecode * code, guint index);


/* Stores & Pops */

void st_bytecode_store_pop_instvar (GooBytecode * code, guint index);

void st_bytecode_store_pop_temporary (GooBytecode * code, guint index);

void st_bytecode_store_pop_literal_var (GooBytecode * code, guint index);


/* Stores */

void st_bytecode_store_instvar (GooBytecode * code, guint index);

void st_bytecode_store_temporary (GooBytecode * code, guint index);

void st_bytecode_store_literal_var (GooBytecode * code, guint index);


/* Returns */

void st_bytecode_return_stack_top (GooBytecode * code);

void st_bytecode_return_special (GooBytecode * code, GooSpecialConstant constant);


/* Jumps */

void st_bytecode_jump (GooBytecode * code, short offset);

void st_bytecode_jump_true (GooBytecode * code, gushort offset);

void st_bytecode_jump_false (GooBytecode * code, gushort offset);



/* Misc */

void st_bytecode_pop (GooBytecode * code);

void st_bytecode_duplicate_stack_top (GooBytecode * code);


/* Sends */

void st_bytecode_super_send_selector (GooBytecode * code, guint index, guint num_args);

void st_bytecode_send_selector (GooBytecode * code, guint index, guint num_args);

void st_bytecode_send_message_special (GooBytecode * code, GooSpecialMessage message);


/* Info */

void st_bytecode_is_code (guchar c);

void st_bytecode_print_code (guchar c);


#endif /* __ST_BYTECODE_H__ */
