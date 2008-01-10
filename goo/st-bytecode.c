/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */
/*
 * st-bytecode.c
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

#include <st-bytecode.h>
#include <st-math.h>

#define INITIAL_CAPACITY 75

struct _GooBytecode
{
    GArray *buffer;
};

typedef enum
{
    invalid push_temp_0 push_temp_1 push_temp_2 push_temp_3 push_temp_4 push_temp_5 push_temp_6 push_temp_7 push_temp_8 push_temp_9 push_temp_10 push_temp_11 push_temp_12 push_temp_13 push_temp_14 push_temp_15 push_temp_n store_pop_temp_0 store_pop_temp_1 store_pop_temp_2 store_pop_temp_3 store_pop_temp_4 store_pop_temp_5 store_pop_temp_6 store_pop_temp_7 store_pop_temp_8 store_pop_temp_9 store_pop_temp_10 store_pop_temp_11 store_pop_temp_12 store_pop_temp_13 store_pop_temp_14 store_pop_temp_15 store_pop_temp_n push_instvar_0 push_instvar_1 push_instvar_2 push_instvar_3 push_instvar_4 push_instvar_5 push_instvar_6 push_instvar_7 push_instvar_8 push_instvar_9 push_instvar_10 push_instvar_11 push_instvar_12 push_instvar_13 push_instvar_14 push_instvar_15 push_instvar_n store_pop_instvar_0 store_pop_instvar_1 store_pop_instvar_2 store_pop_instvar_3 store_pop_instvar_4 store_pop_instvar_5 store_pop_instvar_6 store_pop_instvar_7 store_pop_instvar_8 store_pop_instvar_9 store_pop_instvar_10 store_pop_instvar_11 store_pop_instvar_12 store_pop_instvar_13 store_pop_instvar_14 store_pop_instvar_15 store_pop_instvar_n push_literal_const_0 push_literal_const_1 push_literal_const_2 push_literal_const_3 push_literal_const_4 push_literal_const_5 push_literal_const_6 push_literal_const_7 push_literal_const_8 push_literal_const_9 push_literal_const_10 push_literal_const_11 push_literal_const_12 push_literal_const_13 push_literal_const_14 push_literal_const_15 push_literal_const_16 push_literal_const_17 push_literal_const_18 push_literal_const_19 push_literal_const_20 push_literal_const_21 push_literal_const_22 push_literal_const_23 push_literal_const_24 push_literal_const_25 push_literal_const_26 push_literal_const_27 push_literal_const_28 push_literal_const_29 push_literal_const_30 push_literal_const_31 push_literal_const_n push_literal_var_0 push_literal_var_1 push_literal_var_2 push_literal_var_3 push_literal_var_4 push_literal_var_5 push_literal_var_6 push_literal_var_7 push_literal_var_8 push_literal_var_9 push_literal_var_10 push_literal_var_11 push_literal_var_12 push_literal_var_13 push_literal_var_14 push_literal_var_15 push_literal_var_16 push_literal_var_17 push_literal_var_18 push_literal_var_19 push_literal_var_20 push_literal_var_21 push_literal_var_22 push_literal_var_23 push_literal_var_24 push_literal_var_25 push_literal_var_26 push_literal_var_27 push_literal_var_28 push_literal_var_29 push_literal_var_30 push_literal_var_31 push_literal_var_n store_literal_var_n store_temp_n store_instvar_n push_self push_nil push_true push_false push_minus_one push_zero push_one push_two push_three push_four push_five push_six push_seven push_eight push_nine return_stack_top return_self return_nil return_true return_false pop_stack_top duplicate_stack_top push_active_context create_block jump_true	/* conditional,   short (+-127) forward jump */
	jump_false		/* conditional,   short (+-127) forward jump */
	jump			/* unconditional, long (+-127) forward/backward jump */
	long_jump_true		/* conditional,   short forward jump */
	long_jump_false		/* conditional,   short forward jump */
	long_jump		/* unconditional, long (+-32768) forward/backward jump */
    send_0_args
	send_1_args
	send_2_args
	send_n
	send_super_n
	send_plus
	send_minus
	send_lt
	send_gt
	send_le
	send_ge
	send_eq
	send_ne
	send_mul
	send_div
	send_mod
	send_bitshift
	send_bitand
	send_bitor
	send_bitxor
	send_at
	send_at_put
	send_size
	send_value
	send_value_arg send_identity_eq send_class send_hash send_do send_new send_new_arg
} Code;

typedef enum
{
    push,
    store_pop,
    store,
    pop,
    ret,
    jump,
    send,

} Type;

typedef enum
{
    B				/* byte             */
	BB			/* byte, byte       */
	BBB			/* byte, byte, byte */
} Format;

typedef struct
{
    Type type;
const char *name Format format} CodeSpec;

GooBytecode *
st_bytecode_new (void)
{
    GooBytecode *code;

    code = g_new (GooByteCode, 1);

    code->buffer = g_array_sized_new (false, true, sizeof (char), INITIAL_CAPACITY);

    return code;
}

