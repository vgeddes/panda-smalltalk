/*
 * st-large-integer.c
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

#include "st-large-integer.h"
#include "st-universe.h"
#include "st-types.h"

ST_DEFINE_VTABLE (st_large_integer, st_heap_object_vtable ());

static st_oop_t large_integer_new (mp_int * value);

/* TODO: Move these functions to the primitive interface
 */

#define VALUE(oop)             (&(ST_LARGE_INTEGER(oop)->value))


/* useful macros to avoid duplication of error-handling code */

#define OP_PROLOGUE              \
{                                \
    mp_int value;                \
    int result;			 \
    *error = false;              \
                                 \
    result = mp_init (&value);   \
    if (result != MP_OKAY)       \
	goto out;

#define OP_EPILOGUE                      \
    if (result != MP_OKAY)               \
	goto out;			 \
    return large_integer_new (&value);   \
					 \
out:                                     \
    mp_clear (&value);                   \
    *error = true;			 \
    return st_nil;                       \
}

#define BINARY_OP(op, a, b)                     \
OP_PROLOGUE                                     \
    result = op (VALUE (a), VALUE (b), &value); \
OP_EPILOGUE

#define BINARY_DIV_OP(op, a, b)                       \
OP_PROLOGUE                                           \
    result = op (VALUE (a), VALUE (b), &value, NULL); \
OP_EPILOGUE

#define UNARY_OP(op, a)              \
OP_PROLOGUE                          \
    result = op (VALUE (a), &value); \
OP_EPILOGUE

     static st_oop_t large_integer_add (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_OP (mp_add, a, b);
}

static st_oop_t
large_integer_sub (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_OP (mp_sub, a, b);
}

static st_oop_t
large_integer_mul (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_OP (mp_mul, a, b);
}

static st_oop_t
large_integer_div (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_DIV_OP (mp_div, a, b);
}

static st_oop_t
large_integer_mod (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_OP (mp_mod, a, b);
}

static st_oop_t
large_integer_sqr (st_oop_t a, bool * error)
{
    UNARY_OP (mp_sqr, a);
}

static st_oop_t
large_integer_neg (st_oop_t a, bool * error)
{
    UNARY_OP (mp_neg, a);
}

static st_oop_t
large_integer_abs (st_oop_t a, bool * error)
{
    UNARY_OP (mp_abs, a);
}

static st_oop_t
large_integer_gcd (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_OP (mp_gcd, a, b);
}

static st_oop_t
large_integer_lcm (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_OP (mp_lcm, a, b);
}

static st_oop_t
large_integer_bitor (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_OP (mp_or, a, b);
}

static st_oop_t
large_integer_bitand (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_OP (mp_and, a, b);
}

static st_oop_t
large_integer_bitxor (st_oop_t a, st_oop_t b, bool * error)
{
    BINARY_OP (mp_xor, a, b);
}

static st_oop_t
large_integer_bitshift (st_oop_t a, int shift, bool * error)
{
    mp_int value;
    int result;
    *error = false;

    result = mp_init (&value);
    if (result != MP_OKAY)
	goto out;

    if (shift >= 0)
	result = mp_mul_2d (VALUE (a), shift, &value);
    else
	result = mp_div_2d (VALUE (a), shift, &value, NULL);

    if (result != MP_OKAY)
	goto out;

    return large_integer_new (&value);

  out:
    mp_clear (&value);
    *error = true;
    return st_nil;
}


static bool
large_integer_eq (st_oop_t a, st_oop_t b, bool * error)
{
    *error = false;
    return mp_cmp (VALUE (a), VALUE (b)) == MP_EQ;
}

static bool
large_integer_lt (st_oop_t a, st_oop_t b, bool * error)
{
    *error = false;
    return mp_cmp (VALUE (a), VALUE (b)) == MP_LT;
}

static bool
large_integer_gt (st_oop_t a, st_oop_t b, bool * error)
{
    *error = false;
    return mp_cmp (VALUE (a), VALUE (b)) == MP_GT;
}

static bool
large_integer_le (st_oop_t a, st_oop_t b, bool * error)
{
    *error = false;
    int relation = mp_cmp (VALUE (a), VALUE (b));
    return (relation == MP_LT) || (relation == MP_EQ);
}

static bool
large_integer_ge (st_oop_t a, st_oop_t b, bool * error)
{
    *error = false;
    int relation = mp_cmp (VALUE (a), VALUE (b));
    return (relation == MP_GT) || (relation == MP_EQ);
}

static bool
large_integer_is_positive (st_oop_t a, bool * error)
{
    *error = false;
    return SIGN (VALUE (a)) != MP_NEG;
}

static bool
large_integer_is_prime (st_oop_t a, bool * error)
{
    int result;
    int isprime;
    *error = false;

    int ntrials = mp_prime_rabin_miller_trials (mp_count_bits (VALUE (a)));

    result = mp_prime_is_prime (VALUE (a), ntrials, &isprime);
    if (result != MP_OKAY) {
	*error = true;
	return false;
    }

    return isprime == MP_YES;
}


st_oop_t
st_large_integer_new_from_string (const char *string, guint radix)
{
    mp_int value;
    int result;

    g_assert (string != NULL);

    result = mp_init (&value);
    if (result != MP_OKAY)
	goto out;

    result = mp_read_radix (&value, string, radix);
    if (result != MP_OKAY)
	goto out;

    return large_integer_new (&value);

  out:
    mp_clear (&value);
    g_warning (mp_error_to_string (result));
    return st_nil;
}

char *
st_large_integer_to_string (st_oop_t integer, guint radix)
{
    int result;
    int size;

    result = mp_radix_size (VALUE (integer), radix, &size);
    if (result != MP_OKAY)
	goto out;

    char *str = g_malloc (size);

    mp_toradix (VALUE (integer), str, radix);
    if (result != MP_OKAY)
	goto out;

    return str;

  out:
    g_warning (mp_error_to_string (result));
    return NULL;
}

static st_oop_t
allocate_with_value (mp_int * value)
{
    st_oop_t object = st_allocate_object (sizeof (st_large_integer_t) / sizeof (st_oop_t));

    if (value)
	*VALUE (object) = *value;
    else
	mp_init (VALUE (object));

    return object;
}


static st_oop_t
large_integer_new (mp_int * value)
{
    return allocate_with_value (value);
}

static st_oop_t
allocate (st_oop_t klass)
{
    return allocate_with_value (NULL);
}

static void
st_large_integer_vtable_init (st_vtable_t * table)
{

    table->allocate = allocate;


}
