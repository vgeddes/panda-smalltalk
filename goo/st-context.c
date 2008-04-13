

#include "st-context.h"
#include "st-compiled-method.h"

#include "st-object.h"
#include "st-universe.h"

ST_DEFINE_VTABLE (st_method_context, st_heap_object_vtable ());

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
    ST_CONTEXT_PART_METHOD (context) = st_nil;
    ST_CONTEXT_PART_IP (context) = st_nil;
    ST_CONTEXT_PART_SP (context) = st_nil;
    ST_METHOD_CONTEXT_RECEIVER (context) = st_nil;

    return context;
}

st_oop *
st_method_context_temporary_frame (st_oop context)
{
    return ST_METHOD_CONTEXT_STACK (context);
}

st_oop *
st_method_context_stack_frame (st_oop context)
{
    st_oop method;
    int frame_size;

    method = ST_CONTEXT_PART_METHOD (context);

    frame_size = st_compiled_method_temp_count (method) + st_compiled_method_arg_count (method);

    return ST_METHOD_CONTEXT_STACK (context) + frame_size;
}


static void
st_method_context_vtable_init (STVTable *table)
{


}
