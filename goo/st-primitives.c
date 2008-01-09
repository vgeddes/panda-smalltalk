

#include <st-primitives.h>

typedef struct {
	const char     *name;
	st_prim_func   func;
} PrimitiveEntry;

static void
SmallInteger_minus (ExecutionState es)
{
	st_oop_t receiver = stack_pop (es);
	st_oop_t arg      = stack_pop (es);
	
	long a  = st_small_integer_value (receiver);
	long b  = st_small_integer_value (arg);
	
	st_oop_t c = st_small_integer_new (a - b);
	
	stack_push (es, c);
}

static void
SmallInteger_add (ExecutionState es)
{
	st_oop_t receiver = stack_pop (es);
	st_oop_t arg      = stack_pop (es);
	
	long a  = st_small_integer_value (receiver);
	long b  = st_small_integer_value (arg);
	
	st_oop_t c = st_small_integer_new (a + b);
	
	stack_push (es, c);
}

static void
Object_class (ExecutionState es)
{
	st_oop_t receiver = stack_pop (es);
	
	st_oop_t klass = st_object_class (receiver);
	
	stack_push (es, klass);
}

static void
Object_size (ExecutionState es)
{
	st_oop_t receiver = stack_pop (es);
	
	st_oop_t size = st_array_object_size (receiver);
	
	stack_push (es, size);
}

static void
Object_hash (ExecutionState es)
{
	st_oop_t receiver = stack_pop (es);
	
	st_oop_t hash = st_small_integer_new (st_mark_get_hash (st_object_get_mark (receiver)));
	
	stack_push (es, hash);
}

static void
Object_at (ExecutionState es)
{
	st_oop_t receiver = stack_pop (es);
	st_oop_t index =    stack_pop (es);
	
	st_oop_t element = st_array_object_at (receiver, st_small_integer_value (index));
	
	stack_push (es, element);
}

static void
Object_at_put (ExecutionState es)
{
	st_oop_t receiver = stack_pop (es);
	st_oop_t index    = stack_pop (es);
	st_oop_t object   = stack_pop (es);
	
	st_array_object_at_put (receiver, st_small_integer_value (index), object);
	
	stack_push (es, object);
}

static const PrimitiveEntry prims_table[] =
{
	{ "SmallInteger_add"  ,  SmallInteger_add   },
	{ "SmallInteger_minus",  SmallInteger_minus },
	{ "SmallInteger_lt",     SmallInteger_lt    },
	{ "SmallInteger_gt",     SmallInteger_gt    },
	{ "SmallInteger_le",     SmallInteger_le    },
	{ "SmallInteger_ge",     SmallInteger_ge    },
	{ "SmallInteger_eq",     SmallInteger_eq    },
	{ "SmallInteger_ne",     SmallInteger_ne    },
	{ "SmallInteger_mul",    SmallInteger_mul   },
	{ "SmallInteger_div",    SmallInteger_div   },
	{ "SmallInteger_mod",    SmallInteger_div   },
	{ "SmallInteger_bitOr",  SmallInteger_bitOr   },
	{ "SmallInteger_bitXor", SmallInteger_bitXor  },
	{ "SmallInteger_bitAnd", SmallInteger_bitAnd  },
	{ "SmallInteger_bitShift", SmallInteger_bitShift },
	
	{ "Object_at",         Object_at     },
	{ "Object_at_put",     Object_at_put },
	{ "Object_size",       Object_size   },
	{ "Object_class",      Object_class  },
	{ "Object_hash",       Object_hash   },	
	
	{ "ByteArray_at",      ByteArray_at     },
	{ "ByteArray_at_put",  ByteArray_at_put },
};

/* returns -1 if there no primitive function corresponding
 * to the given name */
int
st_get_primitive_index_for_name (const char *name)
{
	for (int i=0; i < G_N_ELEMENTS (prims_table); i++)
		if (!strcmp (name, prims_table[i].name))
			return i;

	return -1;
}

st_prim_func
st_get_primitive_func_for_index (guint index)
{
	g_assert (index < G_N_ELEMENTS (prims_table));
	return prims_table[index].func;
}


