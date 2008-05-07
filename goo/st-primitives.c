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
#include "st-byte-array.h"
#include "st-float.h"
#include "st-object.h"
#include "st-context.h"
#include <math.h>
#include <string.h>

INLINE st_smi
pop_integer (STExecutionState *es)
{
    st_oop object = ST_STACK_POP (es);
    
    st_interpreter_set_success (es, st_object_is_smi (object));    

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

/* selector: // */
static void
SmallInteger_div (STExecutionState *es)
{
    st_smi y = pop_integer (es);
    st_smi x = pop_integer (es);
    st_oop result = st_nil;

    if (es->success) {
	es->success = y != 0;
	if (es->success)
	    result = st_smi_new (x / y);
    }

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

    if (es->success) {
	es->success = y != 0;
	if (es->success)
	    result = st_smi_new (x % y);
    }

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



INLINE st_smi
pop_float (STExecutionState *es)
{
    st_oop object = ST_STACK_POP (es);

    es->success = st_object_class (object) == st_float_class;

    return st_float_value (object);
}


static void
Float_add (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_float_new (x + y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_minus (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_float_new (x - y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_lt (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = isless (x, y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_gt (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = isgreater (x, y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_le (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = islessequal (x, y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_ge (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = isgreaterequal (x, y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_eq (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = (x == y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_ne (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = (x != y) ? st_true : st_false;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_mul (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = st_float_new (x * y);

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_div (STExecutionState *es)
{
    double y = pop_float (es);
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success) {
	es->success = y != 0;
	result = st_float_new (x / y);
    }

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 2);
}

static void
Float_truncated (STExecutionState *es)
{
    double x = pop_float (es);
    st_oop result = st_nil;

    if (es->success)
	result = (int) x;

    if (es->success)
	ST_STACK_PUSH (es, result);
    else
	ST_STACK_UNPOP (es, 1);
}


static void
Object_at (STExecutionState *es)
{
    st_oop object;
    st_smi index;
    
    index = pop_integer (es);
    object = ST_STACK_POP (es);

    if (!es->success) {
	ST_STACK_UNPOP (es, 2);
	return;
    }

    if (st_object_is_array (object)) {
	
	if (!st_array_range_check (object, index)) {
	    es->success = false;
	    ST_STACK_UNPOP (es, 2);
	    return;
	}
	    
	ST_STACK_PUSH (es, st_array_at (object, index));

    } else if (st_object_is_byte_array (object)) {
	
	if (!st_byte_array_range_check (object, index)) {
	    es->success = false;
	    ST_STACK_UNPOP (es, 2);
	    return;
	}
	    
	ST_STACK_PUSH (es, st_smi_new (st_byte_array_at (object, index)));

    } else {
	es->success = false;

    }
}

static void
Object_at_put (STExecutionState *es)
{
    st_oop object;
    st_oop put_object;
    st_smi index;

    put_object = ST_STACK_POP (es);
    index = pop_integer (es);
    object = ST_STACK_POP (es);

    if (!es->success) {
	ST_STACK_UNPOP (es, 2);
	return;
    }

    if (st_object_is_array (object)) {
	
	if (!st_array_range_check (object, index)) {
	    es->success = false;
	    ST_STACK_UNPOP (es, 2);
	    return;
	}

	st_array_at_put (object, index, put_object);
	ST_STACK_PUSH (es, put_object);

    } else if (st_object_is_byte_array (object)) {
	
	if (!st_byte_array_range_check (object, index)) {
	    es->success = false;
	    ST_STACK_UNPOP (es, 2);
	    return;
	}

	if (!st_object_is_smi (put_object)) {
	    es->success = false;
	    ST_STACK_UNPOP (es, 2);
	    return;
	}

	st_byte_array_at_put (object, index, st_smi_value (put_object));	    
	ST_STACK_PUSH (es, put_object);

    } else {
	es->success = false;

    }
}

static void
Object_size (STExecutionState *es)
{

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
}

static void
Object_performWithArguments (STExecutionState *es)
{
}

static void
Behavior_new (STExecutionState *es)
{
}

static void
Behavior_newArgument (STExecutionState *es)
{
}

static void
ByteArray_at (STExecutionState *es)
{
}

static void
ByteArray_at_put (STExecutionState *es)
{
} 

INLINE void
activate_block_context (STExecutionState *es)
{
    st_oop  block;
    st_smi  argcount;

    block = es->msg_receiver;
    argcount = st_smi_value (st_block_context_argcount (block));
    if (argcount != es->msg_argcount) {
	es->success = false;
	return;
    }

    st_oops_copy (st_block_context_stack (block),
		  es->stack + es->sp - argcount,
		  argcount);

    es->sp -= es->msg_argcount + 1;
    
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

    block  = es->msg_receiver;
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
    
    es->sp -= es->msg_argcount + 1;    
    
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
    { "SmallInteger_mod",      SmallInteger_div      },
    { "SmallInteger_bitOr",    SmallInteger_bitOr    },
    { "SmallInteger_bitXor",   SmallInteger_bitXor   },
    { "SmallInteger_bitAnd",   SmallInteger_bitAnd   },
    { "SmallInteger_bitShift", SmallInteger_bitShift },

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

    { "Object_at",                    Object_at                   },
    { "Object_at_put",                Object_at_put               },
    { "Object_size",                  Object_size                 },
    { "Object_class",                 Object_class                },
    { "Object_identityHash",          Object_identityHash         },
    { "Object_copy",                  Object_copy                 },
    { "Object_equivalent",            Object_equivalent           },
    { "Object_perform",               Object_perform              },
    { "Object_performWithArguments",  Object_performWithArguments },
    
    { "Behavior_new",                 Behavior_new                },
    { "Behavior_newArgument",         Behavior_newArgument        },

    { "ByteArray_at",                 ByteArray_at                },
    { "ByteArray_at_put",             ByteArray_at_put            },

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

