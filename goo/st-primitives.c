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
#include "st-interpreter.h"
#include "st-array.h"
#include "st-word-array.h"
#include "st-byte-array.h"
#include "st-large-integer.h"
#include "st-float.h"
#include "st-object.h"
#include "st-context.h"
#include "st-method.h"
#include "st-symbol.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>


INLINE void
set_success (STExecutionState *es, bool success)
{
    es->success = es->success && success;
}

INLINE st_smi
pop_integer (STExecutionState *es)
{
    st_oop object = ST_STACK_POP (es);
    
    set_success (es, st_object_is_smi (object));    

    return st_smi_value (object);
}

static void
SmallInteger_add (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result;

    if (es->success)
	result = st_smi_new (x + y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_minus (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_smi_new (x - y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_lt (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = (x < y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_gt (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = (x > y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_le (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = (x <= y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_ge (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = (x >= y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_eq (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = (x == y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_ne (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = (x != y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_mul (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_smi_new (x * y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

/* selector: / */
static void
SmallInteger_div (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result;

    set_success (es, y != 0);

    if (x % y != 0)
	set_success (es, false);
    
    if (es->success)
	result = st_smi_new (x / y);
    
    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_intDiv (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result;

    set_success (es, y != 0);
    
    if (es->success)
	result = st_smi_new (x / y);
    
    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_mod (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    set_success (es, y != 0);

    if (es->success)
	result = st_smi_new (x % y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_bitOr (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_smi_new (x | y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_bitXor (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_smi_new (x ^ y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_bitAnd (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_smi_new (x & y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_bitShift (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success) {
	if (y > 0)
	    result = st_smi_new (x << y);
	else if (y < 0)
	    result = st_smi_new (x >> (-y));
	else
	    result = st_smi_new (x);
    }

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
SmallInteger_asFloat (STExecutionState *es)
{
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_float_new ((double) x);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

INLINE st_oop
pop_large_integer (STExecutionState *es)
{
    st_oop object = ST_STACK_POP (es);

    set_success (es, st_object_class (object) == st_large_integer_class);
    
    return object;
}

static void
LargeInteger_add (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_add (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_sub (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_sub (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_mul (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_mul (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_div (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_div (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_mod (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_mod (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_gcd (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_gcd (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_lcm (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_lcm (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_eq (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_eq (x, y, &error) ? st_true : st_false;

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_ne (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_eq (x, y, &error) ? st_false : st_true;

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_lt (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_lt (x, y, &error) ? st_true : st_false;

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_gt (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_gt (x, y, &error) ? st_true : st_false;

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_le (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_le (x, y, &error) ? st_true : st_false;

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_ge (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_ge (x, y, &error) ? st_true : st_false;

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_squared (STExecutionState *es)
{
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_sqr (x, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_bitOr (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_bitor (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_bitAnd (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_bitand (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_bitXor (STExecutionState *es)
{
    st_oop y = pop_large_integer (es);
    st_oop x = pop_large_integer (es);
    st_oop result;
    bool error;
    
    if (es->success)
	result = st_large_integer_bitxor (x, y, &error);

    set_success (es, error == false);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
LargeInteger_printString (STExecutionState *es)
{
    st_smi radix = pop_integer (es);
    st_oop x     = pop_large_integer (es);
    char   *string;
    st_oop result;

    if (radix < 2 || radix > 36)
	set_success (es, false);

    if (es->success) {
	string = st_large_integer_to_string (x, radix);
	result = st_string_new (string);
    }

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}


INLINE st_oop
pop_float (STExecutionState *es)
{
    st_oop object = ST_STACK_POP (es);

    set_success (es, st_object_class (object) == st_float_class);
    
    return object;
}

static void
Float_add (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_float_new (st_float_value (x) + st_float_value (y));

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_minus (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_float_new (st_float_value (x) - st_float_value (y));

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_lt (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = isless (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_gt (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = isgreater (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_le (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = islessequal (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_ge (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = isgreaterequal (st_float_value (x), st_float_value (y)) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_eq (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = (st_float_value (x) == st_float_value (y)) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_ne (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = (st_float_value (x) != st_float_value (y)) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_mul (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_float_new (st_float_value (x) * st_float_value (y));

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_div (STExecutionState *es)
{
    st_oop y = pop_float (es);
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    set_success (es, y != 0);

    if (es->success)
	result = st_float_new (st_float_value (x) / st_float_value (y));

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_truncated (STExecutionState *es)
{
    st_oop x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = (int) st_float_value (x);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 1);
}

static void
print_backtrace (STExecutionState *es)
{
    st_oop context;
    
    context = es->context;

    while (context != st_nil) {

	char *selector;
	char *klass;
	st_oop home;
	st_oop receiver;

	if (st_object_class (context) == st_block_context_class)
	    home = st_block_context_home (context);
	else
	    home = context;

	receiver = st_method_context_receiver (home);

	selector = (char*) st_byte_array_bytes (st_method_selector (st_method_context_method (home)));
	klass    = (char*) st_byte_array_bytes (st_class_name (st_object_class (receiver)));

	printf ("%s>>#%s", klass, selector);
	if (st_object_class (context) == st_block_context_class)
	    printf ("[]\n");
	else
	    printf ("\n");

	if (st_object_class (context) == st_block_context_class)
	    context = st_block_context_caller (context);
	else
	    context = st_context_part_sender (context);
    }
}

static void
Object_error (STExecutionState *es)
{
    st_oop message;

    message = ST_STACK_POP (es);

    printf ("= An error occurred during program execution\n");
    printf ("= %s\n", st_byte_array_bytes (message));

    printf ("\nTraceback:\n");
    print_backtrace (es); 

    exit (1);
}

static void
Object_class (STExecutionState *es)
{
    st_oop object;

    object = ST_STACK_POP (es);

    ST_STACK_PUSH (es, st_object_class (object));
}

static void
Object_identityHash (STExecutionState *es)
{
    st_oop object;
    st_oop result;
    
    object = ST_STACK_POP (es);
    
    if (st_object_is_smi (object))
	result = st_smi_new (st_smi_hash (object));
    else
	result = st_smi_new (st_heap_object_hash (object));
    
    ST_STACK_PUSH (es, result);
}

static void
Object_copy (STExecutionState *es)
{
    g_assert_not_reached ();
}

static void
Object_equivalent (STExecutionState *es)
{
    st_oop y = ST_STACK_POP (es);
    st_oop x = ST_STACK_POP (es);
    
    ST_STACK_PUSH (es, ((x == y) ? st_true : st_false));
}


static void
Object_perform (STExecutionState *es)
{
    st_oop receiver;
    st_oop selector;
    st_oop method;
    guint selector_index;

    selector = es->message_selector;
    es->message_selector = es->stack[es->sp - es->message_argcount];
    receiver = es->message_receiver;

    set_success (es, st_object_is_symbol (es->message_selector));
    method = st_interpreter_lookup_method (es, st_object_class (receiver));
    set_success (es, st_method_arg_count (method) == (es->message_argcount - 1));

    if (es->success) {

	selector_index = es->sp - es->message_argcount;

	st_oops_copy (es->stack + selector_index,
		      es->stack + selector_index + 1,
		      es->message_argcount - 1);

	es->sp -= 1;
	es->message_argcount -= 1;
	st_interpreter_execute_method (es, method);

    } else {
	es->message_selector = selector;
    }
}

static void
Object_perform_withArguments (STExecutionState *es)
{
    st_oop receiver;
    st_oop selector;
    st_oop method;
    st_oop array;
    st_smi array_size;

    array = ST_STACK_POP (es);

    set_success (es, st_object_class (array) == st_array_class);

    if (st_object_class (es->context) == st_block_context_class)
	method = st_method_context_method (st_block_context_home (es->context));
    else
	method = st_method_context_method (es->context);

    array_size = st_smi_value (st_array_size (array));
    set_success (es, (es->sp + array_size - 1) < st_method_stack_depth (method));

    if (es->success) {
	
	selector = es->message_selector;
	es->message_selector = ST_STACK_POP (es);
	receiver = ST_STACK_PEEK (es);
	es->message_argcount = array_size;

	set_success (es, st_object_is_symbol (es->message_selector));
    
	st_oops_copy (es->stack + es->sp,
		      st_array_element (array, 1),
		      array_size);

	es->sp += array_size;

	method = st_interpreter_lookup_method (es, st_object_class (receiver));
	set_success (es, st_method_arg_count (method) == array_size);
    
	if (es->success) {
	    st_interpreter_execute_method (es, method);
	} else {
	    es->sp -= es->message_argcount;
	    ST_STACK_PUSH (es, es->message_selector);
	    ST_STACK_PUSH (es, array);
	    es->message_argcount = 2;
	    es->message_selector = selector;
	}

    } else {
	ST_STACK_UNPOP (es, 1);
    }
}

static void
Behavior_new (STExecutionState *es)
{
    st_oop klass;
    st_oop instance;

    klass = ST_STACK_POP (es);

    instance = st_object_new (klass);

    ST_STACK_PUSH (es, instance);
}

static void
Behavior_newSize (STExecutionState *es)
{
    st_oop klass;
    st_smi size;
    st_oop instance;

    size = pop_integer (es);
    klass = ST_STACK_POP (es);

    instance = st_object_new_arrayed (klass, size);

    ST_STACK_PUSH (es, instance);
}

static void
Array_at (STExecutionState *es)
{
    st_smi index    = pop_integer (es);
    st_oop receiver = ST_STACK_POP (es);
    st_oop result;
	
    if (!st_array_range_check (receiver, index)) {
	set_success (es, false);
	ST_STACK_UNPOP (es, 2);
	return;
    }
    
    result = st_array_at (receiver, index);	    
    
    ST_STACK_PUSH (es, result);
}

static void
Array_at_put (STExecutionState *es)
{
    st_oop object   = ST_STACK_POP (es);
    st_smi index    = pop_integer (es);
    st_oop receiver = ST_STACK_POP (es);

    if (!st_array_range_check (receiver, index)) {
	set_success (es, false);
	ST_STACK_UNPOP (es, 3);
	return;
    }
    
    st_array_at_put (receiver, index, object);	    
    
    ST_STACK_PUSH (es, object);
}


static void
Array_size (STExecutionState *es)
{
    st_oop object;

    object = ST_STACK_POP (es);

    ST_STACK_PUSH (es, st_array_size (object));
}


static void
ByteArray_at (STExecutionState *es)
{
    st_smi index    = pop_integer (es);
    st_oop receiver = ST_STACK_POP (es);
    st_oop result;
	
    if (!es->success) {
	ST_STACK_UNPOP (es, 2);
	return;
    }

    if (!st_byte_array_range_check (receiver, index)) {
	set_success (es, false);
	ST_STACK_UNPOP (es, 2);
	return;
    }
    
    result = st_smi_new (st_byte_array_at (receiver, index));  
    
    ST_STACK_PUSH (es, result);
}

static void
ByteArray_at_put (STExecutionState *es)
{
    st_smi byte     = pop_integer (es);
    st_smi index    = pop_integer (es);
    st_oop receiver = ST_STACK_POP (es);

    if (!es->success) {
	ST_STACK_UNPOP (es, 3);
	return;
    }

    if (!st_byte_array_range_check (receiver, index)) {
	set_success (es, false);
	ST_STACK_UNPOP (es, 3);
	return;
    }
    
    st_byte_array_at_put (receiver, index, byte);	    
    
    ST_STACK_PUSH (es, st_smi_new (byte));
}

static void
ByteArray_size (STExecutionState *es)
{
    st_oop object;

    object = ST_STACK_POP (es);

    ST_STACK_PUSH (es, st_byte_array_size (object));
}

static void
ByteString_at (STExecutionState *es)
{
    st_smi  index = pop_integer (es);
    st_oop  receiver = ST_STACK_POP (es);
    st_oop  character;
    char   *charptr;

    if (G_UNLIKELY (!es->success)) {
	ST_STACK_UNPOP (es, 2);
	return;
    }

    if (G_UNLIKELY (!st_byte_array_range_check (receiver, index))) {
	set_success (es, false);
	ST_STACK_UNPOP (es, 2);
	return;	
    }
    
    charptr = g_utf8_offset_to_pointer ((const char *) st_byte_array_bytes (receiver),
					index - 1);
    
    character = st_character_new (g_utf8_get_char (charptr));

    ST_STACK_PUSH (es, character);
}

static void
ByteString_size (STExecutionState *es)
{
    st_oop object;
    glong size;

    object = ST_STACK_POP (es);

    size = g_utf8_strlen ((const char *) st_byte_array_bytes (object), -1);

    /* TODO: allow size to go into a LargeInteger on overflow */
    ST_STACK_PUSH (es, st_smi_new (size));
}

static void
ByteString_compare (STExecutionState *es)
{
    st_oop argument = ST_STACK_POP (es);
    st_oop receiver = ST_STACK_POP (es);
    int order;

    if (!st_object_is_string (argument))
	set_success (es, false);

    if (es->success)
	order = g_utf8_collate ((const char *) st_byte_array_bytes (receiver),
				(const char *) st_byte_array_bytes (argument));

    if (es->success)
	ST_STACK_PUSH (es, st_smi_new (order));
    else
	ST_STACK_UNPOP (es, 2);
}

static int
compare_ordinal (const char *str1, const char *str2)
{
    char *str1_normalized;
    char *str2_normalized;
    glong diff = 0;
    char *p, *q;

    str1_normalized = g_utf8_normalize (str1, -1, G_NORMALIZE_ALL);
    str2_normalized = g_utf8_normalize (str2, -1, G_NORMALIZE_ALL);
    
    p = str1_normalized;
    q = str2_normalized;

    while (*p && *q) {
	diff = g_utf8_get_char (p) - g_utf8_get_char (q);
	if (diff != 0)
	    break;
	p = g_utf8_next_char(p);
	q = g_utf8_next_char(q);    
    }

    if (diff == 0)
	diff = g_utf8_strlen (str1_normalized, -1) - g_utf8_strlen (str2_normalized, -1);

    g_free (str1_normalized);
    g_free (str2_normalized);

    return (int) diff;
}

static void
ByteString_compareOrdinal (STExecutionState *es)
{
    st_oop argument = ST_STACK_POP (es);
    st_oop receiver = ST_STACK_POP (es);
    int order;

    if (!st_object_is_string (argument))
	set_success (es, false);

    if (es->success)
	order = compare_ordinal ((const char *) st_byte_array_bytes (receiver),
				(const char *) st_byte_array_bytes (argument));

    if (es->success)
	ST_STACK_PUSH (es, st_smi_new (order));
    else
	ST_STACK_UNPOP (es, 2);
}

static void
ByteString_reversed (STExecutionState *es)
{
    st_oop receiver = ST_STACK_POP (es);
    char  *reversed;

    if (!st_object_is_string (receiver))
	set_success (es, false);

    if (es->success) {
	reversed = g_utf8_strreverse ((const char *) st_byte_array_bytes (receiver), -1);
	ST_STACK_PUSH (es, st_string_new (reversed));
	g_free (reversed);
    } else {
	ST_STACK_UNPOP (es, 1);
    }
}

static void
ByteString_hash (STExecutionState *es)
{
    st_oop receiver = ST_STACK_POP (es);
    guint  hash;

    if (!st_object_is_string (receiver))
	set_success (es, false);

    if (es->success) {
	hash = st_byte_array_hash (receiver);
	ST_STACK_PUSH (es, st_smi_new (hash));
    } else {
	ST_STACK_UNPOP (es, 1);	
    }
}

static void
WideString_at (STExecutionState *es)
{
    st_smi index    = pop_integer (es);
    st_oop receiver = ST_STACK_POP (es);
    st_oop character;
	
    if (!es->success) {
	ST_STACK_UNPOP (es, 2);
	return;
    }

    if (!st_word_array_range_check (receiver, index)) {
	set_success (es, false);
	ST_STACK_UNPOP (es, 2);
	return;
    }
    
    character = st_character_new ((gunichar) st_word_array_at (receiver, index));

    ST_STACK_PUSH (es, character);
}

static void
WideString_at_put (STExecutionState *es)
{
    st_oop character = ST_STACK_POP (es);
    st_smi index    = pop_integer (es);
    st_oop receiver = ST_STACK_POP (es);
	
    if (!es->success) {
	ST_STACK_UNPOP (es, 3);
	return;
    }
  
    set_success (es, st_object_class (character) == st_character_class);

    if (!st_word_array_range_check (receiver, index)) {
	set_success (es, false);
	ST_STACK_UNPOP (es, 3);
	return;
    }

    st_word_array_at_put (receiver, index, (guint) st_character_value (character));

    ST_STACK_PUSH (es, character);
}

static void
WordArray_size (STExecutionState *es)
{
    st_oop receiver;

    receiver = ST_STACK_POP (es);

    ST_STACK_PUSH (es, st_word_array_size (receiver));
}

INLINE void
activate_block_context (STExecutionState *es)
{
    st_oop  block;
    st_smi  argcount;

    block = es->message_receiver;
    argcount = st_smi_value (st_block_context_argcount (block));
    if (argcount != es->message_argcount) {
	es->success = false;
	return;
    }

    st_oops_copy (st_block_context_stack (block),
		  es->stack + es->sp - argcount,
		  argcount);

    es->sp -= es->message_argcount + 1;
    
    st_context_part_ip (block) = st_block_context_initial_ip (block);
    st_context_part_sp (block) = st_smi_new (argcount);
    st_block_context_caller (block) = es->context;

    st_interpreter_set_active_context (es, block);
}

static void
BlockContext_value (STExecutionState *es)
{
    activate_block_context (es);
}

static void
BlockContext_valueColon (STExecutionState *es)
{
    activate_block_context (es);
}

static void
BlockContext_value_value (STExecutionState *es)
{
    activate_block_context (es);
}

static void
BlockContext_value_value_value (STExecutionState *es)
{
    activate_block_context (es);
}

static void
BlockContext_valueWithArguments (STExecutionState *es)
{
    st_oop block;
    st_oop values;
    st_smi argcount;

    block  = es->message_receiver;
    values = ST_STACK_PEEK (es);

    if (st_object_class (values) != st_array_class) {
	st_interpreter_set_success (es, false);
	return;
    }

    argcount = st_smi_value (st_block_context_argcount (block));
    if (argcount != st_smi_value (st_array_size (values))) {
	st_interpreter_set_success (es, false);
	return;
    }
    
    st_oops_copy (st_block_context_stack (block),
		  st_array_element (values, 1),
		  argcount);
    
    es->sp -= es->message_argcount + 1;    
    
    st_context_part_ip (block) = st_block_context_initial_ip (block);
    st_context_part_sp (block) = st_smi_new (argcount);
    st_block_context_caller (block) = es->context;

    st_interpreter_set_active_context (es, block);
}

static void
SystemDictionary_quit (STExecutionState *es)
{
}

const STPrimitive st_primitives[] = {
    { "SmallInteger_add",      SmallInteger_add      },
    { "SmallInteger_minus",    SmallInteger_minus    },
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
    { "SmallInteger_asFloat",  SmallInteger_asFloat  },

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
    { "LargeInteger_mod",      LargeInteger_mod      },
    { "LargeInteger_squared",  LargeInteger_squared  },
    { "LargeInteger_bitOr",    LargeInteger_bitOr    },
    { "LargeInteger_bitXor",   LargeInteger_bitXor   },
    { "LargeInteger_bitAnd",   LargeInteger_bitAnd   },
    { "LargeInteger_printString", LargeInteger_printString   },

    { "Float_add",             Float_add           },
    { "Float_minus",           Float_minus         },
    { "Float_lt",              Float_lt            },
    { "Float_gt",              Float_gt            },
    { "Float_le",              Float_le            },
    { "Float_ge",              Float_ge            },
    { "Float_eq",              Float_eq            },
    { "Float_ne",              Float_ne            },
    { "Float_mul",             Float_mul           },
    { "Float_div",             Float_div           },
    { "Float_truncated",       Float_truncated     },

    { "Object_error",                  Object_error                },
    { "Object_class",                  Object_class                },
    { "Object_identityHash",           Object_identityHash         },
    { "Object_copy",                   Object_copy                 },
    { "Object_equivalent",             Object_equivalent           },
    { "Object_perform",                Object_perform               },
    { "Object_perform_withArguments",  Object_perform_withArguments },
    
    { "Behavior_new",                 Behavior_new                },
    { "Behavior_newSize",             Behavior_newSize            },

    { "Array_at",                     Array_at                    },
    { "Array_at_put",                 Array_at_put                },
    { "Array_size",                   Array_size                  },
    { "ByteArray_at",                 ByteArray_at                },
    { "ByteArray_at_put",             ByteArray_at_put            },
    { "ByteArray_size",               ByteArray_size              },

    { "ByteString_at",                 ByteString_at                },
    { "ByteString_size",               ByteString_size              },
    { "ByteString_compare",            ByteString_compare           },
    { "ByteString_compareOrdinal",     ByteString_compareOrdinal    },
    { "ByteString_reversed",           ByteString_reversed          },
    { "ByteString_hash",               ByteString_hash              },

    { "WideString_at",                 WideString_at                },
    { "WideString_at_put",             WideString_at_put            },

    { "WordArray_size",                WordArray_size               },


    { "BlockContext_value",               BlockContext_value               },
    { "BlockContext_valueColon",          BlockContext_valueColon          },
    { "BlockContext_value_value",         BlockContext_value_value         },
    { "BlockContext_value_value_value",   BlockContext_value_value_value   },
    { "BlockContext_valueWithArguments",  BlockContext_valueWithArguments  },

    { "SystemDictionary_quit",        SystemDictionary_quit       },
};

/* returns 0 if there no primitive function corresponding
 * to the given name */
int
st_primitive_index_for_name (const char *name)
{
    g_assert (name != NULL);
    for (int i = 0; i < G_N_ELEMENTS (st_primitives); i++)
	if (streq (name, st_primitives[i].name))
	    return i;
    return -1;
}

