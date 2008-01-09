

#ifndef __ST_PRIMITIVES_H__
#define __ST_PRIMITIVES_H__

#include <glib.h>

G_BEGIN_DECLS

typedef void (*st_prim_func) (void *todo);


int             st_get_primitive_index_for_name (const char *name);

st_prim_func   st_get_primitive_func_for_index (guint index);


G_END_DECLS

#endif /* __ST_PRIMITIVES_H__ */
