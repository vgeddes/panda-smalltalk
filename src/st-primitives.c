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
#include "st-dictionary.h"
#include "st-unicode.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <fcntl.h>
#include <unistd.h>


#define ST_PRIMITIVE_FAIL(cpu)			\
    cpu->success = false


static inline void
set_success (struct st_cpu *cpu, bool success)
{
    cpu->success = cpu->success && success;
}

static inline st_smi
pop_integer (struct st_cpu *cpu)
{
    st_oop object = ST_STACK_POP (cpu);
  
    if (ST_LIKELY (st_object_is_smi (object)))
	return st_smi_value (object);	

    ST_PRIMITIVE_FAIL (cpu);
    return 0;
}

static inline st_smi
pop_integer32 (struct st_cpu *cpu)
{
    st_oop object = ST_STACK_POP (cpu);
 
    if (ST_LIKELY (st_object_is_smi (object)))
	return st_smi_value (object);
    else if (st_object_class (object) == st_large_integer_class)
	return (st_smi) mp_get_int (st_large_integer_value (object));

    ST_PRIMITIVE_FAIL (cpu);
    return 0;
}

static void
SmallInteger_add (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = st_smi_new (x + y);
	ST_STACK_PUSH (cpu, result);
	return;
    }

    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_sub (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = st_smi_new (x - y);
	ST_STACK_PUSH (cpu, result);
	return;
    }

    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_lt (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = (x < y) ? st_true : st_false;
	ST_STACK_PUSH (cpu, result);
	return;
    }

    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_gt (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = (x > y) ? st_true : st_false;
	ST_STACK_PUSH (cpu, result);
	return;
    }

    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_le (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = (x <= y) ? st_true : st_false;
	ST_STACK_PUSH (cpu, result);
	return;
    }

    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_ge (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = (x >= y) ? st_true : st_false;
	ST_STACK_PUSH (cpu, result);
	return;
    }
   
   ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_eq (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = (x == y) ? st_true : st_false;
	ST_STACK_PUSH (cpu, result);
	return;
    }

    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_ne (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = (x != y) ? st_true : st_false;
	ST_STACK_PUSH (cpu, result);
	return;
    }

    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_mul (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = st_smi_new (x * y);
	ST_STACK_PUSH (cpu, result);
	return;
    }

    ST_STACK_UNPOP (cpu, 2);
}

