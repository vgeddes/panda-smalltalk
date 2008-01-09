/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */

#ifndef __ST_LARGE_INTEGER__
#define __ST_LARGE_INTEGER__

#include <tommath.h>
#include <st-heap-object.h>

typedef struct
{
	st_header_t header;
	
	mp_int      value;
	
} st_large_integer_t;


st_oop_t       st_large_integer_new              (mp_int *value);

st_oop_t       st_large_integer_new_from_string  (const char *string, guint radix);

char          *st_large_integer_to_string        (st_oop_t integer, guint radix);

INLINE mp_int *st_large_integer_value            (st_oop_t integer);

st_vtable_t   *st_large_integer_vtable           (void);


/* inline definitions */
#define _ST_LARGE_INTEGER(oop) ((st_large_integer_t *) ST_POINTER (oop))

mp_int *
st_large_integer_value (st_oop_t integer)
{
    return & _ST_LARGE_INTEGER (integer)->value;
}

#endif /* __ST_LARGE_INTEGER */ 


