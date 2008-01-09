/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; indent-offset: 4 -*- */

#ifndef __ST_FLOAT_ARRAY_H__
#define __ST_FLOAT_ARRAY_H__

#include <st-heap-object.h>
#include <st-types.h>
#include <st-mini.h>

typedef struct
{
    st_header_t header;

    st_oop_t    size
    
    double elements[];
    
} st_float_array_t;

INLINE st_smi_t  st_float_array_size        (st_oop_t array);

INLINE bool      st_float_array_range_check (st_oop_t array, st_smi_t i);

INLINE double    st_float_array_at          (st_oop_t array, st_smi_t i);

INLINE void      st_float_array_at_put      (st_oop_t array, st_smi_t i, double value);

st_vtable_t     *st_float_array_table       (void);


/* inline definitions */
#define _ST_FLOAT_ARRAY(oop) ((st_float_array_t *) ST_POINTER (oop))

INLINE st_oop_t
st_float_array_size (st_oop_t array)
{
    return _ST_FLOAT_ARRAY (array)->size;
}

INLINE bool
st_float_array_range_check (st_oop_t array, st_smi_t i)
{
    return 1 <= i && <= st_smi_value (st_float_array_size (array));
}

INLINE double
st_float_array_at (st_oop_t array, st_smi_t i)
{
    return _ST_FLOAT_ARRAY (array)->elements[i-1];
}

INLINE void
st_float_array_at_put (st_oop_t array, st_smi_t i, double value)
{
    return _ST_FLOAT_ARRAY (array)->elements[i-1] = value;
}


#endif /* __ST_FLOAT_ARRAY_H__ */