/* selector: / */
static void
SmallInteger_div (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;
    
    if (ST_LIKELY (cpu->success)) {

	if (y != 0 && x % y == 0) {
	    result = st_smi_new (x / y);
	    ST_STACK_PUSH (cpu, result);
	    return;
	} else {
	    ST_PRIMITIVE_FAIL (cpu);
	}
    }
    
    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_intDiv (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {

	if (y != 0) {
	    result = st_smi_new (x / y);
	    ST_STACK_PUSH (cpu, result);
	    return;
	} else {
	    ST_PRIMITIVE_FAIL (cpu);
	}
    }
    
    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_mod (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;
    
    if (ST_LIKELY (cpu->success)) {
	result = st_smi_new (x % y);
	ST_STACK_PUSH (cpu, result);
	return;	
    }
    
    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_bitOr (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result = st_nil;

    if (ST_LIKELY (cpu->success)) {
	result = st_smi_new (x | y);
	ST_STACK_PUSH (cpu, result);
	return;	
    }
    
    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_bitXor (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result;

    if (ST_LIKELY (cpu->success)) {
	result = st_smi_new (x ^ y);
	ST_STACK_PUSH (cpu, result);
	return;	
    }
    
    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_bitAnd (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result = st_nil;

    if (ST_LIKELY (cpu->success)) {
	result = st_smi_new (x & y);
	ST_STACK_PUSH (cpu, result);
	return;	
    }
    
    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_bitShift (struct st_cpu *cpu)
{
    st_smi y = pop_integer (cpu);
    st_smi x = pop_integer (cpu);
    st_oop result = st_nil;

    if (ST_LIKELY (cpu->success)) {
	if (y > 0)
	    result = st_smi_new (x << y);
	else if (y < 0)
	    result = st_smi_new (x >> (-y));
	else
	    result = st_smi_new (x);

	ST_STACK_PUSH (cpu, result);
	return;
    }

    ST_STACK_UNPOP (cpu, 2);
}

static void
SmallInteger_asFloat (struct st_cpu *cpu)
{
    st_smi x = pop_integer (cpu);
    st_oop result = st_nil;

    if (ST_LIKELY (cpu->success)) {
	result = st_float_new ((double) x);
	ST_STACK_PUSH (cpu, result);
	return;	
    }
    
    ST_STACK_UNPOP (cpu, 1);
}

static void
SmallInteger_asLargeInteger (struct st_cpu *cpu)
{
    st_smi receiver = pop_integer (cpu);
    mp_int value;
    st_oop result;

    mp_init_set (&value, abs (receiver));

    if (receiver < 0)
	mp_neg (&value, &value);
    
    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
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
pop_large_integer (struct st_cpu *cpu)
{
    st_oop object = ST_STACK_POP (cpu);

    set_success (cpu, st_object_class (object) == st_large_integer_class);
    
   return object;
}

static void
LargeInteger_add (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu); 
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    BINARY_OP (mp_add, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_sub (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    BINARY_OP (mp_sub, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_mul (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    BINARY_OP (mp_mul, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_div (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    mp_int quotient, remainder;
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
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
	ST_STACK_PUSH (cpu, result);
	mp_clear (&remainder);
    } else {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 2);
	mp_clear_multi (&quotient, &remainder, NULL);
    }
}

static void
LargeInteger_intDiv (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    BINARY_DIV_OP (mp_div, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_mod (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    BINARY_OP (mp_mod, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_gcd (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    BINARY_OP (mp_gcd, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_lcm (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    BINARY_OP (mp_lcm, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_eq (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    int    relation;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_EQ) ? st_true : st_false;
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_ne (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    int    relation;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_EQ) ? st_false : st_true;
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_lt (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    int    relation;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    relation = mp_cmp (VALUE (a), VALUE (b));    
    result = (relation == MP_LT) ? st_true : st_false;
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_gt (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);

    st_oop result;
    int    relation;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_GT) ? st_true : st_false;
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_le (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    int    relation;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_LT || relation == MP_EQ) ? st_true : st_false;
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_ge (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    int    relation;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_GT || relation == MP_EQ) ? st_true : st_false;
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_squared (struct st_cpu *cpu)
{
    st_oop receiver = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 1);
	return;
    }
    
    UNARY_OP (mp_sqr, receiver);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_bitOr (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    BINARY_OP (mp_or, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_bitAnd (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    BINARY_OP (mp_and, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_bitXor (struct st_cpu *cpu)
{
    st_oop b = pop_large_integer (cpu);
    st_oop a = pop_large_integer (cpu);
    st_oop result;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    BINARY_OP (mp_xor, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_bitShift (struct st_cpu *cpu)
{
    st_smi displacement = pop_integer32 (cpu);
    st_oop receiver     = pop_large_integer (cpu);
    st_oop result;
    mp_int value;
    
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    mp_init (&value);

    if (displacement >= 0)
	mp_mul_2d (VALUE (receiver), displacement, &value);
    else
	mp_div_2d (VALUE (receiver), abs (displacement), &value, NULL);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (cpu, result);
}

static void
LargeInteger_asFloat (struct st_cpu *cpu)
{
    st_oop receiver = pop_large_integer (cpu);
    char  *string;
    double dblval;

    string = st_large_integer_to_string (receiver, 10);

    dblval = strtod (string, NULL);
    st_free (string);

    ST_STACK_PUSH (cpu, st_float_new (dblval));
}

static void
LargeInteger_printString (struct st_cpu *cpu)
{
    st_smi radix = pop_integer (cpu);
    st_oop x     = pop_large_integer (cpu);
    char   *string;
    st_oop result;

    if (radix < 2 || radix > 36)
	set_success (cpu, false);

    if (cpu->success) {
	string = st_large_integer_to_string (x, radix);
	result = st_string_new (string);
    }

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
LargeInteger_hash (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
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

    ST_STACK_PUSH (cpu, st_smi_new (result));
}


static inline st_oop
pop_float (struct st_cpu *cpu)
{
    st_oop object = ST_STACK_POP (cpu);

    set_success (cpu, st_object_class (object) == st_float_class);
    
    return object;
}

static void
Float_add (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    if (cpu->success)
	result = st_float_new (st_float_value (x) + st_float_value (y));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_sub (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    if (cpu->success)
	result = st_float_new (st_float_value (x) - st_float_value (y));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_lt (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    if (cpu->success)
	result = isless (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_gt (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    if (cpu->success)
	result = isgreater (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_le (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    if (cpu->success)
	result = islessequal (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_ge (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    if (cpu->success)
	result = isgreaterequal (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_eq (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    if (cpu->success)
	result = (st_float_value (x) == st_float_value (y)) ? st_true : st_false;

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_ne (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    if (cpu->success)
	result = (st_float_value (x) != st_float_value (y)) ? st_true : st_false;

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_mul (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    if (cpu->success)
	result = st_float_new (st_float_value (x) * st_float_value (y));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_div (struct st_cpu *cpu)
{
    st_oop y = pop_float (cpu);
    st_oop x = pop_float (cpu);
    st_oop result = st_nil;

    set_success (cpu, y != 0);

    if (cpu->success)
	result = st_float_new (st_float_value (x) / st_float_value (y));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
Float_sin (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (sin (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_cos (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (cos (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_tan (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (tan (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_arcSin (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (asin (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_arcCos (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (acos (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_arcTan (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (atan (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_sqrt (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (sqrt (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_log (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (log10 (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_ln (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (log (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_exp (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (exp (value));

    if (cpu->success)
	ST_STACK_PUSH (cpu, result);
    else
	ST_STACK_UNPOP (cpu, 1);
}

static void
Float_truncated (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_smi result;

    result = (st_smi) trunc (st_float_value (receiver));

    ST_STACK_PUSH (cpu, st_smi_new (result));
}

static void
Float_fractionPart (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    double frac_part, int_part;
    st_oop result;

    frac_part = modf (st_float_value (receiver), &int_part);

    result = st_float_new (frac_part);

    ST_STACK_PUSH (cpu, result);
}

static void
Float_integerPart (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    double int_part;
    st_oop result;

    modf (st_float_value (receiver), &int_part);

    result = st_smi_new ((st_smi) int_part);
    ST_STACK_PUSH (cpu, result);
}

static void
Float_hash (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
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

    ST_STACK_PUSH (cpu, st_smi_new (result));
}

static void
print_backtrace (struct st_cpu *cpu)
{
    st_oop context;
    
    context = cpu->context;

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
Object_error (struct st_cpu *cpu)
{
    st_oop message;

    message = ST_STACK_POP (cpu);

    printf ("= An error occurred during program execution\n");
    printf ("= %s\n", st_byte_array_bytes (message));

    printf ("\nTraceback:\n");
    print_backtrace (cpu); 

    exit (1);
}

static void
Object_class (struct st_cpu *cpu)
{
    st_oop object;

    object = ST_STACK_POP (cpu);

    ST_STACK_PUSH (cpu, st_object_class (object));
}

static void
Object_identityHash (struct st_cpu *cpu)
{
    st_oop object;
    st_oop result;
    
    object = ST_STACK_POP (cpu);
    
    if (st_object_is_heap (object))
	result = ST_OBJECT_HASH (object);
    else if (st_object_is_smi (object))
	result = st_smi_new (st_smi_hash (object));
    else
	result = st_smi_new (st_character_hash (object));
    
    ST_STACK_PUSH (cpu, result);
}

static void
Object_copy (struct st_cpu *cpu)
{
    st_oop receiver;
    st_oop copy;
    st_oop class;
    st_smi size;

    (void) ST_STACK_POP (cpu);

    if (!st_object_is_heap (cpu->message_receiver)) {
	ST_STACK_PUSH (cpu, cpu->message_receiver);
	return;
    }

    switch (st_object_format (cpu->message_receiver)) {

    case ST_FORMAT_OBJECT:
    {
	class = ST_OBJECT_CLASS (cpu->message_receiver);
	size = st_smi_value (ST_BEHAVIOR_INSTANCE_SIZE (class));
	copy = st_object_new (class);
	st_oops_copy (ST_OBJECT_FIELDS (copy),
		      ST_OBJECT_FIELDS (cpu->message_receiver),
		      size);
	break;

    }
    case ST_FORMAT_ARRAY:
    {
	size = st_smi_value (ST_ARRAYED_OBJECT (cpu->message_receiver)->size);
	copy = st_object_new_arrayed (ST_OBJECT_CLASS (cpu->message_receiver), size);
	st_oops_copy (ST_ARRAY (copy)->elements,
		      ST_ARRAY (cpu->message_receiver)->elements,
		      size);
	break;
    }
    case ST_FORMAT_BYTE_ARRAY:
    {
	size = st_smi_value (ST_ARRAYED_OBJECT (cpu->message_receiver)->size);
	copy = st_object_new_arrayed (ST_OBJECT_CLASS (cpu->message_receiver), size);
	memcpy (st_byte_array_bytes (copy),
		st_byte_array_bytes (cpu->message_receiver),
		size);
	break;
    }
    case ST_FORMAT_FLOAT_ARRAY:
    {
	size = st_smi_value (st_arrayed_object_size (cpu->message_receiver));
	copy = st_object_new_arrayed (ST_OBJECT_CLASS (cpu->message_receiver), size);
	memcpy (st_float_array_elements (copy),
		st_float_array_elements (cpu->message_receiver),
		sizeof (double) * size);

	break;
    }
    case ST_FORMAT_WORD_ARRAY:
    {
	size = st_smi_value (st_arrayed_object_size (cpu->message_receiver));
	copy = st_object_new_arrayed (ST_OBJECT_CLASS (cpu->message_receiver), size);
	memcpy (st_word_array_elements (copy),
		st_word_array_elements (cpu->message_receiver),
		sizeof (st_uint) * size);
	break;
    }
    case ST_FORMAT_FLOAT:
    {
	copy = st_object_new (st_float_class);
	st_float_set_value (copy, st_float_value (cpu->message_receiver));
	break;
    }
    case ST_FORMAT_LARGE_INTEGER:
    {
	mp_int value;
	int    result;

	copy = st_large_integer_new (NULL);
	
	result = mp_init_copy (st_large_integer_value (copy),
			       st_large_integer_value (cpu->message_receiver));
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

    ST_STACK_PUSH (cpu, copy);
}

static void
Object_equivalent (struct st_cpu *cpu)
{
    st_oop y = ST_STACK_POP (cpu);
    st_oop x = ST_STACK_POP (cpu);
    
    ST_STACK_PUSH (cpu, ((x == y) ? st_true : st_false));
}

static st_oop
lookup_method (st_oop class, st_oop selector)
{
    st_oop method;
    st_oop parent = class;
    st_uint index;

    while (parent != st_nil) {
	method = st_dictionary_at (ST_BEHAVIOR_METHOD_DICTIONARY (parent), selector);
	if (method != st_nil)
	    return method;
	parent = ST_BEHAVIOR_SUPERCLASS (parent);
    }

    return 0;
}

static void
Object_perform (struct st_cpu *cpu)
{
    st_oop receiver;
    st_oop selector;
    st_oop method;
    st_uint selector_index;

    selector = cpu->message_selector;
    cpu->message_selector = cpu->stack[cpu->sp - cpu->message_argcount];
    receiver = cpu->message_receiver;

    set_success (cpu, st_object_is_symbol (cpu->message_selector));
    method = lookup_method (st_object_class (receiver), cpu->message_selector);
    set_success (cpu, st_method_get_arg_count (method) == (cpu->message_argcount - 1));

    if (cpu->success) {
	selector_index = cpu->sp - cpu->message_argcount;

	st_oops_move (cpu->stack + selector_index,
		      cpu->stack + selector_index + 1,
		      cpu->message_argcount - 1);

	cpu->sp -= 1;
	cpu->message_argcount -= 1;
	cpu->new_method = method;
	st_cpu_execute_method ();

    } else {
	cpu->message_selector = selector;
    }
}

static void
Object_perform_withArguments (struct st_cpu *cpu)
{
    st_oop receiver;
    st_oop selector;
    st_oop method;
    st_oop array;
    st_smi array_size;

    array = ST_STACK_POP (cpu);

    set_success (cpu, st_object_format (array) == ST_FORMAT_ARRAY);

    if (ST_OBJECT_CLASS (cpu->context) == st_block_context_class)
	method = ST_METHOD_CONTEXT_METHOD (ST_BLOCK_CONTEXT_HOME (cpu->context));
    else
	method = ST_METHOD_CONTEXT_METHOD (cpu->context);

    array_size = st_smi_value (st_arrayed_object_size (array));
    set_success (cpu, (cpu->sp + array_size - 1) < (st_method_get_large_context (method) ? 32 : 12));

    if (cpu->success) {
	
	selector = cpu->message_selector;
	cpu->message_selector = ST_STACK_POP (cpu);
	receiver = ST_STACK_PEEK (cpu);
	cpu->message_argcount = array_size;

	set_success (cpu, st_object_is_symbol (cpu->message_selector));
    
	st_oops_copy (cpu->stack + cpu->sp,
		      st_array_elements (array),
		      array_size);

	cpu->sp += array_size;

	method = lookup_method (st_object_class (receiver), cpu->message_selector);
	set_success (cpu, st_method_get_arg_count (method) == array_size);
    
	if (cpu->success) {
	    cpu->new_method = method;
	    st_cpu_execute_method ();
	} else {
	    cpu->sp -= cpu->message_argcount;
	    ST_STACK_PUSH (cpu, cpu->message_selector);
	    ST_STACK_PUSH (cpu, array);
	    cpu->message_argcount = 2;
	    cpu->message_selector = selector;
	}

    } else {
	ST_STACK_UNPOP (cpu, 1);
    }
}

static void
Behavior_new (struct st_cpu *cpu)
{
    st_oop class;
    st_oop instance;
    st_smi format;

    class = ST_STACK_POP (cpu);

    switch (st_smi_value (ST_BEHAVIOR_FORMAT (class))) {
    case ST_FORMAT_OBJECT:
	instance =  st_object_allocate (class);
	break;
    case ST_FORMAT_CONTEXT:
	/* not implemented */
	abort ();
	break;
    case ST_FORMAT_FLOAT:
	instance =  st_float_allocate (class);
	break;
    case ST_FORMAT_LARGE_INTEGER:
	instance = st_large_integer_allocate (class, NULL);
	break;
    default:
	/* should not reach */
	abort ();
    }

    ST_STACK_PUSH (cpu, instance);
}

static void
Behavior_newSize (struct st_cpu *cpu)
{
    st_oop class;
    st_smi size;
    st_smi format;
    st_oop instance;

    size = pop_integer32 (cpu);
    class = ST_STACK_POP (cpu);

    switch (st_smi_value (ST_BEHAVIOR_FORMAT (class))) {
    case ST_FORMAT_ARRAY:
	instance = st_array_allocate (class, size);
	break;
    case ST_FORMAT_BYTE_ARRAY:
	instance = st_byte_array_allocate (class, size);
	break;
    case ST_FORMAT_WORD_ARRAY:
	instance = st_word_array_allocate (class, size);
	break;
    case ST_FORMAT_FLOAT_ARRAY:
	instance = st_float_array_allocate (class, size);
	break;
    case ST_FORMAT_INTEGER_ARRAY:
	/* not implemented */
	abort ();
	break;
    default:
	/* should not reach */
	abort ();
    }

    ST_STACK_PUSH (cpu, instance);
}

static void
SequenceableCollection_size (struct st_cpu *cpu)
{
    st_oop object;

    object = ST_STACK_POP (cpu);

    ST_STACK_PUSH (cpu, st_arrayed_object_size (object));
}

static void
Array_at (struct st_cpu *cpu)
{
    st_smi index    = pop_integer32 (cpu);
    st_oop receiver = ST_STACK_POP (cpu);
    
    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    ST_STACK_PUSH (cpu, st_array_at (receiver, index));
}

static void
Array_at_put (struct st_cpu *cpu)
{
    st_oop object   = ST_STACK_POP (cpu);
    st_smi index    = pop_integer32 (cpu);
    st_oop receiver = ST_STACK_POP (cpu);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 3);
	return;
    }
    
    st_array_at_put (receiver, index, object);	    
    ST_STACK_PUSH (cpu, object);
}

static void
ByteArray_at (struct st_cpu *cpu)
{
    st_smi index    = pop_integer32 (cpu);
    st_oop receiver = ST_STACK_POP (cpu);
    st_oop result;
	
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    result = st_smi_new (st_byte_array_at (receiver, index));  
    
    ST_STACK_PUSH (cpu, result);
}

static void
ByteArray_at_put (struct st_cpu *cpu)
{
    st_smi byte     = pop_integer (cpu);
    st_smi index    = pop_integer32 (cpu);
    st_oop receiver = ST_STACK_POP (cpu);

    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 3);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 3);
	return;
    }
    
    st_byte_array_at_put (receiver, index, byte);	    
    
    ST_STACK_PUSH (cpu, st_smi_new (byte));
}

static void
ByteArray_hash (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);
    st_uint  hash;

    hash = st_byte_array_hash (receiver);

    ST_STACK_PUSH (cpu, st_smi_new (hash));   
}

static void
ByteString_at (struct st_cpu *cpu)
{
    st_smi  index    = pop_integer32 (cpu);
    st_oop  receiver = ST_STACK_POP (cpu);
    st_oop  character;
    char   *charptr;

    if (ST_UNLIKELY (!cpu->success)) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    character = st_character_new (st_byte_array_at (receiver, index));

    ST_STACK_PUSH (cpu, character);
}

static void
ByteString_at_put (struct st_cpu *cpu)
{
    st_oop character = ST_STACK_POP (cpu);
    st_smi index    = pop_integer32 (cpu);
    st_oop receiver = ST_STACK_POP (cpu);
	
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 3);
	return;
    }
  
    set_success (cpu, st_object_class (character) == st_character_class);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 3);
	return;
    }

    st_byte_array_at_put (receiver, index, (st_uchar) st_character_value (character));

    ST_STACK_PUSH (cpu, character);
}


static void
ByteString_size (struct st_cpu *cpu)
{
    st_oop receiver;
    st_uint size;

    receiver = ST_STACK_POP (cpu);

    size = st_arrayed_object_size (receiver);

    /* TODO: allow size to go into a LargeInteger on overflow */
    ST_STACK_PUSH (cpu, size);
}

static void
ByteString_compare (struct st_cpu *cpu)
{
    st_oop argument = ST_STACK_POP (cpu);
    st_oop receiver = ST_STACK_POP (cpu);
    int order;

    if (st_object_format (argument) != ST_FORMAT_BYTE_ARRAY)
	set_success (cpu, false);

    if (cpu->success)
	order = strcmp ((const char *) st_byte_array_bytes (receiver),
			(const char *) st_byte_array_bytes (argument));

    if (cpu->success)
	ST_STACK_PUSH (cpu, st_smi_new (order));
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
WideString_at (struct st_cpu *cpu)
{
    st_smi index    = pop_integer32 (cpu);
    st_oop receiver = ST_STACK_POP (cpu);
    st_uchar *bytes;
    st_unichar c;
	
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    if (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    c = st_word_array_at (receiver, index);

    ST_STACK_PUSH (cpu, st_character_new (c));
}

static void
WideString_at_put (struct st_cpu *cpu)
{
    st_oop character = ST_STACK_POP (cpu);
    st_smi index    = pop_integer32 (cpu);
    st_oop receiver = ST_STACK_POP (cpu);
    st_uchar *bytes;
    st_unichar c;	

    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 3);
	return;
    }
  
    set_success (cpu, st_object_class (character) == st_character_class);

    if (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 3);
	return;
    }

    st_word_array_at_put (receiver, index, character);

    ST_STACK_PUSH (cpu, character);
}

static void
WordArray_at (struct st_cpu *cpu)
{
    st_oop receiver;
    st_smi index;
    st_uint  element;

    index = pop_integer32 (cpu);
    receiver = ST_STACK_POP (cpu);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 2);
	return;
    }
    
    element = st_word_array_at (receiver, index);

    ST_STACK_PUSH (cpu, st_smi_new (element));
}

static void
WordArray_at_put (struct st_cpu *cpu)
{
    st_smi value    = pop_integer (cpu);
    st_smi index    = pop_integer32 (cpu);
    st_oop receiver = ST_STACK_POP (cpu);
	
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 3);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 3);
	return;
    }

    st_word_array_at_put (receiver, index, value);

    ST_STACK_PUSH (cpu, st_smi_new (value));
}

static void
FloatArray_at (struct st_cpu *cpu)
{
    st_oop receiver;
    st_smi index;
    double  element;

    index = pop_integer32 (cpu);
    receiver = ST_STACK_POP (cpu);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    element = st_float_array_at (receiver, index);
    ST_STACK_PUSH (cpu, st_float_new (element));
}

static void
FloatArray_at_put (struct st_cpu *cpu)
{
    st_oop flt      = ST_STACK_POP (cpu);
    st_smi index    = pop_integer32 (cpu);
    st_oop receiver = ST_STACK_POP (cpu);

    set_success (cpu, st_object_is_heap (flt) &&
		 st_object_format (flt) == ST_FORMAT_FLOAT);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (cpu, false);
	ST_STACK_UNPOP (cpu, 3);
	return;
    }
	
    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 3);
	return;
    }

    st_float_array_at_put (receiver, index, st_float_value (flt));
    ST_STACK_PUSH (cpu, flt);
}

static inline void
activate_block_context (struct st_cpu *cpu)
{
    st_oop  block;
    st_smi  argcount;
    st_oop home;

    block = cpu->message_receiver;
    argcount = st_smi_value (ST_BLOCK_CONTEXT_ARGCOUNT (block));
    if (ST_UNLIKELY (argcount != cpu->message_argcount)) {
	cpu->success = false;
	return;
    }

    st_oops_copy (ST_BLOCK_CONTEXT_STACK (block),
		  cpu->stack + cpu->sp - argcount,
		  argcount);
    cpu->sp -= cpu->message_argcount + 1;
    
    ST_CONTEXT_PART_IP (block) = ST_BLOCK_CONTEXT_INITIALIP (block);
    ST_CONTEXT_PART_SP (block) = st_smi_new (argcount);
    ST_BLOCK_CONTEXT_CALLER (block) = cpu->context;

    ST_CONTEXT_PART_IP (cpu->context) = st_smi_new (cpu->ip);
    ST_CONTEXT_PART_SP (cpu->context) = st_smi_new (cpu->sp);

    home = ST_BLOCK_CONTEXT_HOME (block);
    cpu->context  = block;
    cpu->method   = ST_METHOD_CONTEXT_METHOD (home);
    cpu->receiver = ST_METHOD_CONTEXT_RECEIVER (home);
    cpu->literals = st_array_elements (ST_METHOD_LITERALS (cpu->method));
    cpu->temps    = ST_METHOD_CONTEXT_STACK (home);
    cpu->stack    = ST_BLOCK_CONTEXT_STACK (block);
    cpu->sp       = st_smi_value (ST_CONTEXT_PART_SP (block));
    cpu->ip       = st_smi_value (ST_CONTEXT_PART_IP (block));
    cpu->bytecode = st_method_bytecode_bytes (cpu->method);

}

static void
BlockContext_value (struct st_cpu *cpu)
{
    activate_block_context (cpu);
}

static void
BlockContext_valueWithArguments (struct st_cpu *cpu)
{
    st_oop block;
    st_oop values;
    st_smi argcount;

    block  = cpu->message_receiver;
    values = ST_STACK_PEEK (cpu);

    if (st_object_class (values) != st_array_class) {
	set_success (cpu, false);
	return;
    }

    argcount = st_smi_value (ST_BLOCK_CONTEXT_ARGCOUNT (block));
    if (argcount != st_smi_value (st_arrayed_object_size (values))) {
	set_success (cpu, false);
	return;
    }
    
    st_oops_copy (ST_BLOCK_CONTEXT_STACK (block),
		  ST_ARRAY (values)->elements,
		  argcount);
    
    cpu->sp -= cpu->message_argcount + 1;

    ST_CONTEXT_PART_IP (block) = ST_BLOCK_CONTEXT_INITIALIP (block);
    ST_CONTEXT_PART_SP (block) = st_smi_new (argcount);
    ST_BLOCK_CONTEXT_CALLER (block) = cpu->context;

    st_cpu_set_active_context (block);
}

static void
UndefinedObject_exitWithResult (struct st_cpu *cpu)
{
    longjmp (cpu->main_loop, 0);
}

static void
Character_value (struct st_cpu *cpu)
{
    st_oop receiver = ST_STACK_POP (cpu);

    ST_STACK_PUSH (cpu, st_smi_new (st_character_value (receiver)));
}

static void
Character_characterFor (struct st_cpu *cpu)
{
    st_oop receiver;
    st_smi value;

    value = pop_integer (cpu);
    receiver = ST_STACK_POP (cpu);

    if (cpu->success)
	ST_STACK_PUSH (cpu, st_character_new (value));
    else
	ST_STACK_UNPOP (cpu, 2);
}

static void
FileStream_open (struct st_cpu *cpu)
{
    int fd;
    int mode;
    st_oop name;
    st_oop mode_oop;
    char *str;

    mode_oop = ST_STACK_POP (cpu);
    name =  ST_STACK_POP (cpu);

    if (st_object_class (mode_oop) != st_symbol_class) {
	ST_PRIMITIVE_FAIL (cpu);
	return;
    }

    if (st_object_class (name) != st_string_class) {
	ST_PRIMITIVE_FAIL (cpu);
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
	ST_PRIMITIVE_FAIL (cpu);
	return;
    }

    str = st_byte_array_bytes (name);
    fd = open (str, O_CREAT | mode, 0644);
    ST_STACK_PUSH (cpu, st_smi_new (fd));
}

static void
FileStream_close (struct st_cpu *cpu)
{
    int fd;
    int byte;

    fd = pop_integer (cpu);
    (void) ST_STACK_POP (cpu);

    if (!cpu->success) {
	ST_STACK_UNPOP (cpu, 2);
	return;
    }

    if (close (fd) != 0) {
	ST_STACK_UNPOP (cpu, 2);
	ST_PRIMITIVE_FAIL (cpu);
    }

    ST_STACK_PUSH (cpu, cpu->message_receiver);
}

static void
FileStream_write (struct st_cpu *cpu)
{
    int fd;
    int byte;

    byte = pop_integer (cpu);
    fd = pop_integer (cpu);
    (void) ST_STACK_POP (cpu);

    if (!cpu->success) {
	ST_PRIMITIVE_FAIL (cpu);
	ST_STACK_UNPOP (cpu, 3);	
    }

    if (write (fd, &byte, 1) < 0) {
	ST_PRIMITIVE_FAIL (cpu);
	ST_STACK_UNPOP (cpu, 3);	
    }

    ST_STACK_PUSH (cpu, cpu->message_receiver);
}

static void
FileStream_read (struct st_cpu *cpu)
{

}

static void
FileStream_writeN (struct st_cpu *cpu)
{
    st_oop array;
    int fd;
    st_uint size;
    char *buf;

    array = ST_STACK_POP (cpu);
    fd = pop_integer (cpu);

    if (st_object_format (array) != ST_FORMAT_BYTE_ARRAY) {
	ST_PRIMITIVE_FAIL (cpu);
	return;
    }

    if (!cpu->success)
	return;

    size = st_smi_value (st_arrayed_object_size (array));
    buf = st_byte_array_bytes (array);

    write (fd, buf, size);

    ST_STACK_PUSH (cpu, st_true);
}

static void
FileStream_readN (struct st_cpu *cpu)
{

}

static void
FileStream_position (struct st_cpu *cpu)
{

}

static void
FileStream_setPosition (struct st_cpu *cpu)
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

