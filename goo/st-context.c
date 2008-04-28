

#include "st-context.h"
#include "st-array.h"
#include "st-compiled-method.h"

#include "st-object.h"
#include "st-universe.h"

st_oop
st_method_context_new (st_oop method)
{
    st_oop context;
    int stack_size;

    stack_size = st_compiled_method_stack_depth (method);
    stack_size += st_compiled_method_temp_count (method);
    stack_size += st_compiled_method_arg_count (method);

    context = st_allocate_object (ST_TYPE_SIZE (STMethodContext) + stack_size);
    st_object_initialize_header (context, st_method_context_class);

    ST_CONTEXT_PART_SENDER (context) = st_nil;
    ST_CONTEXT_PART_IP (context) = st_smi_new (0);
    ST_CONTEXT_PART_SP (context) = st_smi_new (0);
    ST_METHOD_CONTEXT_RECEIVER (context) = st_nil;
    ST_METHOD_CONTEXT_METHOD (context) = method;
    
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

    method = ST_METHOD_CONTEXT_METHOD (context);

    frame_size = st_compiled_method_temp_count (method) + st_compiled_method_arg_count (method);

    return ST_METHOD_CONTEXT (context)->stack + frame_size;
}

st_oop
st_block_context_new (st_oop home, guint initial_ip, guint argcount)
{
    st_oop context;
    st_oop method;
    int stack_size;

    method = ST_METHOD_CONTEXT_METHOD (home);
 
    context = st_allocate_object (ST_TYPE_SIZE (STBlockContext) + argcount);
    st_object_initialize_header (context, st_block_context_class);

    ST_CONTEXT_PART_SENDER (context) = st_nil;
    ST_CONTEXT_PART_IP (context) = st_smi_new (0);
    ST_CONTEXT_PART_SP (context) = st_smi_new (0);

    ST_BLOCK_CONTEXT_INITIAL_IP (context) = st_smi_new (initial_ip);
    ST_BLOCK_CONTEXT_ARGCOUNT   (context) = st_smi_new (argcount);
    ST_BLOCK_CONTEXT_CALLER     (context) = st_nil;
    ST_BLOCK_CONTEXT_HOME       (context) = home;

    return context;
}

st_oop
st_message_new (st_oop selector, st_oop *args, guint args_size)
{
    st_oop msg;
    st_oop array;

    msg = st_object_new (st_global_get ("Message"));

    array = st_object_new_arrayed (st_array_class, args_size);

    for (guint i = 1; i <= args_size; i++)
	* st_array_element (array, i) = args[i - 1];

    st_heap_object_instvars (msg)[0] = selector;
    st_heap_object_instvars (msg)[1] = array;
}
