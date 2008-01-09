/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */

#include <st-float-array.h>

ST_DEFINE_VTABLE (st_float, st_heap_object_vtable ())

static bool
is_arrayed (void)
{
	return true;
}

static st_oop_t
allocate_arrayed (st_oop_t klass, st_smi_t size)
{
	g_assert (size > 0);

	st_oop_t object = st_allocate_object ((sizeof (st_float_array_t) / sizeof (st_oop_t)) + size);
	st_object_initialize_header (object, klass);

	st_arrayed_object_set_size (object, size);
	
	double *elements = _ST_FLOAT_ARRAY (object)->elements;
	
	for (st_smi_t i = 0; i < size; i++)
		elements[i] = (double) 0;
	
}

static st_oop_t
allocate (st_oop_t klass)
{
    return allocate_arrayed (klass, 0);
}

static void
st_float_vtable_init (st_vtable_t *table)
{
    assert_static (sizeof (st_float_array_t) == (sizeof (st_header_t) + sizeof (st_oop_t)));

    table->allocate_arrayed = allocate_arrayed;
    table->allocate         = allocate;
	
    table->is_arrayed = is_arrayed;
}


