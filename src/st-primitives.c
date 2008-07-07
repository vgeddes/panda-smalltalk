/*
 * st-primitives.c
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

#include "st-primitives.h"
#include "st-processor.h"
#include "st-array.h"
#include "st-large-integer.h"
#include "st-float.h"
#include "st-array.h"
#include "st-object.h"
#include "st-behavior.h"
#include "st-context.h"
#include "st-method.h"
#include "st-symbol.h"
#include "st-character.h"
#include "st-unicode.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <fcntl.h>
#include <unistd.h>


#define ST_PRIMITIVE_FAIL(pr)			\
    pr->success = false


static inline void
set_success (st_processor *pr, bool success)
{
    pr->success = pr->success && success;
}

static inline st_smi
pop_integer (st_processor *pr)
{
    st_oop object = ST_STACK_POP (pr);
  
    if (ST_LIKELY (st_object_is_smi (object)))
	return st_smi_value (object);	

    ST_PRIMITIVE_FAIL (pr);
    return 0;
}

static inline st_smi
pop_integer32 (st_processor *pr)
{
    st_oop object = ST_STACK_POP (pr);
 
    if (ST_LIKELY (st_object_is_smi (object)))
	return st_smi_value (object);
    else if (st_object_class (object) == st_large_integer_class)
	return (st_smi) mp_get_int (st_large_integer_value (object));

    ST_PRIMITIVE_FAIL (pr);
    return 0;
}

static void
SmallInteger_add (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = st_smi_new (x + y);
	ST_STACK_PUSH (pr, result);
	return;
    }

    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_sub (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = st_smi_new (x - y);
	ST_STACK_PUSH (pr, result);
	return;
    }

    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_lt (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = (x < y) ? st_true : st_false;
	ST_STACK_PUSH (pr, result);
	return;
    }

    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_gt (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = (x > y) ? st_true : st_false;
	ST_STACK_PUSH (pr, result);
	return;
    }

    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_le (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = (x <= y) ? st_true : st_false;
	ST_STACK_PUSH (pr, result);
	return;
    }

    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_ge (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = (x >= y) ? st_true : st_false;
	ST_STACK_PUSH (pr, result);
	return;
    }
   
   ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_eq (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = (x == y) ? st_true : st_false;
	ST_STACK_PUSH (pr, result);
	return;
    }

    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_ne (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = (x != y) ? st_true : st_false;
	ST_STACK_PUSH (pr, result);
	return;
    }

    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_mul (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = st_smi_new (x * y);
	ST_STACK_PUSH (pr, result);
	return;
    }

    ST_STACK_UNPOP (pr, 2);
}

/* selector: / */
static void
SmallInteger_div (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;
    
    if (ST_LIKELY (pr->success)) {

	if (y != 0 && x % y == 0) {
	    result = st_smi_new (x / y);
	    ST_STACK_PUSH (pr, result);
	    return;
	} else {
	    ST_PRIMITIVE_FAIL (pr);
	}
    }
    
    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_intDiv (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {

	if (y != 0) {
	    result = st_smi_new (x / y);
	    ST_STACK_PUSH (pr, result);
	    return;
	} else {
	    ST_PRIMITIVE_FAIL (pr);
	}
    }
    
    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_mod (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;
    
    if (ST_LIKELY (pr->success)) {
	result = st_smi_new (x % y);
	ST_STACK_PUSH (pr, result);
	return;	
    }
    
    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_bitOr (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result = st_nil;

    if (ST_LIKELY (pr->success)) {
	result = st_smi_new (x | y);
	ST_STACK_PUSH (pr, result);
	return;	
    }
    
    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_bitXor (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result;

    if (ST_LIKELY (pr->success)) {
	result = st_smi_new (x ^ y);
	ST_STACK_PUSH (pr, result);
	return;	
    }
    
    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_bitAnd (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result = st_nil;

    if (ST_LIKELY (pr->success)) {
	result = st_smi_new (x & y);
	ST_STACK_PUSH (pr, result);
	return;	
    }
    
    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_bitShift (st_processor *pr)
{
    st_smi y = pop_integer (pr);
    st_smi x = pop_integer (pr);
    st_oop result = st_nil;

    if (ST_LIKELY (pr->success)) {
	if (y > 0)
	    result = st_smi_new (x << y);
	else if (y < 0)
	    result = st_smi_new (x >> (-y));
	else
	    result = st_smi_new (x);

	ST_STACK_PUSH (pr, result);
	return;
    }

    ST_STACK_UNPOP (pr, 2);
}

static void
SmallInteger_asFloat (st_processor *pr)
{
    st_smi x = pop_integer (pr);
    st_oop result = st_nil;

    if (ST_LIKELY (pr->success)) {
	result = st_float_new ((double) x);
	ST_STACK_PUSH (pr, result);
	return;	
    }
    
    ST_STACK_UNPOP (pr, 1);
}

static void
SmallInteger_asLargeInteger (st_processor *pr)
{
    st_smi receiver = pop_integer (pr);
    mp_int value;
    st_oop result;

    mp_init_set (&value, abs (receiver));

    if (receiver < 0)
	mp_neg (&value, &value);
    
    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

#define VALUE(oop) (&(ST_LARGE_INTEGER(oop)->value))

/* useful macros to avoid duplication of error-handling code */

#define OP_PROLOGUE			 \
    mp_int value;                        \
    mp_init (&value);


#define BINARY_OP(op, a, b)                     \
OP_PROLOGUE					\
    result = op (VALUE (a), VALUE (b), &value);

#define BINARY_DIV_OP(op, a, b)                       \
OP_PROLOGUE                                           \
    result = op (VALUE (a), VALUE (b), &value, NULL);

#define UNARY_OP(op, a)              \
OP_PROLOGUE                          \
    result = op (VALUE (a), &value);


static inline st_oop
pop_large_integer (st_processor *pr)
{
    st_oop object = ST_STACK_POP (pr);

    set_success (pr, st_object_class (object) == st_large_integer_class);
    
    return object;
}

static void
LargeInteger_add (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr); 
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    BINARY_OP (mp_add, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_sub (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    BINARY_OP (mp_sub, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_mul (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    BINARY_OP (mp_mul, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_div (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    mp_int quotient, remainder;
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    mp_init_multi (&quotient, &remainder, NULL);
    mp_div (VALUE (a), VALUE (b), &quotient, &remainder);

    int size;
    char *str;

    mp_radix_size (&remainder, 10, &size);
    str  = st_malloc (size);
    mp_toradix (&remainder, str, 10);

    if (mp_cmp_d (&remainder, 0) == MP_EQ) {
	result = st_large_integer_new (&quotient);
	ST_STACK_PUSH (pr, result);
	mp_clear (&remainder);
    } else {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 2);
	mp_clear_multi (&quotient, &remainder, NULL);
    }
}

static void
LargeInteger_intDiv (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    BINARY_DIV_OP (mp_div, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_mod (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    BINARY_OP (mp_mod, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_gcd (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    BINARY_OP (mp_gcd, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_lcm (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    BINARY_OP (mp_lcm, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_eq (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    int    relation;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_EQ) ? st_true : st_false;
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_ne (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    int    relation;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_EQ) ? st_false : st_true;
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_lt (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    int    relation;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    relation = mp_cmp (VALUE (a), VALUE (b));    
    result = (relation == MP_LT) ? st_true : st_false;
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_gt (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);

    st_oop result;
    int    relation;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_GT) ? st_true : st_false;
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_le (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    int    relation;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_LT || relation == MP_EQ) ? st_true : st_false;
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_ge (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    int    relation;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_GT || relation == MP_EQ) ? st_true : st_false;
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_squared (st_processor *pr)
{
    st_oop receiver = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 1);
	return;
    }
    
    UNARY_OP (mp_sqr, receiver);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_bitOr (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    BINARY_OP (mp_or, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_bitAnd (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    BINARY_OP (mp_and, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_bitXor (st_processor *pr)
{
    st_oop b = pop_large_integer (pr);
    st_oop a = pop_large_integer (pr);
    st_oop result;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    BINARY_OP (mp_xor, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_bitShift (st_processor *pr)
{
    st_smi displacement = pop_integer32 (pr);
    st_oop receiver     = pop_large_integer (pr);
    st_oop result;
    mp_int value;
    
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    mp_init (&value);

    if (displacement >= 0)
	mp_mul_2d (VALUE (receiver), displacement, &value);
    else
	mp_div_2d (VALUE (receiver), abs (displacement), &value, NULL);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (pr, result);
}

static void
LargeInteger_asFloat (st_processor *pr)
{
    st_oop receiver = pop_large_integer (pr);
    char  *string;
    double dblval;

    string = st_large_integer_to_string (receiver, 10);

    dblval = strtod (string, NULL);
    st_free (string);

    ST_STACK_PUSH (pr, st_float_new (dblval));
}

static void
LargeInteger_printString (st_processor *pr)
{
    st_smi radix = pop_integer (pr);
    st_oop x     = pop_large_integer (pr);
    char   *string;
    st_oop result;

    if (radix < 2 || radix > 36)
	set_success (pr, false);

    if (pr->success) {
	string = st_large_integer_to_string (x, radix);
	result = st_string_new (string);
    }

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
LargeInteger_hash (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    mp_int *value;
    st_smi result;
    const char *c;
    unsigned int hash;
    int len;

    value = st_large_integer_value (receiver);
    c = (const char *) value->dp;
    len = value->used * sizeof (mp_digit);
    hash = 5381;

    for(unsigned int i = 0; i < len; i++)
	if (c[i])
	    hash = ((hash << 5) + hash) + c[i];

    result = hash;

    if (result < 0)
	result = -result;

    ST_STACK_PUSH (pr, st_smi_new (result));
}


static inline st_oop
pop_float (st_processor *pr)
{
    st_oop object = ST_STACK_POP (pr);

    set_success (pr, st_object_class (object) == st_float_class);
    
    return object;
}

static void
Float_add (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    if (pr->success)
	result = st_float_new (st_float_value (x) + st_float_value (y));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_sub (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    if (pr->success)
	result = st_float_new (st_float_value (x) - st_float_value (y));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_lt (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    if (pr->success)
	result = isless (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_gt (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    if (pr->success)
	result = isgreater (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_le (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    if (pr->success)
	result = islessequal (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_ge (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    if (pr->success)
	result = isgreaterequal (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_eq (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    if (pr->success)
	result = (st_float_value (x) == st_float_value (y)) ? st_true : st_false;

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_ne (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    if (pr->success)
	result = (st_float_value (x) != st_float_value (y)) ? st_true : st_false;

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_mul (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    if (pr->success)
	result = st_float_new (st_float_value (x) * st_float_value (y));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_div (st_processor *pr)
{
    st_oop y = pop_float (pr);
    st_oop x = pop_float (pr);
    st_oop result = st_nil;

    set_success (pr, y != 0);

    if (pr->success)
	result = st_float_new (st_float_value (x) / st_float_value (y));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
Float_sin (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (sin (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_cos (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (cos (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_tan (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (tan (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_arcSin (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (asin (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_arcCos (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (acos (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_arcTan (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (atan (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_sqrt (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (sqrt (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_log (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (log10 (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_ln (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (log (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_exp (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (exp (value));

    if (pr->success)
	ST_STACK_PUSH (pr, result);
    else
	ST_STACK_UNPOP (pr, 1);
}

static void
Float_truncated (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_smi result;

    result = (st_smi) trunc (st_float_value (receiver));

    ST_STACK_PUSH (pr, st_smi_new (result));
}

static void
Float_fractionPart (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    double frac_part, int_part;
    st_oop result;

    frac_part = modf (st_float_value (receiver), &int_part);

    result = st_float_new (frac_part);

    ST_STACK_PUSH (pr, result);
}

static void
Float_integerPart (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    double int_part;
    st_oop result;

    modf (st_float_value (receiver), &int_part);

    result = st_smi_new ((st_smi) int_part);
    ST_STACK_PUSH (pr, result);
}

static void
Float_hash (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    unsigned int hash = 0;
    st_smi result;
    double value;
    unsigned char *c;
    
    value = st_float_value (receiver);

    if (value == 0)
	value = fabs (value);

    c = (unsigned char *) & value;
    for (int i = 0; i < sizeof (double); i++) {
	hash = (hash * 971) ^ c[i];
    }

    result = hash;

    if (result < 0)
	result = -result;

    ST_STACK_PUSH (pr, st_smi_new (result));
}

static void
print_backtrace (st_processor *pr)
{
    st_oop context;
    
    context = pr->context;

    while (context != st_nil) {

	char *selector;
	char *class;
	st_oop home;
	st_oop receiver;

	if (st_object_class (context) == st_block_context_class)
	    home = ST_BLOCK_CONTEXT_HOME (context);
	else
	    home = context;

	receiver = ST_METHOD_CONTEXT_RECEIVER (home);

	selector = (char*) st_byte_array_bytes (ST_METHOD_SELECTOR (ST_METHOD_CONTEXT_METHOD (home)));
  
	if (st_object_class (st_object_class (receiver)) == st_metaclass_class)
	    class = st_strdup_printf ("%s class", (char *) st_byte_array_bytes (ST_CLASS (receiver)->name));
	else
	    class = (char*) st_byte_array_bytes (ST_CLASS (st_object_class (receiver))->name);    

	printf ("%s>>#%s", class, selector);
	if (st_object_class (context) == st_block_context_class)
	    printf ("[]\n");
	else
	    printf ("\n");

	if (st_object_class (context) == st_block_context_class)
	    context = ST_BLOCK_CONTEXT_CALLER (context);
	else
	    context = ST_CONTEXT_PART_SENDER (context);
    }
}

static void
Object_error (st_processor *pr)
{
    st_oop message;

    message = ST_STACK_POP (pr);

    printf ("= An error occurred during program execution\n");
    printf ("= %s\n", st_byte_array_bytes (message));

    printf ("\nTraceback:\n");
    print_backtrace (pr); 

    exit (1);
}

static void
Object_class (st_processor *pr)
{
    st_oop object;

    object = ST_STACK_POP (pr);

    ST_STACK_PUSH (pr, st_object_class (object));
}

static void
Object_identityHash (st_processor *pr)
{
    st_oop object;
    st_oop result;
    
    object = ST_STACK_POP (pr);
    
    if (st_object_is_heap (object))
	result = ST_HEADER (object)->hash;
    else if (st_object_is_smi (object))
	result = st_smi_new (st_smi_hash (object));
    else
	result = st_smi_new (st_character_hash (object));
    
    ST_STACK_PUSH (pr, result);
}

static void
Object_copy (st_processor *pr)
{
    st_oop receiver;
    st_oop copy;
    st_oop class;
    st_smi size;

    (void) ST_STACK_POP (pr);

    if (!st_object_is_heap (pr->message_receiver)) {
	ST_STACK_PUSH (pr, pr->message_receiver);
	return;
    }

    switch (st_object_format (pr->message_receiver)) {

    case ST_FORMAT_OBJECT:
    {
	class = ST_HEADER (pr->message_receiver)->class;
	size = st_smi_value (ST_BEHAVIOR_INSTANCE_SIZE (class));
	copy = st_object_new (class);
	st_oops_copy (ST_HEADER (copy)->fields,
		      ST_HEADER (pr->message_receiver)->fields,
		      size);
	break;

    }
    case ST_FORMAT_ARRAY:
    {
	size = st_smi_value (ST_ARRAYED_OBJECT (pr->message_receiver)->size);
	copy = st_object_new_arrayed (ST_HEADER (pr->message_receiver)->class, size);
	st_oops_copy (ST_ARRAY (copy)->elements,
		      ST_ARRAY (pr->message_receiver)->elements,
		      size);
	break;
    }
    case ST_FORMAT_BYTE_ARRAY:
    {
	size = st_smi_value (ST_ARRAYED_OBJECT (pr->message_receiver)->size);
	copy = st_object_new_arrayed (ST_HEADER (pr->message_receiver)->class, size);
	memcpy (st_byte_array_bytes (copy),
		st_byte_array_bytes (pr->message_receiver),
		size);
	break;
    }
    case ST_FORMAT_FLOAT_ARRAY:
    {
	size = st_smi_value (st_arrayed_object_size (pr->message_receiver));
	copy = st_object_new_arrayed (ST_HEADER (pr->message_receiver)->class, size);
	memcpy (st_float_array_elements (copy),
		st_float_array_elements (pr->message_receiver),
		sizeof (double) * size);

	break;
    }
    case ST_FORMAT_WORD_ARRAY:
    {
	size = st_smi_value (st_arrayed_object_size (pr->message_receiver));
	copy = st_object_new_arrayed (ST_HEADER (pr->message_receiver)->class, size);
	memcpy (st_word_array_elements (copy),
		st_word_array_elements (pr->message_receiver),
		sizeof (st_uint) * size);
	break;
    }
    case ST_FORMAT_FLOAT:
    {
	copy = st_object_new (st_float_class);
	st_float_set_value (copy, st_float_value (pr->message_receiver));
	break;
    }
    case ST_FORMAT_LARGE_INTEGER:
    {
	mp_int value;
	int    result;

	copy = st_large_integer_new (NULL);
	
	result = mp_init_copy (st_large_integer_value (copy),
			       st_large_integer_value (pr->message_receiver));
	if (result != MP_OKAY)
	    st_assert_not_reached ();
	break;
    }
    case ST_FORMAT_CONTEXT:
    case ST_FORMAT_INTEGER_ARRAY:
    default:
	/* not implemented yet */
	st_assert_not_reached ();
    }

    ST_STACK_PUSH (pr, copy);
}

static void
Object_equivalent (st_processor *pr)
{
    st_oop y = ST_STACK_POP (pr);
    st_oop x = ST_STACK_POP (pr);
    
    ST_STACK_PUSH (pr, ((x == y) ? st_true : st_false));
}

static void
Object_perform (st_processor *pr)
{
    st_oop receiver;
    st_oop selector;
    st_oop method;
    st_uint selector_index;

    selector = pr->message_selector;
    pr->message_selector = pr->stack[pr->sp - pr->message_argcount];
    receiver = pr->message_receiver;

    set_success (pr, st_object_is_symbol (pr->message_selector));
    pr->new_method = st_processor_lookup_method (pr, st_object_class (receiver));
    set_success (pr, st_method_get_arg_count (method) == (pr->message_argcount - 1));

    if (pr->success) {
	selector_index = pr->sp - pr->message_argcount;

	st_oops_move (pr->stack + selector_index,
		      pr->stack + selector_index + 1,
		      pr->message_argcount - 1);

	pr->sp -= 1;
	pr->message_argcount -= 1;
	st_processor_execute_method (pr);

    } else {
	pr->message_selector = selector;
    }
}

static void
Object_perform_withArguments (st_processor *pr)
{
    st_oop receiver;
    st_oop selector;
    st_oop method;
    st_oop array;
    st_smi array_size;

    array = ST_STACK_POP (pr);

    set_success (pr, st_object_class (array) == st_array_class);

    if (st_object_class (pr->context) == st_block_context_class)
	method = ST_METHOD_CONTEXT_METHOD (ST_BLOCK_CONTEXT_HOME (pr->context));
    else
	method = ST_METHOD_CONTEXT_METHOD (pr->context);

    array_size = st_smi_value (st_arrayed_object_size (array));
    set_success (pr, (pr->sp + array_size - 1) < st_method_get_stack_depth (method));

    if (pr->success) {
	
	selector = pr->message_selector;
	pr->message_selector = ST_STACK_POP (pr);
	receiver = ST_STACK_PEEK (pr);
	pr->message_argcount = array_size;

	set_success (pr, st_object_is_symbol (pr->message_selector));
    
	st_oops_copy (pr->stack + pr->sp,
		      st_array_elements (array),
		      array_size);

	pr->sp += array_size;

	pr->new_method = st_processor_lookup_method (pr, st_object_class (receiver));
	set_success (pr, st_method_get_arg_count (pr->new_method) == array_size);
    
	if (pr->success) {
	    st_processor_execute_method (pr);
	} else {
	    pr->sp -= pr->message_argcount;
	    ST_STACK_PUSH (pr, pr->message_selector);
	    ST_STACK_PUSH (pr, array);
	    pr->message_argcount = 2;
	    pr->message_selector = selector;
	}

    } else {
	ST_STACK_UNPOP (pr, 1);
    }
}

static void
Behavior_new (st_processor *pr)
{
    st_oop class;
    st_oop instance;
    st_smi format;

    class = ST_STACK_POP (pr);
    format = st_smi_value (ST_BEHAVIOR_FORMAT (class));

    set_success (pr, st_descriptors[format]->allocate != NULL);

    if (!pr->success) {
	ST_STACK_UNPOP (pr, 1);
	return;
    }

    instance = st_descriptors[format]->allocate (class);
    ST_STACK_PUSH (pr, instance);
}

static void
Behavior_newSize (st_processor *pr)
{
    st_oop class;
    st_smi size;
    st_smi format;
    st_oop instance;

    size = pop_integer32 (pr);
    class = ST_STACK_POP (pr);

    format = st_smi_value (ST_BEHAVIOR_FORMAT (class));

    set_success (pr, st_descriptors[format]->allocate_arrayed != NULL);

    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    instance = st_descriptors[format]->allocate_arrayed (class, size);
    ST_STACK_PUSH (pr, instance);
}

static void
SequenceableCollection_size (st_processor *pr)
{
    st_oop object;

    object = ST_STACK_POP (pr);

    ST_STACK_PUSH (pr, st_arrayed_object_size (object));
}

static void
Array_at (st_processor *pr)
{
    st_smi index    = pop_integer (pr);
    st_oop receiver = ST_STACK_POP (pr);
    
    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    ST_STACK_PUSH (pr, st_array_at (receiver, index));
}

static void
Array_at_put (st_processor *pr)
{
    st_oop object   = ST_STACK_POP (pr);
    st_smi index    = pop_integer32 (pr);
    st_oop receiver = ST_STACK_POP (pr);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 3);
	return;
    }
    
    st_array_at_put (receiver, index, object);	    
    ST_STACK_PUSH (pr, object);
}

static void
ByteArray_at (st_processor *pr)
{
    st_smi index    = pop_integer32 (pr);
    st_oop receiver = ST_STACK_POP (pr);
    st_oop result;
	
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    result = st_smi_new (st_byte_array_at (receiver, index));  
    
    ST_STACK_PUSH (pr, result);
}

static void
ByteArray_at_put (st_processor *pr)
{
    st_smi byte     = pop_integer (pr);
    st_smi index    = pop_integer32 (pr);
    st_oop receiver = ST_STACK_POP (pr);

    if (!pr->success) {
	ST_STACK_UNPOP (pr, 3);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 3);
	return;
    }
    
    st_byte_array_at_put (receiver, index, byte);	    
    
    ST_STACK_PUSH (pr, st_smi_new (byte));
}

static void
ByteArray_hash (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);
    st_uint  hash;

    hash = st_byte_array_hash (receiver);

    ST_STACK_PUSH (pr, st_smi_new (hash));   
}

static void
ByteString_at (st_processor *pr)
{
    st_smi  index    = pop_integer32 (pr);
    st_oop  receiver = ST_STACK_POP (pr);
    st_oop  character;
    char   *charptr;

    if (ST_UNLIKELY (!pr->success)) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    character = st_character_new (st_byte_array_at (receiver, index));

    ST_STACK_PUSH (pr, character);
}

static void
ByteString_at_put (st_processor *pr)
{
    st_oop character = ST_STACK_POP (pr);
    st_smi index    = pop_integer32 (pr);
    st_oop receiver = ST_STACK_POP (pr);
	
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 3);
	return;
    }
  
    set_success (pr, st_object_class (character) == st_character_class);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 3);
	return;
    }

    st_byte_array_at_put (receiver, index, (st_uchar) st_character_value (character));

    ST_STACK_PUSH (pr, character);
}


static void
ByteString_size (st_processor *pr)
{
    st_oop receiver;
    st_uint size;

    receiver = ST_STACK_POP (pr);

    size = st_arrayed_object_size (receiver);

    /* TODO: allow size to go into a LargeInteger on overflow */
    ST_STACK_PUSH (pr, size);
}

static void
ByteString_compare (st_processor *pr)
{
    st_oop argument = ST_STACK_POP (pr);
    st_oop receiver = ST_STACK_POP (pr);
    int order;

    if (st_object_format (argument) != ST_FORMAT_BYTE_ARRAY)
	set_success (pr, false);

    if (pr->success)
	order = strcmp ((const char *) st_byte_array_bytes (receiver),
			(const char *) st_byte_array_bytes (argument));

    if (pr->success)
	ST_STACK_PUSH (pr, st_smi_new (order));
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
WideString_at (st_processor *pr)
{
    st_smi index    = pop_integer32 (pr);
    st_oop receiver = ST_STACK_POP (pr);
    st_uchar *bytes;
    st_unichar c;
	
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    if (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    c = st_word_array_at (receiver, index);

    ST_STACK_PUSH (pr, st_character_new (c));
}

static void
WideString_at_put (st_processor *pr)
{
    st_oop character = ST_STACK_POP (pr);
    st_smi index    = pop_integer32 (pr);
    st_oop receiver = ST_STACK_POP (pr);
    st_uchar *bytes;
    st_unichar c;	

    if (!pr->success) {
	ST_STACK_UNPOP (pr, 3);
	return;
    }
  
    set_success (pr, st_object_class (character) == st_character_class);

    if (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 3);
	return;
    }

    st_word_array_at_put (receiver, index, character);

    ST_STACK_PUSH (pr, character);
}

static void
WordArray_at (st_processor *pr)
{
    st_oop receiver;
    st_smi index;
    st_uint  element;

    index = pop_integer32 (pr);
    receiver = ST_STACK_POP (pr);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 2);
	return;
    }
    
    element = st_word_array_at (receiver, index);

    ST_STACK_PUSH (pr, st_smi_new (element));
}

static void
WordArray_at_put (st_processor *pr)
{
    st_smi value    = pop_integer (pr);
    st_smi index    = pop_integer32 (pr);
    st_oop receiver = ST_STACK_POP (pr);
	
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 3);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 3);
	return;
    }

    st_word_array_at_put (receiver, index, value);

    ST_STACK_PUSH (pr, st_smi_new (value));
}

static void
FloatArray_at (st_processor *pr)
{
    st_oop receiver;
    st_smi index;
    double  element;

    index = pop_integer32 (pr);
    receiver = ST_STACK_POP (pr);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    element = st_float_array_at (receiver, index);
    ST_STACK_PUSH (pr, st_float_new (element));
}

static void
FloatArray_at_put (st_processor *pr)
{
    st_oop flt      = ST_STACK_POP (pr);
    st_smi index    = pop_integer32 (pr);
    st_oop receiver = ST_STACK_POP (pr);

    set_success (pr, st_object_is_heap (flt) &&
		 st_object_format (flt) == ST_FORMAT_FLOAT);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (pr, false);
	ST_STACK_UNPOP (pr, 3);
	return;
    }
	
    if (!pr->success) {
	ST_STACK_UNPOP (pr, 3);
	return;
    }

    st_float_array_at_put (receiver, index, st_float_value (flt));
    ST_STACK_PUSH (pr, flt);
}

static inline void
activate_block_context (st_processor *pr)
{
    st_oop  block;
    st_smi  argcount;

    block = pr->message_receiver;
    argcount = st_smi_value (ST_BLOCK_CONTEXT_ARGCOUNT (block));
    if (argcount != pr->message_argcount) {
	pr->success = false;
	return;
    }

    st_oops_copy (ST_BLOCK_CONTEXT_STACK (block),
		  pr->stack + pr->sp - argcount,
		  argcount);

    pr->sp -= pr->message_argcount + 1;
    
    ST_CONTEXT_PART_IP (block) = ST_BLOCK_CONTEXT_INITIALIP (block);
    ST_CONTEXT_PART_SP (block) = st_smi_new (argcount);
    ST_BLOCK_CONTEXT_CALLER (block) = pr->context;

    st_processor_set_active_context (pr, block);
}

static void
BlockContext_value (st_processor *pr)
{
    activate_block_context (pr);
}

static void
BlockContext_valueWithArguments (st_processor *pr)
{
    st_oop block;
    st_oop values;
    st_smi argcount;

    block  = pr->message_receiver;
    values = ST_STACK_PEEK (pr);

    if (st_object_class (values) != st_array_class) {
	set_success (pr, false);
	return;
    }

    argcount = st_smi_value (ST_BLOCK_CONTEXT_ARGCOUNT (block));
    if (argcount != st_smi_value (st_arrayed_object_size (values))) {
	set_success (pr, false);
	return;
    }
    
    st_oops_copy (ST_BLOCK_CONTEXT_STACK (block),
		  ST_ARRAY (values)->elements,
		  argcount);
    
    pr->sp -= pr->message_argcount + 1;

    ST_CONTEXT_PART_IP (block) = ST_BLOCK_CONTEXT_INITIALIP (block);
    ST_CONTEXT_PART_SP (block) = st_smi_new (argcount);
    ST_BLOCK_CONTEXT_CALLER (block) = pr->context;

    st_processor_set_active_context (pr, block);
}

static void
UndefinedObject_exitWithResult (st_processor *pr)
{
    longjmp (pr->main_loop, 0);
}

static void
Character_value (st_processor *pr)
{
    st_oop receiver = ST_STACK_POP (pr);

    ST_STACK_PUSH (pr, st_smi_new (st_character_value (receiver)));
}

static void
Character_characterFor (st_processor *pr)
{
    st_oop receiver;
    st_smi value;

    value = pop_integer (pr);
    receiver = ST_STACK_POP (pr);

    if (pr->success)
	ST_STACK_PUSH (pr, st_character_new (value));
    else
	ST_STACK_UNPOP (pr, 2);
}

static void
FileStream_open (st_processor *pr)
{
    int fd;
    int mode;
    st_oop name;
    st_oop mode_oop;
    char *str;

    mode_oop = ST_STACK_POP (pr);
    name =  ST_STACK_POP (pr);

    if (st_object_class (mode_oop) != st_symbol_class) {
	ST_PRIMITIVE_FAIL (pr);
	return;
    }

    if (st_object_class (name) != st_string_class) {
	ST_PRIMITIVE_FAIL (pr);
	return;
    }
    
    str = st_byte_array_bytes (mode_oop);
    if (streq (str, "read"))
	mode = O_RDONLY;
    else if (streq (str, "write"))
	mode = O_WRONLY;
    else if (streq (str, "readWrite"))
	mode = O_RDWR;
    else {
	ST_PRIMITIVE_FAIL (pr);
	return;
    }

    str = st_byte_array_bytes (name);
    fd = open (str, O_CREAT | mode, 0644);
    ST_STACK_PUSH (pr, st_smi_new (fd));
}

static void
FileStream_close (st_processor *pr)
{
    int fd;
    int byte;

    fd = pop_integer (pr);
    (void) ST_STACK_POP (pr);

    if (!pr->success) {
	ST_STACK_UNPOP (pr, 2);
	return;
    }

    if (close (fd) != 0) {
	ST_STACK_UNPOP (pr, 2);
	ST_PRIMITIVE_FAIL (pr);
    }

    ST_STACK_PUSH (pr, pr->message_receiver);
}

static void
FileStream_write (st_processor *pr)
{
    int fd;
    int byte;

    byte = pop_integer (pr);
    fd = pop_integer (pr);
    (void) ST_STACK_POP (pr);

    if (!pr->success) {
	ST_PRIMITIVE_FAIL (pr);
	ST_STACK_UNPOP (pr, 3);	
    }

    if (write (fd, &byte, 1) < 0) {
	ST_PRIMITIVE_FAIL (pr);
	ST_STACK_UNPOP (pr, 3);	
    }

    ST_STACK_PUSH (pr, pr->message_receiver);
}

static void
FileStream_read (st_processor *pr)
{

}

static void
FileStream_writeN (st_processor *pr)
{
    st_oop array;
    int fd;
    st_uint size;
    char *buf;

    array = ST_STACK_POP (pr);
    fd = pop_integer (pr);

    if (st_object_format (array) != ST_FORMAT_BYTE_ARRAY) {
	ST_PRIMITIVE_FAIL (pr);
	return;
    }

    if (!pr->success)
	return;

    size = st_smi_value (st_arrayed_object_size (array));
    buf = st_byte_array_bytes (array);

    write (fd, buf, size);

    ST_STACK_PUSH (pr, st_true);
}

static void
FileStream_readN (st_processor *pr)
{

}

static void
FileStream_position (st_processor *pr)
{

}

static void
FileStream_setPosition (st_processor *pr)
{

}

const struct st_primitive st_primitives[] = {
    { "SmallInteger_add",      SmallInteger_add      },
    { "SmallInteger_sub",      SmallInteger_sub      },
    { "SmallInteger_lt",       SmallInteger_lt       },
    { "SmallInteger_gt",       SmallInteger_gt       },
    { "SmallInteger_le",       SmallInteger_le       },
    { "SmallInteger_ge",       SmallInteger_ge       },
    { "SmallInteger_eq",       SmallInteger_eq       },
    { "SmallInteger_ne",       SmallInteger_ne       },
    { "SmallInteger_mul",      SmallInteger_mul      },
    { "SmallInteger_div",      SmallInteger_div      },
    { "SmallInteger_intDiv",   SmallInteger_intDiv   },
    { "SmallInteger_mod",      SmallInteger_mod      },
    { "SmallInteger_bitOr",    SmallInteger_bitOr    },
    { "SmallInteger_bitXor",   SmallInteger_bitXor   },
    { "SmallInteger_bitAnd",   SmallInteger_bitAnd   },
    { "SmallInteger_bitShift", SmallInteger_bitShift },
    { "SmallInteger_asFloat",         SmallInteger_asFloat  },
    { "SmallInteger_asLargeInteger",  SmallInteger_asLargeInteger  },

    { "LargeInteger_add",      LargeInteger_add      },
    { "LargeInteger_sub",      LargeInteger_sub      },
    { "LargeInteger_lt",       LargeInteger_lt       },
    { "LargeInteger_gt",       LargeInteger_gt       },
    { "LargeInteger_le",       LargeInteger_le       },
    { "LargeInteger_ge",       LargeInteger_ge       },
    { "LargeInteger_eq",       LargeInteger_eq       },
    { "LargeInteger_ne",       LargeInteger_ne       },
    { "LargeInteger_mul",      LargeInteger_mul      },
    { "LargeInteger_div",      LargeInteger_div      },
    { "LargeInteger_intDiv",   LargeInteger_intDiv   },
    { "LargeInteger_mod",      LargeInteger_mod      },
    { "LargeInteger_gcd",      LargeInteger_gcd      },
    { "LargeInteger_lcm",      LargeInteger_lcm      },
    { "LargeInteger_squared",  LargeInteger_squared  },
    { "LargeInteger_bitOr",    LargeInteger_bitOr    },
    { "LargeInteger_bitXor",   LargeInteger_bitXor   },
    { "LargeInteger_bitAnd",   LargeInteger_bitAnd   },
    { "LargeInteger_bitShift", LargeInteger_bitShift },
    { "LargeInteger_printString", LargeInteger_printString   },
    { "LargeInteger_asFloat",     LargeInteger_asFloat   },
    { "LargeInteger_hash",     LargeInteger_hash   },

    { "Float_add",             Float_add           },
    { "Float_sub",             Float_sub           },
    { "Float_lt",              Float_lt            },
    { "Float_gt",              Float_gt            },
    { "Float_le",              Float_le            },
    { "Float_ge",              Float_ge            },
    { "Float_eq",              Float_eq            },
    { "Float_ne",              Float_ne            },
    { "Float_mul",             Float_mul           },
    { "Float_div",             Float_div           },
    { "Float_exp",             Float_exp           },
    { "Float_sin",             Float_sin           },
    { "Float_cos",             Float_cos           },
    { "Float_tan",             Float_tan           },
    { "Float_arcSin",          Float_arcSin        },
    { "Float_arcCos",          Float_arcCos        },
    { "Float_arcTan",          Float_arcTan        },
    { "Float_ln",              Float_ln            },
    { "Float_log",             Float_log           },
    { "Float_sqrt",            Float_sqrt          },
    { "Float_truncated",       Float_truncated     },
    { "Float_fractionPart",    Float_fractionPart  },
    { "Float_integerPart",     Float_integerPart   },
    { "Float_hash",            Float_hash          },

    { "Object_error",                  Object_error                },
    { "Object_class",                  Object_class                },
    { "Object_identityHash",           Object_identityHash         },
    { "Object_copy",                   Object_copy                 },
    { "Object_equivalent",             Object_equivalent           },
    { "Object_perform",                Object_perform               },
    { "Object_perform_withArguments",  Object_perform_withArguments },
    
    { "Behavior_new",                 Behavior_new                },
    { "Behavior_newSize",             Behavior_newSize            },


    { "SequenceableCollection_size",   SequenceableCollection_size },           

    { "Array_at",                      Array_at                    },
    { "Array_at_put",                  Array_at_put                },

    { "ByteArray_at",                  ByteArray_at                },
    { "ByteArray_at_put",              ByteArray_at_put            },
    { "ByteArray_hash",                ByteArray_hash              },

    { "ByteString_at",                 ByteString_at               },
    { "ByteString_at_put",             ByteString_at_put           },
    { "ByteString_size",               ByteString_size             },
    { "ByteString_compare",            ByteString_compare          },

    { "WideString_at",                 WideString_at               },
    { "WideString_at_put",             WideString_at_put           },

    { "WordArray_at",                  WordArray_at                },
    { "WordArray_at_put",              WordArray_at_put            },

    { "FloatArray_at",                 FloatArray_at               },
    { "FloatArray_at_put",             FloatArray_at_put           },

    { "UndefinedObject_exitWithResult", UndefinedObject_exitWithResult },

    { "Character_value",                Character_value },
    { "Character_characterFor",         Character_characterFor },

    { "BlockContext_value",               BlockContext_value               },
    { "BlockContext_valueWithArguments",  BlockContext_valueWithArguments  },

    { "FileStream_open", FileStream_open },
    { "FileStream_close", FileStream_close },
    { "FileStream_write", FileStream_write },
    { "FileStream_read", FileStream_read },
    { "FileStream_writeN", FileStream_writeN },
    { "FileStream_readN", FileStream_readN },
    { "FileStream_position", FileStream_position },
    { "FileStream_setPosition", FileStream_setPosition },
};

/* returns 0 if there no primitive function corresponding
 * to the given name */
int
st_primitive_index_for_name (const char *name)
{
    st_assert (name != NULL);
    for (int i = 0; i < ST_N_ELEMENTS (st_primitives); i++)
	if (streq (name, st_primitives[i].name))
	    return i;
    return -1;
}

