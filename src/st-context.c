

#include "st-context.h"
#include "st-array.h"
#include "st-method.h"

#include "st-object.h"
#include "st-universe.h"

st_oop
st_method_context_new (st_oop sender, st_oop receiver, st_oop method)
{
    st_oop  context;
    int     stack_size;
    int     temps_size;
    st_oop *stack;

    temps_size = st_method_get_temp_count (method) + st_method_get_arg_count (method);
    stack_size = st_method_get_stack_depth (method) + temps_size;

    context = st_space_allocate_object (om->moving_space, ST_TYPE_SIZE (struct st_method_context) + stack_size);
    st_heap_object_initialize_header (context, st_method_context_class);

    ST_CONTEXT_PART (context)->sender     = sender;
    ST_CONTEXT_PART (context)->ip         = st_smi_new (0);
    ST_CONTEXT_PART (context)->sp         = st_smi_new (0);
    ST_METHOD_CONTEXT (context)->receiver = receiver;
    ST_METHOD_CONTEXT (context)->method   = method;

    stack = ST_METHOD_CONTEXT (context)->stack;
    for (st_uint i=0; i < stack_size; i++)
	stack[i] = st_nil;

    return context;
}

st_oop
st_block_context_new (st_oop home, st_uint initial_ip, st_uint argcount)
{
    st_oop context;
    st_oop method;
    st_oop *stack;
    int stack_size;

    method =  ST_METHOD_CONTEXT (home)->method;

    stack_size = st_method_get_stack_depth (method);
    stack_size += argcount;

    context = st_space_allocate_object (om->moving_space, ST_TYPE_SIZE (struct st_block_context) + stack_size);
    st_heap_object_initialize_header (context, st_block_context_class);

    ST_CONTEXT_PART (context)->sender = st_nil;
    ST_CONTEXT_PART (context)->ip     = st_smi_new (0);
    ST_CONTEXT_PART (context)->sp     = st_smi_new (0);

    ST_BLOCK_CONTEXT (context)->initial_ip = st_smi_new (initial_ip);
    ST_BLOCK_CONTEXT (context)->argcount   = st_smi_new (argcount);
    ST_BLOCK_CONTEXT (context)->caller     = st_nil;
    ST_BLOCK_CONTEXT (context)->home       = home;

    stack = ST_BLOCK_CONTEXT (context)->stack;
    for (st_uint i=0; i < stack_size; i++)
	stack[i] = st_nil;

    return context;
}

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
