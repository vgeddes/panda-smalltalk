

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
    st_oop *temps;

    temps_size = st_method_temp_count (method) + st_method_arg_count (method);
    stack_size = st_method_stack_depth (method) + temps_size;

    context = st_allocate_object (ST_TYPE_SIZE (STMethodContext) + stack_size);
    st_heap_object_initialize_header (context, st_method_context_class);

    st_context_part_sender (context)     = sender;
    st_context_part_ip (context)         = st_smi_new (0);
    st_context_part_sp (context)         = st_smi_new (0);
    st_method_context_receiver (context) = receiver;
    st_method_context_method (context)   = method;

    temps = st_method_context_temporary_frame (context);
    for (guint i=0; i < temps_size; i++)
	temps[i] = st_nil;

    return context;
}

st_oop *
st_method_context_temporary_frame (st_oop context)
{
    return ST_METHOD_CONTEXT (context)->stack;
}

st_oop *
st_method_context_stack_frame (st_oop context)
{
    st_oop method;
    int frame_size;

    method = st_method_context_method (context);

    frame_size = st_method_temp_count (method) + st_method_arg_count (method);

    return ST_METHOD_CONTEXT (context)->stack + frame_size;
}

st_oop
st_block_context_new (st_oop home, guint initial_ip, guint argcount)
{
    st_oop context;
    st_oop method;
    int stack_size;

    method = st_method_context_method (home);

    stack_size = st_method_stack_depth (method);
    stack_size += argcount;

    context = st_allocate_object (ST_TYPE_SIZE (STBlockContext) + stack_size);
    st_heap_object_initialize_header (context, st_block_context_class);

    st_context_part_sender (context) = st_nil;
    st_context_part_ip (context) = st_smi_new (0);
    st_context_part_sp (context) = st_smi_new (0);

    st_block_context_initial_ip (context) = st_smi_new (initial_ip);
    st_block_context_argcount   (context) = st_smi_new (argcount);
    st_block_context_caller     (context) = st_nil;
    st_block_context_home       (context) = home;

    return context;
}

st_oop
st_message_new (st_oop selector, st_oop arguments)
{
    st_oop object;

    object = st_object_new (st_global_get ("Message"));

    st_heap_object_body (object)[0] = selector;
    st_heap_object_body (object)[1] = arguments;

    return object;
}
