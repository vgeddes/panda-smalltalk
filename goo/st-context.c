

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

    ST_CONTEXT_PART (context)->sender     = sender;
    ST_CONTEXT_PART (context)->ip         = st_smi_new (0);
    ST_CONTEXT_PART (context)->sp         = st_smi_new (0);
    ST_METHOD_CONTEXT (context)->receiver = receiver;
    ST_METHOD_CONTEXT (context)->method   = method;

    temps = ST_METHOD_CONTEXT_TEMPORARY_FRAME (context);
    for (st_uint i=0; i < temps_size; i++)
	temps[i] = st_nil;

    return context;
}

st_oop
st_block_context_new (st_oop home, st_uint initial_ip, st_uint argcount)
{
    st_oop context;
    st_oop method;
    int stack_size;

    method =  ST_METHOD_CONTEXT (home)->method;

    stack_size = st_method_stack_depth (method);
    stack_size += argcount;

    context = st_allocate_object (ST_TYPE_SIZE (STBlockContext) + stack_size);
    st_heap_object_initialize_header (context, st_block_context_class);

    ST_CONTEXT_PART (context)->sender = st_nil;
    ST_CONTEXT_PART (context)->ip     = st_smi_new (0);
    ST_CONTEXT_PART (context)->sp     = st_smi_new (0);

    ST_BLOCK_CONTEXT (context)->initial_ip = st_smi_new (initial_ip);
    ST_BLOCK_CONTEXT (context)->argcount   = st_smi_new (argcount);
    ST_BLOCK_CONTEXT (context)->caller     = st_nil;
    ST_BLOCK_CONTEXT (context)->home       = home;

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
