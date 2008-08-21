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
#include "st-machine.h"
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
#include "st-compiler.h"
#include "st-handle.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


#define ST_PRIMITIVE_FAIL(machine)			\
    machine->success = false


static inline void
set_success (st_machine *machine, bool success)
{
    machine->success = machine->success && success;
}

static inline int
pop_integer (st_machine *machine)
{
    st_oop object = ST_STACK_POP (machine);
  
    if (ST_LIKELY (st_object_is_smi (object)))
	return st_smi_value (object);	

    ST_PRIMITIVE_FAIL (machine);
    return 0;
}

static inline int
pop_integer32 (st_machine *machine)
{
    st_oop object = ST_STACK_POP (machine);
 
    if (ST_LIKELY (st_object_is_smi (object)))
	return st_smi_value (object);
    else if (st_object_class (object) == ST_LARGE_INTEGER_CLASS)
	return (int) mp_get_int (st_large_integer_value (object));

    ST_PRIMITIVE_FAIL (machine);
    return 0;
}

static void
SmallInteger_add (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    int result;

    if (ST_LIKELY (machine->success)) {
	result = x + y; 
	if (((result << 1) ^ (result << 2)) >= 0) {
	    ST_STACK_PUSH (machine, st_smi_new (result));
	    return;
	} else {
	    ST_PRIMITIVE_FAIL (machine);
	}
    }
    
    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_sub (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    int result;


    if (ST_LIKELY (machine->success)) {
	result = x + y; 
	if (((result << 1) ^ (result << 2)) >= 0) {
	    ST_STACK_PUSH (machine, st_smi_new (result));
	    return;
	} else {
	    ST_PRIMITIVE_FAIL (machine);
	}
    }

    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_lt (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;

    if (ST_LIKELY (machine->success)) {
	result = (x < y) ? ST_TRUE : ST_FALSE;
	ST_STACK_PUSH (machine, result);
	return;
    }

    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_gt (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;

    if (ST_LIKELY (machine->success)) {
	result = (x > y) ? ST_TRUE : ST_FALSE;
	ST_STACK_PUSH (machine, result);
	return;
    }

    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_le (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;

    if (ST_LIKELY (machine->success)) {
	result = (x <= y) ? ST_TRUE : ST_FALSE;
	ST_STACK_PUSH (machine, result);
	return;
    }

    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_ge (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;

    if (ST_LIKELY (machine->success)) {
	result = (x >= y) ? ST_TRUE : ST_FALSE;
	ST_STACK_PUSH (machine, result);
	return;
    }
   
   ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_eq (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;

    if (ST_LIKELY (machine->success)) {
	result = (x == y) ? ST_TRUE : ST_FALSE;
	ST_STACK_PUSH (machine, result);
	return;
    }

    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_ne (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;

    if (ST_LIKELY (machine->success)) {
	result = (x != y) ? ST_TRUE : ST_FALSE;
	ST_STACK_PUSH (machine, result);
	return;
    }

    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_mul (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    int64_t result;

    if (machine->success) {
	result = x * y;
	if (result >= ST_SMALL_INTEGER_MIN && result <= ST_SMALL_INTEGER_MAX) {
	    ST_STACK_PUSH (machine, st_smi_new ((int) result));
	    return;
	} else {
	    ST_PRIMITIVE_FAIL (machine);
	}
    }

    ST_STACK_UNPOP (machine, 2);
}

/* selector: / */
static void
SmallInteger_div (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;
    
    if (ST_LIKELY (machine->success)) {

	if (y != 0 && x % y == 0) {
	    result = st_smi_new (x / y);
	    ST_STACK_PUSH (machine, result);
	    return;
	} else {
	    ST_PRIMITIVE_FAIL (machine);
	}
    }
    
    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_intDiv (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;

    if (ST_LIKELY (machine->success)) {

	if (y != 0) {
	    result = st_smi_new (x / y);
	    ST_STACK_PUSH (machine, result);
	    return;
	} else {
	    ST_PRIMITIVE_FAIL (machine);
	}
    }
    
    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_mod (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;
    
    if (ST_LIKELY (machine->success)) {
	result = st_smi_new (x % y);
	ST_STACK_PUSH (machine, result);
	return;	
    }
    
    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_bitOr (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result = ST_NIL;

    if (ST_LIKELY (machine->success)) {
	result = st_smi_new (x | y);
	ST_STACK_PUSH (machine, result);
	return;	
    }
    
    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_bitXor (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result;

    if (ST_LIKELY (machine->success)) {
	result = st_smi_new (x ^ y);
	ST_STACK_PUSH (machine, result);
	return;	
    }
    
    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_bitAnd (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result = ST_NIL;

    if (ST_LIKELY (machine->success)) {
	result = st_smi_new (x & y);
	ST_STACK_PUSH (machine, result);
	return;	
    }
    
    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_bitShift (st_machine *machine)
{
    int y = pop_integer (machine);
    int x = pop_integer (machine);
    st_oop result = ST_NIL;

    if (ST_LIKELY (machine->success)) {
	if (y > 0)
	    result = st_smi_new (x << y);
	else if (y < 0)
	    result = st_smi_new (x >> (-y));
	else
	    result = st_smi_new (x);

	ST_STACK_PUSH (machine, result);
	return;
    }

    ST_STACK_UNPOP (machine, 2);
}

static void
SmallInteger_asFloat (st_machine *machine)
{
    int x = pop_integer (machine);
    st_oop result = ST_NIL;

    if (ST_LIKELY (machine->success)) {
	result = st_float_new ((double) x);
	ST_STACK_PUSH (machine, result);
	return;	
    }
    
    ST_STACK_UNPOP (machine, 1);
}

static void
SmallInteger_asLargeInteger (st_machine *machine)
{
    int receiver = pop_integer (machine);
    mp_int value;
    st_oop result;

    mp_init_set (&value, abs (receiver));

    if (receiver < 0)
	mp_neg (&value, &value);
    
    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
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
pop_large_integer (st_machine *machine)
{
    st_oop object = ST_STACK_POP (machine);

    set_success (machine, st_object_class (object) == ST_LARGE_INTEGER_CLASS);
    
    return object;
}

static void
LargeInteger_add (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine); 
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    BINARY_OP (mp_add, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_sub (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    BINARY_OP (mp_sub, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_mul (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    BINARY_OP (mp_mul, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_div (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    mp_int quotient, remainder;
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
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
	ST_STACK_PUSH (machine, result);
	mp_clear (&remainder);
    } else {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 2);
	mp_clear_multi (&quotient, &remainder, NULL);
    }
}

static void
LargeInteger_intDiv (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    BINARY_DIV_OP (mp_div, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_mod (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    BINARY_OP (mp_mod, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_gcd (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    BINARY_OP (mp_gcd, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_lcm (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    BINARY_OP (mp_lcm, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_eq (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    int    relation;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_EQ) ? ST_TRUE : ST_FALSE;
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_ne (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    int    relation;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_EQ) ? ST_FALSE : ST_TRUE;
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_lt (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    int    relation;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    relation = mp_cmp (VALUE (a), VALUE (b));    
    result = (relation == MP_LT) ? ST_TRUE : ST_FALSE;
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_gt (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);

    st_oop result;
    int    relation;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_GT) ? ST_TRUE : ST_FALSE;
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_le (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    int    relation;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_LT || relation == MP_EQ) ? ST_TRUE : ST_FALSE;
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_ge (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    int    relation;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    relation = mp_cmp (VALUE (a), VALUE (b));
    result = (relation == MP_GT || relation == MP_EQ) ? ST_TRUE : ST_FALSE;
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_squared (st_machine *machine)
{
    st_oop receiver = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 1);
	return;
    }
    
    UNARY_OP (mp_sqr, receiver);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_bitOr (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    BINARY_OP (mp_or, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_bitAnd (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    BINARY_OP (mp_and, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_bitXor (st_machine *machine)
{
    st_oop b = pop_large_integer (machine);
    st_oop a = pop_large_integer (machine);
    st_oop result;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    BINARY_OP (mp_xor, a, b);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

static void
LargeInteger_bitShift (st_machine *machine)
{
    int displacement = pop_integer32 (machine);
    st_oop receiver     = pop_large_integer (machine);
    st_oop result;
    mp_int value;
    
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    mp_init (&value);

    if (displacement >= 0)
	mp_mul_2d (VALUE (receiver), displacement, &value);
    else
	mp_div_2d (VALUE (receiver), abs (displacement), &value, NULL);

    result = st_large_integer_new (&value);
    ST_STACK_PUSH (machine, result);
}

#define ST_DIGIT_RADIX (1L << DIGIT_BIT)


static void
LargeInteger_asFloat (st_machine *machine)
{
    st_oop  receiver = pop_large_integer (machine);
    char   *string;
    double  result;
    mp_int *m;
    int i;

    m = st_large_integer_value (receiver);
    if (m->used == 0) {
	ST_STACK_PUSH (machine, st_float_new (0));
	return;
    }

    i = m->used;
    result = DIGIT (m, i);    
    while (--i >= 0)
	result = (result * ST_DIGIT_RADIX) + DIGIT (m, i);

    if (m->sign == MP_NEG)
	result = -result;

    ST_STACK_PUSH (machine, st_float_new (result));
}

static void
LargeInteger_printStringBase (st_machine *machine)
{
    int     radix = pop_integer (machine);
    st_oop  x     = pop_large_integer (machine);
    char   *string;
    st_oop  result;

    if (radix < 2 || radix > 36)
	set_success (machine, false);

    if (machine->success) {
	string = st_large_integer_to_string (x, radix);
	result = st_string_new (string);
    }

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
LargeInteger_hash (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    mp_int *value;
    int result;
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

    ST_STACK_PUSH (machine, st_smi_new (result));
}


static inline st_oop
pop_float (st_machine *machine)
{
    st_oop object = ST_STACK_POP (machine);

    set_success (machine, st_object_class (object) == ST_FLOAT_CLASS);
    
    return object;
}

static void
Float_add (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    if (machine->success)
	result = st_float_new (st_float_value (x) + st_float_value (y));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_sub (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    if (machine->success)
	result = st_float_new (st_float_value (x) - st_float_value (y));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_lt (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    if (machine->success)
	result = isless (st_float_value (x), st_float_value (y)) ? ST_TRUE : ST_FALSE;

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_gt (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    if (machine->success)
	result = isgreater (st_float_value (x), st_float_value (y)) ? ST_TRUE : ST_FALSE;

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_le (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    if (machine->success)
	result = islessequal (st_float_value (x), st_float_value (y)) ? ST_TRUE : ST_FALSE;

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_ge (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    if (machine->success)
	result = isgreaterequal (st_float_value (x), st_float_value (y)) ? ST_TRUE : ST_FALSE;

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_eq (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    if (machine->success)
	result = (st_float_value (x) == st_float_value (y)) ? ST_TRUE : ST_FALSE;

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_ne (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    if (machine->success)
	result = (st_float_value (x) != st_float_value (y)) ? ST_TRUE : ST_FALSE;

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_mul (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    if (machine->success)
	result = st_float_new (st_float_value (x) * st_float_value (y));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_div (st_machine *machine)
{
    st_oop y = pop_float (machine);
    st_oop x = pop_float (machine);
    st_oop result = ST_NIL;

    set_success (machine, y != 0);

    if (machine->success)
	result = st_float_new (st_float_value (x) / st_float_value (y));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
Float_sin (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (sin (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_cos (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (cos (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_tan (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (tan (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_arcSin (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (asin (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_arcCos (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (acos (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_arcTan (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (atan (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_sqrt (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (sqrt (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_log (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (log10 (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_ln (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (log (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_exp (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
    double value;

    value = st_float_value (receiver);

    result = st_float_new (exp (value));

    if (machine->success)
	ST_STACK_PUSH (machine, result);
    else
	ST_STACK_UNPOP (machine, 1);
}

static void
Float_truncated (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    int result;

    result = (int) trunc (st_float_value (receiver));

    ST_STACK_PUSH (machine, st_smi_new (result));
}

static void
Float_fractionPart (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    double frac_part, int_part;
    st_oop result;

    frac_part = modf (st_float_value (receiver), &int_part);

    result = st_float_new (frac_part);

    ST_STACK_PUSH (machine, result);
}

static void
Float_integerPart (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    double int_part;
    st_oop result;

    modf (st_float_value (receiver), &int_part);

    result = st_smi_new ((int) int_part);
    ST_STACK_PUSH (machine, result);
}

static void
Float_hash (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    unsigned int hash = 0;
    int result;
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

    ST_STACK_PUSH (machine, st_smi_new (result));
}

static void
Float_printStringBase (st_machine *machine)
{
    int base     = pop_integer(machine);
    st_oop receiver = ST_STACK_POP (machine);
    char  *tmp;
    st_oop string;

    if (!machine->success ||
	!st_object_is_heap (receiver) ||
	st_object_format (receiver) != ST_FORMAT_FLOAT) {
	machine->success = false;
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    /* ignore base for the time being */
    tmp = st_strdup_printf ("%g", st_float_value (receiver));
    string = st_string_new (tmp);
    st_free (tmp);

    ST_STACK_PUSH (machine, string);
}

static void
Object_error (st_machine *machine)
{
    st_oop message;
    st_oop traceback;
    st_oop receiver;

    traceback = ST_STACK_POP (machine);
    message = ST_STACK_POP (machine);
    receiver = ST_STACK_POP (machine);
    
    if (!st_object_is_heap (traceback) ||
	st_object_format (traceback) != ST_FORMAT_BYTE_ARRAY) {
	/* can't resume execution in this prim */
	abort();
    }

    if (!st_object_is_heap (message) ||
	st_object_format (message) != ST_FORMAT_BYTE_ARRAY) {
	/* can't resume execution in this prim */
	abort();
    }

    printf ("An error occurred during program execution\n");
    printf ("message: %s\n\n", st_byte_array_bytes (message));

    printf ("Traceback:\n");
    puts (st_byte_array_bytes (traceback));

    /* set success to false to signal error */
    machine->success = false;
    longjmp (machine->main_loop, 0);
}

static void
Object_class (st_machine *machine)
{
    st_oop object;

    object = ST_STACK_POP (machine);

    ST_STACK_PUSH (machine, st_object_class (object));
}

static void
Object_identityHash (st_machine *machine)
{
    st_oop  object;
    st_uint hash;
    
    object = ST_STACK_POP (machine);
    
    if (st_object_is_smi (object))
	hash = st_smi_hash (object);
    else if (st_object_is_character (object))
	hash = st_character_hash (object);
    else {
	st_object_set_hashed (object, true);
	hash = st_identity_hashtable_hash (memory->ht, object);
    }
    ST_STACK_PUSH (machine, st_smi_new (hash));
}

static void
Object_copy (st_machine *machine)
{
    st_oop receiver;
    st_oop copy;
    st_oop class;
    int size;

    (void) ST_STACK_POP (machine);

    if (!st_object_is_heap (machine->message_receiver)) {
	ST_STACK_PUSH (machine, machine->message_receiver);
	return;
    }

    switch (st_object_format (machine->message_receiver)) {

    case ST_FORMAT_OBJECT:
    {
	class = ST_OBJECT_CLASS (machine->message_receiver);
	size = st_smi_value (ST_BEHAVIOR_INSTANCE_SIZE (class));
	copy = st_object_new (class);
	st_oops_copy (ST_OBJECT_FIELDS (copy),
		      ST_OBJECT_FIELDS (machine->message_receiver),
		      size);
	break;

    }
    case ST_FORMAT_ARRAY:
    {
	size = st_smi_value (ST_ARRAYED_OBJECT (machine->message_receiver)->size);
	copy = st_object_new_arrayed (ST_OBJECT_CLASS (machine->message_receiver), size);
	st_oops_copy (ST_ARRAY (copy)->elements,
		      ST_ARRAY (machine->message_receiver)->elements,
		      size);
	break;
    }
    case ST_FORMAT_BYTE_ARRAY:
    {
	size = st_smi_value (ST_ARRAYED_OBJECT (machine->message_receiver)->size);
	copy = st_object_new_arrayed (ST_OBJECT_CLASS (machine->message_receiver), size);
	memcpy (st_byte_array_bytes (copy),
		st_byte_array_bytes (machine->message_receiver),
		size);
	break;
    }
    case ST_FORMAT_FLOAT_ARRAY:
    {
	size = st_smi_value (st_arrayed_object_size (machine->message_receiver));
	copy = st_object_new_arrayed (ST_OBJECT_CLASS (machine->message_receiver), size);
	memcpy (st_float_array_elements (copy),
		st_float_array_elements (machine->message_receiver),
		sizeof (double) * size);

	break;
    }
    case ST_FORMAT_WORD_ARRAY:
    {
	size = st_smi_value (st_arrayed_object_size (machine->message_receiver));
	copy = st_object_new_arrayed (ST_OBJECT_CLASS (machine->message_receiver), size);
	memcpy (st_word_array_elements (copy),
		st_word_array_elements (machine->message_receiver),
		sizeof (st_uint) * size);
	break;
    }
    case ST_FORMAT_FLOAT:
    {
	copy = st_object_new (ST_FLOAT_CLASS);
	st_float_set_value (copy, st_float_value (machine->message_receiver));
	break;
    }
    case ST_FORMAT_LARGE_INTEGER:
    {
	mp_int value;
	int    result;

	copy = st_object_new (ST_LARGE_INTEGER_CLASS);
	
	result = mp_init_copy (st_large_integer_value (copy),
			       st_large_integer_value (machine->message_receiver));
	if (result != MP_OKAY)
	    abort ();
	break;
    }
    case ST_FORMAT_HANDLE:
	
	copy = st_object_new (ST_HANDLE_CLASS);
	ST_HANDLE_VALUE (copy) = ST_HANDLE_VALUE (machine->message_receiver);
	break;
    case ST_FORMAT_CONTEXT:
    case ST_FORMAT_INTEGER_ARRAY:
    default:
	/* not implemented yet */
	abort ();
    }

    ST_STACK_PUSH (machine, copy);
}

static void
Object_equivalent (st_machine *machine)
{
    st_oop y = ST_STACK_POP (machine);
    st_oop x = ST_STACK_POP (machine);
    
    ST_STACK_PUSH (machine, ((x == y) ? ST_TRUE : ST_FALSE));
}

static st_oop
lookup_method (st_oop class, st_oop selector)
{
    st_oop method;
    st_oop parent = class;
    st_uint index;

    while (parent != ST_NIL) {
	method = st_dictionary_at (ST_BEHAVIOR_METHOD_DICTIONARY (parent), selector);
	if (method != ST_NIL)
	    return method;
	parent = ST_BEHAVIOR_SUPERCLASS (parent);
    }

    return 0;
}

static void
Object_perform (st_machine *machine)
{
    st_oop receiver;
    st_oop selector;
    st_oop method;
    st_uint selector_index;

    selector = machine->message_selector;
    machine->message_selector = machine->stack[machine->sp - machine->message_argcount];
    receiver = machine->message_receiver;

    set_success (machine, st_object_is_symbol (machine->message_selector));
    method = lookup_method (st_object_class (receiver), machine->message_selector);
    set_success (machine, st_method_get_arg_count (method) == (machine->message_argcount - 1));

    if (machine->success) {
	selector_index = machine->sp - machine->message_argcount;

	st_oops_move (machine->stack + selector_index,
		      machine->stack + selector_index + 1,
		      machine->message_argcount - 1);

	machine->sp -= 1;
	machine->message_argcount -= 1;
	machine->new_method = method;
	st_machine_execute_method (machine);

    } else {
	machine->message_selector = selector;
    }
}

static void
Object_perform_withArguments (st_machine *machine)
{
    st_oop receiver;
    st_oop selector;
    st_oop method;
    st_oop array;
    int array_size;

    array = ST_STACK_POP (machine);

    set_success (machine, st_object_format (array) == ST_FORMAT_ARRAY);

    if (ST_OBJECT_CLASS (machine->context) == ST_BLOCK_CONTEXT_CLASS)
	method = ST_METHOD_CONTEXT_METHOD (ST_BLOCK_CONTEXT_HOME (machine->context));
    else
	method = ST_METHOD_CONTEXT_METHOD (machine->context);

    array_size = st_smi_value (st_arrayed_object_size (array));
    set_success (machine, (machine->sp + array_size - 1) < (st_method_get_large_context (method) ? 32 : 12));

    if (machine->success) {
	
	selector = machine->message_selector;
	machine->message_selector = ST_STACK_POP (machine);
	receiver = ST_STACK_PEEK (machine);
	machine->message_argcount = array_size;

	set_success (machine, st_object_is_symbol (machine->message_selector));
    
	st_oops_copy (machine->stack + machine->sp,
		      st_array_elements (array),
		      array_size);

	machine->sp += array_size;

	method = lookup_method (st_object_class (receiver), machine->message_selector);
	set_success (machine, st_method_get_arg_count (method) == array_size);
    
	if (machine->success) {
	    machine->new_method = method;
	    st_machine_execute_method (machine);
	} else {
	    machine->sp -= machine->message_argcount;
	    ST_STACK_PUSH (machine, machine->message_selector);
	    ST_STACK_PUSH (machine, array);
	    machine->message_argcount = 2;
	    machine->message_selector = selector;
	}

    } else {
	ST_STACK_UNPOP (machine, 1);
    }
}

static void
Behavior_new (st_machine *machine)
{
    st_oop class;
    st_oop instance;
    int format;

    class = ST_STACK_POP (machine);

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
    case ST_FORMAT_HANDLE:
	instance = st_handle_allocate (class);
	break;
    default:
	/* should not reach */
	abort ();
    }

    ST_STACK_PUSH (machine, instance);
}

static void
Behavior_newSize (st_machine *machine)
{
    st_oop class;
    int size;
    int format;
    st_oop instance;

    size = pop_integer32 (machine);
    class = ST_STACK_POP (machine);

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

    ST_STACK_PUSH (machine, instance);
}

static void
Behavior_compile (st_machine *machine)
{
    st_compiler_error error;
    st_oop receiver;
    st_oop string;
    
    string = ST_STACK_POP (machine);
    receiver = ST_STACK_POP (machine);
    if (!st_object_is_heap (string) ||
	st_object_format (string) != ST_FORMAT_BYTE_ARRAY) {
	machine->success = false;
	ST_STACK_UNPOP (machine, 2);
	return;
    }
   
    if (!st_compile_string (receiver,
			   (char *) st_byte_array_bytes (string),
			   &error)) {
	machine->success = false;
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    ST_STACK_PUSH (machine, receiver);
}

static void
SequenceableCollection_size (st_machine *machine)
{
    st_oop object;

    object = ST_STACK_POP (machine);

    ST_STACK_PUSH (machine, st_arrayed_object_size (object));
}

static void
Array_at (st_machine *machine)
{
    int index    = pop_integer32 (machine);
    st_oop receiver = ST_STACK_POP (machine);
    
    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    ST_STACK_PUSH (machine, st_array_at (receiver, index));
}

static void
Array_at_put (st_machine *machine)
{
    st_oop object   = ST_STACK_POP (machine);
    int index    = pop_integer32 (machine);
    st_oop receiver = ST_STACK_POP (machine);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 3);
	return;
    }
    
    st_array_at_put (receiver, index, object);	    
    ST_STACK_PUSH (machine, object);
}

static void
ByteArray_at (st_machine *machine)
{
    int    index    = pop_integer32 (machine);
    st_oop receiver = ST_STACK_POP (machine);
    st_oop result;
	
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    result = st_smi_new (st_byte_array_at (receiver, index));  
    
    ST_STACK_PUSH (machine, result);
}

static void
ByteArray_at_put (st_machine *machine)
{
    int byte     = pop_integer (machine);
    int index    = pop_integer32 (machine);
    st_oop receiver = ST_STACK_POP (machine);

    if (!machine->success) {
	ST_STACK_UNPOP (machine, 3);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 3);
	return;
    }
    
    st_byte_array_at_put (receiver, index, byte);	    
    
    ST_STACK_PUSH (machine, st_smi_new (byte));
}

static void
ByteArray_hash (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);
    st_uint  hash;

    hash = st_byte_array_hash (receiver);

    ST_STACK_PUSH (machine, st_smi_new (hash));   
}

static void
ByteString_at (st_machine *machine)
{
    int  index    = pop_integer32 (machine);
    st_oop  receiver = ST_STACK_POP (machine);
    st_oop  character;
    char   *charptr;

    if (ST_UNLIKELY (!machine->success)) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    character = st_character_new (st_byte_array_at (receiver, index));

    ST_STACK_PUSH (machine, character);
}

static void
ByteString_at_put (st_machine *machine)
{
    st_oop character = ST_STACK_POP (machine);
    int index    = pop_integer32 (machine);
    st_oop receiver = ST_STACK_POP (machine);
	
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 3);
	return;
    }
  
    set_success (machine, st_object_class (character) == ST_CHARACTER_CLASS);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 3);
	return;
    }

    st_byte_array_at_put (receiver, index, (st_uchar) st_character_value (character));

    ST_STACK_PUSH (machine, character);
}


static void
ByteString_size (st_machine *machine)
{
    st_oop receiver;
    st_uint size;

    receiver = ST_STACK_POP (machine);

    size = st_arrayed_object_size (receiver);

    /* TODO: allow size to go into a LargeInteger on overflow */
    ST_STACK_PUSH (machine, size);
}

static void
ByteString_compare (st_machine *machine)
{
    st_oop argument = ST_STACK_POP (machine);
    st_oop receiver = ST_STACK_POP (machine);
    int order;

    if (st_object_format (argument) != ST_FORMAT_BYTE_ARRAY)
	set_success (machine, false);

    if (machine->success)
	order = strcmp ((const char *) st_byte_array_bytes (receiver),
			(const char *) st_byte_array_bytes (argument));

    if (machine->success)
	ST_STACK_PUSH (machine, st_smi_new (order));
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
WideString_at (st_machine *machine)
{
    int index    = pop_integer32 (machine);
    st_oop receiver = ST_STACK_POP (machine);
    st_uchar *bytes;
    st_unichar c;
	
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    if (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    c = st_word_array_at (receiver, index);

    ST_STACK_PUSH (machine, st_character_new (c));
}

static void
WideString_at_put (st_machine *machine)
{
    st_oop character = ST_STACK_POP (machine);
    int index    = pop_integer32 (machine);
    st_oop receiver = ST_STACK_POP (machine);
    st_uchar *bytes;
    st_unichar c;	

    if (!machine->success) {
	ST_STACK_UNPOP (machine, 3);
	return;
    }
  
    set_success (machine, st_object_class (character) == ST_CHARACTER_CLASS);

    if (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 3);
	return;
    }

    st_word_array_at_put (receiver, index, character);

    ST_STACK_PUSH (machine, character);
}

static void
WordArray_at (st_machine *machine)
{
    st_oop receiver;
    int index;
    st_uint  element;

    index = pop_integer32 (machine);
    receiver = ST_STACK_POP (machine);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 2);
	return;
    }
    
    element = st_word_array_at (receiver, index);

    ST_STACK_PUSH (machine, st_smi_new (element));
}

static void
WordArray_at_put (st_machine *machine)
{
    int value    = pop_integer (machine);
    int index    = pop_integer32 (machine);
    st_oop receiver = ST_STACK_POP (machine);
	
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 3);
	return;
    }

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 3);
	return;
    }

    st_word_array_at_put (receiver, index, value);

    ST_STACK_PUSH (machine, st_smi_new (value));
}

static void
FloatArray_at (st_machine *machine)
{
    st_oop receiver;
    int index;
    double  element;

    index = pop_integer32 (machine);
    receiver = ST_STACK_POP (machine);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    element = st_float_array_at (receiver, index);
    ST_STACK_PUSH (machine, st_float_new (element));
}

static void
FloatArray_at_put (st_machine *machine)
{
    st_oop flt      = ST_STACK_POP (machine);
    int index    = pop_integer32 (machine);
    st_oop receiver = ST_STACK_POP (machine);

    set_success (machine, st_object_is_heap (flt) &&
		 st_object_format (flt) == ST_FORMAT_FLOAT);

    if (ST_UNLIKELY (index < 1 || index > st_smi_value (st_arrayed_object_size (receiver)))) {
	set_success (machine, false);
	ST_STACK_UNPOP (machine, 3);
	return;
    }
	
    if (!machine->success) {
	ST_STACK_UNPOP (machine, 3);
	return;
    }

    st_float_array_at_put (receiver, index, st_float_value (flt));
    ST_STACK_PUSH (machine, flt);
}

static void
BlockContext_value (st_machine *machine)
{
    st_oop  block;
    st_uint  argcount;
    st_oop home;

    block = machine->message_receiver;
    argcount = st_smi_value (ST_BLOCK_CONTEXT_ARGCOUNT (block));
    if (ST_UNLIKELY (argcount != machine->message_argcount)) {
	machine->success = false;
	return;
    }

    st_oops_copy (ST_BLOCK_CONTEXT_STACK (block),
		  machine->stack + machine->sp - argcount,
		  argcount);
    machine->sp -= machine->message_argcount + 1;
    
    ST_CONTEXT_PART_IP (block) = ST_BLOCK_CONTEXT_INITIALIP (block);
    ST_CONTEXT_PART_SP (block) = st_smi_new (argcount);
    ST_CONTEXT_PART_SENDER (block) = machine->context;

    st_machine_set_active_context (machine, block);
}

static void
BlockContext_valueWithArguments (st_machine *machine)
{
    st_oop block;
    st_oop values;
    int argcount;

    block  = machine->message_receiver;
    values = ST_STACK_PEEK (machine);

    if (st_object_class (values) != ST_ARRAY_CLASS) {
	set_success (machine, false);
	return;
    }

    argcount = st_smi_value (ST_BLOCK_CONTEXT_ARGCOUNT (block));
    if (argcount != st_smi_value (st_arrayed_object_size (values))) {
	set_success (machine, false);
	return;
    }
    
    st_oops_copy (ST_BLOCK_CONTEXT_STACK (block),
		  ST_ARRAY (values)->elements,
		  argcount);
    
    machine->sp -= machine->message_argcount + 1;

    ST_CONTEXT_PART_IP (block) = ST_BLOCK_CONTEXT_INITIALIP (block);
    ST_CONTEXT_PART_SP (block) = st_smi_new (argcount);
    ST_CONTEXT_PART_SENDER (block) = machine->context;

    st_machine_set_active_context (machine, block);
}

static void
System_exitWithResult (st_machine *machine)
{
    /* set success to true to signal that everything was alright */
    machine->success = true;
    longjmp (machine->main_loop, 0);
}

static void
Character_value (st_machine *machine)
{
    st_oop receiver = ST_STACK_POP (machine);

    ST_STACK_PUSH (machine, st_smi_new (st_character_value (receiver)));
}

static void
Character_characterFor (st_machine *machine)
{
    st_oop receiver;
    int value;

    value = pop_integer (machine);
    receiver = ST_STACK_POP (machine);

    if (machine->success)
	ST_STACK_PUSH (machine, st_character_new (value));
    else
	ST_STACK_UNPOP (machine, 2);
}

static void
FileStream_open (st_machine *machine)
{
    st_oop filename;
    st_oop handle;
    char  *str;
    int flags, mode;
    int fd;

    mode     = pop_integer32 (machine);
    filename = ST_STACK_POP (machine);
    if (st_object_format (filename) != ST_FORMAT_BYTE_ARRAY) {
	machine->success = false;
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    if (mode == 0)
	flags = O_RDONLY;
    else if (mode == 1)
	flags = O_WRONLY;
    else {
	machine->success = false;
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    str = st_byte_array_bytes (filename);

    fd = open (str, O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
	fprintf (stderr, strerror (errno));
	machine->success = false;
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    ftruncate (fd, 0);

    /* pop receiver */
    (void) ST_STACK_POP (machine);

    handle = st_object_new (ST_HANDLE_CLASS);
    ST_HANDLE_VALUE (handle) = fd;

    ST_STACK_PUSH (machine, handle);
}

static void
FileStream_close (st_machine *machine)
{
    st_oop handle;
    int    fd;

    handle = ST_STACK_POP (machine);
    fd = ST_HANDLE_VALUE (handle);

    if (close (fd) < 0) {
	machine->success = false;
	ST_STACK_UNPOP (machine, 1);
	return;
    }

    /* leave receiver on stack */

}

static void
FileStream_write (st_machine *machine)
{
    st_oop handle;
    st_oop array;
    int fd;
    char *buffer;
    size_t total, size;
    ssize_t count;

    array = ST_STACK_POP (machine);
    handle = ST_STACK_POP (machine);
    if (st_object_format (array) != ST_FORMAT_BYTE_ARRAY) {
	machine->success = false;
	ST_STACK_UNPOP (machine, 1);
	return;
    }
    if (st_object_format (handle) != ST_FORMAT_HANDLE) {
	machine->success = false;
	ST_STACK_UNPOP (machine, 2);
	return;
    }

    fd = ST_HANDLE_VALUE (handle);
    buffer = st_byte_array_bytes (array);
    size = st_smi_value (st_arrayed_object_size (array));
    
    total = 0;
    while (total < size) {
	count = write (fd, buffer + total, size - total);
	if (count < 0) {
	    machine->success = false;
	    ST_STACK_UNPOP (machine, 2);
	    return;
	}
	total += count;
    }

    /* leave receiver on stack */
}

static void
FileStream_seek (st_machine *machine)
{
    /* not implemented yet */
    abort ();
}

static void
FileStream_read (st_machine *machine)
{
    /* not implemented yet */
    abort (); 
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
    { "LargeInteger_printStringBase", LargeInteger_printStringBase   },
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
    { "Float_printStringBase",     Float_printStringBase   },

    { "Object_error",                  Object_error                },
    { "Object_class",                  Object_class                },
    { "Object_identityHash",           Object_identityHash         },
    { "Object_copy",                   Object_copy                 },
    { "Object_equivalent",             Object_equivalent           },
    { "Object_perform",                Object_perform               },
    { "Object_perform_withArguments",  Object_perform_withArguments },
    
    { "Behavior_new",                 Behavior_new                },
    { "Behavior_newSize",             Behavior_newSize            },
    { "Behavior_compile",             Behavior_compile            },


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

    { "System_exitWithResult",          System_exitWithResult },

    { "Character_value",                 Character_value },
    { "Character_characterFor",          Character_characterFor },

    { "BlockContext_value",              BlockContext_value               },
    { "BlockContext_valueWithArguments", BlockContext_valueWithArguments  },

    { "FileStream_open",        FileStream_open      },
    { "FileStream_close",       FileStream_close     },
    { "FileStream_read",        FileStream_read      },
    { "FileStream_write",       FileStream_write     },
    { "FileStream_seek",        FileStream_seek      },

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