char *
st_bytecode_get_bytes (GooBytecode * code)
{
    return code->buffer->data;

}

guint
st_bytecode_get_size (GooBytecode * code)
{
    return code->buffer->len;
}


void
st_bytecode_append (GooBytecode * code, GooBytecode * another)
{
    g_assert (code != NULL && another != NULL);

    g_array_append_vals (code->buffer, another->buffer->data, another->buffer->len);

}

static void
gen_code (GooBytecode * code, Format format, guchar a, guchar b, guchar c)
{
    switch (format) {

    case B:
	g_array_append_val (code->buffer, a);
	break;
    case BB:
	g_array_append_val (code->buffer, a);
	g_array_append_val (code->buffer, b);
	break;
    case BBB:
	g_array_append_val (code->buffer, a);
	g_array_append_val (code->buffer, b);
	g_array_append_val (code->buffer, c);
	break;
    default:
	g_assert_not_reached ();
    }
}

void
st_bytecode_push_instvar (GooBytecode * code, guint index)
{
    guchar c;
    if (index < 16) {
	c = push_instvar_0 + index;
	gen_code (code, B, c, 0, 0);

    } else {
	c = push_instvar_n;
	gen_code (code, BB, c, index, 0);
    }
}

void
st_bytecode_push_temp (GooBytecode * code, guint index)
{
    guchar c;
    if (index < 16) {
	c = push_temp_0 + index;
	gen_code (code, B, c, 0, 0);

    } else {
	c = push_temp_n;
	gen_code (code, BB, c, index, 0);
    }
}

void
st_bytecode_push_literal_const (GooBytecode * code, guint index)
{
    guchar c;
    if (index < 32) {
	c = push_literal_const_0 + index;
	gen_code (code, B, c, 0, 0);

    } else {
	c = push_literal_const_n;
	gen_code (code, BB, c, index, 0);
    }
}

void
st_bytecode_push_literal_var (GooBytecode * code, guint index)
{
    guchar c;
    if (index < 32) {
	c = push_literal_var_0 + index;
	gen_code (code, B, c, 0, 0);

    } else {
	c = push_literal_var_n;
	gen_code (code, BB, c, index, 0);
    }
}

void
st_bytecode_store_pop_instvar (GooBytecode * code, guint index)
{
    guchar c;
    if (index < 16) {
	c = store_pop_instvar_0 + index;
	gen_code (code, B, c, 0, 0);

    } else {
	c = store_pop_instvar_n;
	gen_code (code, BB, c, index, 0);
    }
}

void
st_bytecode_store_pop_temporary (GooBytecode * code, guint index)
{
    guchar c;
    if (index < 16) {
	c = store_pop_temp_0 + index;
	gen_code (code, B, c, 0, 0);

    } else {
	c = store_pop_temp_n;
	gen_code (code, BB, c, index, 0);
    }
}

void
st_bytecode_store_pop_literal_var (GooBytecode * code, guint index)
{
    guchar c;
    if (index < 16) {
	c = store_pop_literal_var_0 + index;
	gen_code (code, B, c, 0, 0);

    } else {
	c = store_pop_literal_var_n;
	gen_code (code, BB, c, index, 0);
    }
}

void
st_bytecode_store_instvar (GooBytecode * code, guint index)
{
    gen_code (code, B, store_instvar_n, index, 0);
}

void
st_bytecode_store_temporary (GooBytecode * code, guint index)
{
    gen_code (code, B, store_temp_n, index, 0);
}

void
st_bytecode_store_literal_var (GooBytecode * code, guint index)
{
    gen_code (code, B, store_literal_var_n, index, 0);
}

#define HIGH_BYTE(num) ((num & 0xFF00) >> 8)
#define LOW_BYTE(num)  (num & 0xFF)

void
st_bytecode_jump (GooBytecode * code, short offset)
{
    g_assert (abs (offset) > 0);

    if (-127 <= offset && offset <= 127) {
	gen_code (code, BB, jump, (guchar) offset, 0);
    } else {
	gen_code (code, BBB, long_jump, HIGH_BYTE (offset), LOW_BYTE (offset));
    }
}

void
jump_conditional (GooBytecode * code, bool condition, gushort offset)
{
    guchar c;
    g_assert (offset > 0);

    if (offset <= 255) {
	c = condition ? jump_true : jump_false;
	gen_code (code, BB, c, (guchar) offset, 0);
    } else {
	c = condition ? long_jump_true : long_jump_false;
	gen_code (code, BBB, c, HIGH_BYTE (offset), LOW_BYTE (offset));
    }
}

void
st_bytecode_jump_true (GooBytecode * code, gushort offset)
{
    jump_conditional (code, true, offset);
}

void
st_bytecode_jump_false (GooBytecode * code, gushort offset)
{
    jump_conditional (code, false, offset);
}



/*
 * this will destroy all bytes in the code as well!
 */
void
st_bytecode_destroy (GooBytecode * code)
{
    g_assert (code != NULL);

    g_array_free (code->bytes, TRUE);
    g_free (code);
}
