
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

    object = st_object_new (memory->moving_space, st_global_get ("Message"));

    ST_HEADER (object)->fields[0] = selector;
    ST_HEADER (object)->fields[1] = arguments;

    return object;
}

static st_oop
context_copy (st_oop object)
{
    fprintf (stderr, "not implemented");
    st_assert_not_reached ();
}

static st_uint
context_size (st_oop object)
{
    st_oop class;
    st_uint size;

    class = ST_HEADER (object)->class;
    size = ST_SIZE_OOPS (struct st_header) + st_smi_value (ST_BEHAVIOR (class)->instance_size);
    size += 32;
    
    return size;
}

static void
context_contents (st_oop object, struct contents *contents)
{
    st_oop class;

    class = ST_HEADER (object)->class;
    contents->oops = ST_HEADER (object)->fields;
    contents->size = st_smi_value (ST_BEHAVIOR (class)->instance_size);
    contents->size += 32;
}

st_descriptor *
st_context_descriptor (void)
{    
    static st_descriptor __descriptor =
	{ .allocate         = NULL,
	  .allocate_arrayed = NULL,
	  .copy             = context_copy,
	  .size             = context_size,
	  .contents         = context_contents,
	};
    
    return & __descriptor;
}
