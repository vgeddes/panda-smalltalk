
#include "st-context.h"
#include "st-array.h"
#include "st-method.h"
#include "st-object.h"
#include "st-behavior.h"
#include "st-universe.h"

st_oop
st_message_new (st_oop selector, st_oop arguments)
{
    st_oop object;

    object = st_object_new (st_global_get ("Message"));

    ST_HEADER (object)->fields[0] = selector;
    ST_HEADER (object)->fields[1] = arguments;

    return object;
}

static st_uint
context_size (st_oop object)
{
    return ST_SIZE_OOPS (struct st_header) + st_object_instance_size (object) + 32;
}

static void
context_contents (st_oop object, st_oop **oops, st_uint *size)
{
    *oops = ST_HEADER (object)->fields;
    *size = st_object_instance_size (object) + 32;
}

st_descriptor *
st_context_descriptor (void)
{    
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = NULL,
	  .size             = context_size,
	  .contents         = context_contents,
	};
    
    return & __descriptor;
}
