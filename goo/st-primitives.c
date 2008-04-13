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
#include "st-object.h"

#include <string.h>

static void
SmallInteger_add (STInterpreter *state)
{
    st_oop x = ST_STACK_POP (state);
    st_oop y = ST_STACK_POP (state);
    st_oop z;

    state->success = true;

    if (!st_object_is_smi (x))
	goto out;

    if (!st_object_is_smi (y))
	goto out;

    z = st_smi_new (st_smi_value (x) + st_smi_value (y));
    
    ST_STACK_PUSH (state, z);

out:
    state->success = false;

}

static void
SmallInteger_minus (STInterpreter *state)
{
    st_oop x = ST_STACK_POP (state);
    st_oop y = ST_STACK_POP (state);
    st_oop z;

    state->success = true;

    if (!st_object_is_smi (x))
	goto out;

    if (!st_object_is_smi (y))
	goto out;

    z = st_smi_new (st_smi_value (y) - st_smi_value (x));
    
    ST_STACK_PUSH (state, z);

out:
    state->success = false;
}

static void
SmallInteger_lt (STInterpreter *state)
{
}

static void
SmallInteger_gt (STInterpreter *state)
{
}

static void
SmallInteger_le (STInterpreter *state)
{
}

static void
SmallInteger_ge (STInterpreter *state)
{
}

static void
SmallInteger_eq (STInterpreter *state)
{
}

static void
SmallInteger_ne (STInterpreter *state)
{
}

static void
SmallInteger_mul (STInterpreter *state)
{
}

static void
SmallInteger_div (STInterpreter *state)
{
}

static void
SmallInteger_bitOr (STInterpreter *state)
{
}

static void
SmallInteger_bitXor (STInterpreter *state)
{
}

static void
SmallInteger_bitAnd (STInterpreter *state)
{
}

static void
SmallInteger_bitShift (STInterpreter *state)
{
}

static void
Float_add (STInterpreter *state)
{
}

static void
Float_minus (STInterpreter *state)
{
}

static void
Float_lt (STInterpreter *state)
{
}

static void
Float_gt (STInterpreter *state)
{
}

static void
Float_le (STInterpreter *state)
{
}

static void
Float_ge (STInterpreter *state)
{
}

static void
Float_eq (STInterpreter *state)
{
}

static void
Float_ne (STInterpreter *state)
{
}

static void
Float_mul (STInterpreter *state)
{
}

static void
Float_div (STInterpreter *state)
{
}

static void
Float_truncated (STInterpreter *state)
{
}

static void
Float_fractionPart (STInterpreter *state)
{
}

static void
Float_exponent (STInterpreter *state)
{
}

static void
Object_at (STInterpreter *state)
{
}

static void
Object_at_put (STInterpreter *state)
{
}

static void
Object_size (STInterpreter *state)
{
}

static void
Object_class (STInterpreter *state)
{
}

static void
Object_hash (STInterpreter *state)
{
}

static void
Object_identityHash (STInterpreter *state)
{
}

static void
Object_copy (STInterpreter *state)
{
}

static void
Object_equivalent (STInterpreter *state)
{
}

static void
Object_perform (STInterpreter *state)
{
}

static void
Object_performWithArguments (STInterpreter *state)
{
}

static void
Behavior_new (STInterpreter *state)
{
}

static void
Behavior_newArgument (STInterpreter *state)
{
}

static void
ByteArray_at (STInterpreter *state)
{
}

static void
ByteArray_at_put (STInterpreter *state)
{
}

static void
BlockContext_value (STInterpreter *state)
{
}

static void
BlockContext_valueArg (STInterpreter *state)
{
}

static void
BlockContext_valueWithArgs (STInterpreter *state)
{
}

static void
SystemDictionary_quit (STInterpreter *state)
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
    { "Float_fractionPart",    Float_fractionPart  },
    { "Float_exponent",        Float_exponent      },

    { "Object_at",                    Object_at                   },
    { "Object_at_put",                Object_at_put               },
    { "Object_size",                  Object_size                 },
    { "Object_class",                 Object_class                },
    { "Object_hash",                  Object_hash                 },
    { "Object_identityHash",          Object_identityHash         },
    { "Object_copy",                  Object_copy                 },
    { "Object_equivalent",            Object_equivalent           },
    { "Object_perform",               Object_perform              },
    { "Object_performWithArguments",  Object_performWithArguments },
    
    { "Behavior_new",                 Behavior_new                },
    { "Behavior_newArgument",         Behavior_newArgument        },

    { "ByteArray_at",                 ByteArray_at                },
    { "ByteArray_at_put",             ByteArray_at_put            },

    { "BlockContext_value",           BlockContext_value          },
    { "BlockContext_valueArg",        BlockContext_valueArg       },
    { "BlockContext_valueWithArgs",   BlockContext_valueWithArgs  },

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

