
#ifndef __ST_VIRTUAL_SPACE__
#define __ST_VIRTUAL_SPACE__

#include <glib.h>

typedef struct _GooVirtualSpace GooVirtualSpace;

GooVirtualSpace  *st_virtual_space_new  (gsize size);

static inline char *
st_virtual_space_get_start (GooVirtualSpace *space)
{
	return space->start;
}

static inline char *
st_virtual_space_get_end (GooVirtualSpace *space)
{
	return space->end;
}
						       
void  st_virtual_space_destroy  (GooVirtualSpace *space);


#endif /* __ST_VIRTUAL_SPACE__ */
