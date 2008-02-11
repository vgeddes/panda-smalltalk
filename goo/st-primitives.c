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

static void
SmallInteger_add (void)
{
}

static void
SmallInteger_minus (void)
{
}

static void
SmallInteger_lt (void)
{
}

static void
SmallInteger_gt (void)
{
}

static void
SmallInteger_le (void)
{
}

static void
SmallInteger_ge (void)
{
}

static void
SmallInteger_eq (void)
{
}

static void
SmallInteger_ne (void)
{
}

static void
SmallInteger_mul (void)
{
}

static void
SmallInteger_div (void)
{
}

static void
SmallInteger_bitOr (void)
{
}

static void
SmallInteger_bitXor (void)
{
}

static void
SmallInteger_bitAnd (void)
{
}

static void
SmallInteger_bitShift (void)
{
}

static void
Float_add (void)
{
}

static void
Float_minus (void)
{
}

static void
Float_lt (void)
{
}

static void
Float_gt (void)
{
}

static void
Float_le (void)
{
}

static void
Float_ge (void)
{
}

static void
Float_eq (void)
{
}

static void
Float_ne (void)
{
}

static void
Float_mul (void)
{
}

static void
Float_div (void)
{
}

static void
Float_truncated (void)
{
}

static void
Float_fractionPart (void)
{
}

static void
Float_exponent (void)
{
}

static void
Object_at (void)
{
}

static void
Object_at_put (void)
{
}

static void
Object_size (void)
{
}

static void
Object_class (void)
{
}

static void
Object_hash (void)
{
}

static void
Object_identityHash (void)
{
}

static void
Object_copy (void)
{
}

static void
Object_equivalent (void)
{
}

static void
Object_perform (void)
{
}

static void
Object_performWithArguments (void)
{
}

static void
Behavior_new (void)
{
}

static void
Behavior_newArgument (void)
{
}

static void
ByteArray_at (void)
{
}

static void
ByteArray_at_put (void)
{
}

static void
BlockContext_value (void)
{
}

static void
BlockContext_valueArg (void)
{
}

static void
BlockContext_valueWithArgs (void)
{
}

static void
SystemDictionary_quit (void)
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

/* returns -1 if there no primitive function corresponding
 * to the given name */
int
st_primitive_index_for_name (const char *name)
{
    g_assert (name != NULL);
    for (int i = 0; i < G_N_ELEMENTS (st_primitives); i++)
	if (strcmp (name, st_primitives[i].name) == 0)
	    return i;
    return -1;
}

