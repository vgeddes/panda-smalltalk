#include "st-context.h"
#include "st-array.h"
#include "st-method.h"

#include "st-object.h"
#include "st-universe.h"

st_oop
st_message_new (st_oop selector, st_oop arguments)
{
    st_oop object;

    object = st_object_new (om->moving_space, st_global_get ("Message"));

    st_heap_object_body (object)[0] = selector;
    st_heap_object_body (object)[1] = arguments;

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

    class = st_heap_object_class (object);

    size = (sizeof (struct st_header) / sizeof (st_oop)) + st_smi_value (ST_BEHAVIOR (class)->instance_size);
    
    if (class == st_method_context_class)
	size += ST_METHOD_CONTEXT_STACK_SIZE (object);
    else
	size += ST_BLOCK_CONTEXT_STACK_SIZE (object);

    return size;
}

static void
context_contents (st_oop object, struct contents *contents)
{
    st_oop class;

    class = st_heap_object_class (object);
    contents->oops = st_heap_object_body (object);
    contents->size = st_smi_value (ST_BEHAVIOR (class)->instance_size);
    
    if (class == st_method_context_class)
	contents->size += ST_METHOD_CONTEXT_STACK_SIZE (object);
    else
	contents->size += ST_BLOCK_CONTEXT_STACK_SIZE (object);
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
