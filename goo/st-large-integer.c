/*
 * st-large-integer.c
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

#include "st-large-integer.h"
#include "st-universe.h"
#include "st-types.h"
#include "math.h"

#define VALUE(oop)             (&(ST_LARGE_INTEGER(oop)->value))


/* useful macros to avoid duplication of error-handling code */

#define OP_PROLOGUE			 \
{					 \
    mp_int value;			 \
    int result;				 \
    *error = false;			 \
    					 \
    result = mp_init (&value);		 \
    if (result != MP_OKAY)		 \
	goto out;


#define OP_EPILOGUE			    \
    if (result != MP_OKAY)		    \
	goto out;			    \
    return st_large_integer_new (&value);   \
    					    \
out:					    \
    mp_clear (&value);			    \
    *error = true;			    \
    return st_nil;			    \
 }

#define BINARY_OP(op, a, b)                     \
OP_PROLOGUE					\
    result = op (VALUE (a), VALUE (b), &value);	\
OP_EPILOGUE

#define BINARY_DIV_OP(op, a, b)                       \
OP_PROLOGUE                                           \
    result = op (VALUE (a), VALUE (b), &value, NULL); \
OP_EPILOGUE

#define UNARY_OP(op, a)              \
OP_PROLOGUE                          \
    result = op (VALUE (a), &value); \
OP_EPILOGUE

st_oop
st_large_integer_add (st_oop a, st_oop b, bool *error)
{
    BINARY_OP (mp_add, a, b);
}

st_oop
st_large_integer_sub (st_oop a, st_oop b, bool *error)
{
    BINARY_OP (mp_sub, a, b);
}

st_oop
st_large_integer_mul (st_oop a, st_oop b, bool *error)
{
    BINARY_OP (mp_mul, a, b);
}

st_oop
st_large_integer_div (st_oop a, st_oop b, bool *error)
{
    BINARY_DIV_OP (mp_div, a, b);
}

st_oop
st_large_integer_mod (st_oop a, st_oop b, bool *error)
{
    BINARY_OP (mp_mod, a, b);
}

st_oop
st_large_integer_sqr (st_oop a, bool *error)
{
    UNARY_OP (mp_sqr, a);
}

st_oop
st_large_integer_neg (st_oop a, bool *error)
{
    UNARY_OP (mp_neg, a);
}

st_oop
st_large_integer_abs (st_oop a, bool *error)
{
    UNARY_OP (mp_abs, a);
}

st_oop
st_large_integer_gcd (st_oop a, st_oop b, bool *error)
{
    BINARY_OP (mp_gcd, a, b);
}

st_oop
st_large_integer_lcm (st_oop a, st_oop b, bool *error)
{
    BINARY_OP (mp_lcm, a, b);
}

st_oop
st_large_integer_bitor (st_oop a, st_oop b, bool *error)
{
    BINARY_OP (mp_or, a, b);
}

st_oop
st_large_integer_bitand (st_oop a, st_oop b, bool *error)
{
    BINARY_OP (mp_and, a, b);
}

st_oop
st_large_integer_bitxor (st_oop a, st_oop b, bool *error)
{
    BINARY_OP (mp_xor, a, b);
}

st_oop
st_large_integer_bitshift (st_oop a, int shift, bool *error)
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

    return st_large_integer_new (&value);

  out:
    mp_clear (&value);
    *error = true;
    return st_nil;
}


bool
st_large_integer_eq (st_oop a, st_oop b, bool * error)
{
    *error = false;
    return mp_cmp (VALUE (a), VALUE (b)) == MP_EQ;
}

bool
st_large_integer_lt (st_oop a, st_oop b, bool * error)
{
    *error = false;
    return mp_cmp (VALUE (a), VALUE (b)) == MP_LT;
}

bool
st_large_integer_gt (st_oop a, st_oop b, bool * error)
{
    *error = false;
    return mp_cmp (VALUE (a), VALUE (b)) == MP_GT;
}

bool
st_large_integer_le (st_oop a, st_oop b, bool * error)
{
    *error = false;
    int relation = mp_cmp (VALUE (a), VALUE (b));
    return (relation == MP_LT) || (relation == MP_EQ);
}

bool
st_large_integer_ge (st_oop a, st_oop b, bool * error)
{
    *error = false;
    int relation = mp_cmp (VALUE (a), VALUE (b));
    return (relation == MP_GT) || (relation == MP_EQ);
}

bool
st_large_integer_is_positive (st_oop a, bool * error)
{
    *error = false;
    return SIGN (VALUE (a)) != MP_NEG;
}

bool
st_large_integer_is_prime (st_oop a, bool * error)
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

st_oop
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

    return st_large_integer_new (&value);

  out:
    mp_clear (&value);
    g_warning (mp_error_to_string (result));
    return st_nil;
}

st_oop
st_large_integer_new_from_smi (st_smi integer)
{
    mp_int value;
    int    result;
    bool   negative = integer < 0;
    st_smi integer_abs = abs(integer);

    result = mp_init_set_int (&value, integer_abs);
    if (result != MP_OKAY)
	goto out;
    
    if (negative)
	mp_neg (&value, &value);
    
    return st_large_integer_new (&value);

  out:
    mp_clear (&value);
    g_warning (mp_error_to_string (result));
    return st_nil;
}

char *
st_large_integer_to_string (st_oop integer, guint radix)
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

static st_oop
allocate_with_value (st_oop klass, mp_int * value)
{
    st_oop object = st_allocate_object (sizeof (STLargeInteger) / sizeof (st_oop));

    st_heap_object_initialize_header (object, klass);

    if (value)
	*VALUE (object) = *value;
    else
	mp_init (VALUE (object));

    return object;
}

st_oop
st_large_integer_new (mp_int * value)
{
    return allocate_with_value (st_large_integer_class, value);
    
}

static st_oop
allocate (st_oop klass)
{
    return allocate_with_value (klass, NULL);
}


static st_oop
large_integer_copy (st_oop object)
{
    mp_int value;
    int result;

    result = mp_init_copy (&value, VALUE (object));
    if (result != MP_OKAY)
	g_assert_not_reached ();

    return st_large_integer_new (&value);
}

const STDescriptor *
st_large_integer_descriptor (void)
{
    static const STDescriptor __descriptor =
	{ .allocate         = allocate,
	  .allocate_arrayed = NULL,
	  .copy             = large_integer_copy,
	};

    return & __descriptor;
}
